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

function blu.http_source(url, file_name, sha256_hash, dest_dir)
  if _ACTION == "vscode" or _ACTION == "export-compile-commands" then
    return
  end

  local file_path = blu.tempdir .. "/" .. sha256_hash .. file_name
  if os.isfile(file_path) then
    return
  end

  print("sourcing " .. file_name .. " via http")
  -- TODO(Vince): validate the sha

  local result_str, response_code = http.download(url, file_path, {
  progress = progress_indicator})
  if response_code ~= 200 then
    error("[" .. _ACTION .. "] blu.http_source(): http.download failed  " .. response_code .. " for " .. url .. " with result " .. result_str)
    return false
  end

  if os.isfile(file_path) then
    if zip.extract(file_path, dest_dir) ~= 0 then
      error("failed to extract zip " .. file_name)
      return false
    end
  end

  return true
end

