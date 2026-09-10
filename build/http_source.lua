last_percent = 0

function progress_indicator(total, current)
  local ratio = current / total;

  ratio = math.min(math.max(ratio, 0), 1);
  local percent = math.floor(ratio * 100);
  -- to avoid spam
  if percent % 5 == 0 and last_percent ~= percent then
    print("[Premake] Download progress (" .. percent .. "%)")
  end

  last_percent = percent
end

-- Premake exposes no hashing function, so the digest comes from whichever
-- platform tool is present. Every parser below is anchored: the cache file name
-- contains the *expected* digest, and a loose scan of a tool's output would
-- match that copy of it instead of the computed one, verifying nothing.
local function read_sha256(path)
  local quoted = '"' .. path .. '"'
  local candidates
  if os.ishost("windows") then
    candidates = {
      {
        command = "certutil -hashfile " .. quoted .. " SHA256",
        -- A header line, the digest, then a status line. Older builds space
        -- the digest out in byte pairs.
        parse = function(output)
          local lines = {}
          for line in output:gmatch("[^\r\n]+") do
            lines[#lines + 1] = line
          end
          if not lines[2] then
            return nil
          end
          return (lines[2]:gsub("%s", ""))
        end,
      },
    }
  else
    candidates = {
      {
        command = "sha256sum " .. quoted,
        parse = function(output) return output:match("^(%x+)%s") end,
      },
      {
        command = "shasum -a 256 " .. quoted,
        parse = function(output) return output:match("^(%x+)%s") end,
      },
      {
        command = "openssl dgst -sha256 " .. quoted,
        parse = function(output) return output:match("=%s*(%x+)%s*$") end,
      },
    }
  end

  local tried = {}
  for _, candidate in ipairs(candidates) do
    local output = os.outputof(candidate.command)
    if output then
      local digest = candidate.parse(output)
      if digest then
        digest = digest:lower()
        if #digest == 64 and digest:match("^%x+$") then
          return digest
        end
      end
    end
    tried[#tried + 1] = candidate.command:match("^(%S+)")
  end

  return nil, "no usable SHA-256 tool (tried " .. table.concat(tried, ", ") .. ")"
end

-- Checks the file at |path| against |expected|. A digest that cannot be
-- computed counts as a mismatch: an archive this build cannot verify is one it
-- does not unpack.
local function verify_sha256(path, expected, file_name)
  local digest, reason = read_sha256(path)
  if not digest then
    return false, "cannot verify " .. file_name .. ": " .. reason
  end
  if digest ~= expected then
    return false, "sha256 mismatch for " .. file_name .. " (expected " .. expected ..
                  ", got " .. digest .. ")"
  end
  return true
end

function blu.http_source(url, file_name, sha256_hash, dest_dir)
  if _ACTION == "vscode" or _ACTION == "export-compile-commands" then
    return
  end

  local expected = type(sha256_hash) == "string" and sha256_hash:lower() or ""
  if #expected ~= 64 or not expected:match("^%x+$") then
    error("blu.http_source(): " .. tostring(file_name) ..
          " needs a 64 character SHA-256 to verify against, got '" ..
          tostring(sha256_hash) .. "'")
    return false
  end

  local file_path = blu.tempdir .. "/" .. expected .. file_name
  if os.isfile(file_path) then
    -- The cached copy gets re-verified rather than trusted. Its name carries
    -- the expected digest, but nothing stops another process from writing
    -- different bytes under that name.
    local cached_ok, cached_why = verify_sha256(file_path, expected, file_name)
    if cached_ok then
      return true
    end
    print("[Premake] discarding cached " .. file_name .. ": " .. cached_why)
    os.remove(file_path)
  end

  print("sourcing " .. file_name .. " via http")

  -- Download to a staging name and only move it into the cache once verified,
  -- so a truncated or tampered archive is never left where the next run would
  -- pick it up as a cache hit.
  local staging_path = file_path .. ".part"
  os.remove(staging_path)

  local result_str, response_code = http.download(url, staging_path, {
  progress = progress_indicator})
  if response_code ~= 200 then
    os.remove(staging_path)
    error("[" .. _ACTION .. "] blu.http_source(): http.download failed  " .. tostring(response_code) .. " for " .. url .. " with result " .. tostring(result_str))
    return false
  end

  local verified, why = verify_sha256(staging_path, expected, file_name)
  if not verified then
    os.remove(staging_path)
    error("[" .. _ACTION .. "] blu.http_source(): " .. why .. " from " .. url)
    return false
  end

  if not os.rename(staging_path, file_path) then
    os.remove(staging_path)
    error("[" .. _ACTION .. "] blu.http_source(): could not move the verified " ..
          file_name .. " into " .. file_path)
    return false
  end

  if zip.extract(file_path, dest_dir) ~= 0 then
    error("failed to extract zip " .. file_name)
    return false
  end

  return true
end
