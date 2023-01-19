local p = premake

p.modules.supreme_package_manager = {}
local m = p.modules.export_compile_commands

local workspace = p.workspace
local project = p.project

m.last_percent = 0

function m.indicateProgress(total, current)
  local ratio = current / total;
  ratio = math.min(math.max(ratio, 0), 1);
  local percent = math.floor(ratio * 100);
  last_percent = percent
end

function m.httpRequest(url, file_path)
  local result_str, response_code = http.download(url, file_path, {
  progress = progress_indicator})
  if response_code ~= 200 then
    error("[" .. _ACTION .. "] blu.http_source(): http.download failed  " .. response_code .. " for " .. url .. " with result " .. result_str)
    return false
  end

  local extension = file_name:match("^.+(%..+)$")
  if os.isfile(file_path) and extension == ".zip"  then
    if zip.extract(file_path, dest_dir) ~= 0 then
      error("failed to extract zip " .. file_name)
      return false
    end
  end

  return true
end

function m.fetchPackages()
  -- fetch the package index.json
  local package_index_url = "https://api.github.com/repos/gitster/git/contents/"

  local packe_index_url = "https://raw.githubusercontent.com/Blu3wolf/surpreme-pack/master/index.json"
  local package_index = "https://raw.githubusercontent.com/Blu3wolf/surpreme-pack/master/index.json"
  local package_index_file = "surpreme-pack/index.json"
  local result = m.httpRequest(package_index, package_index_file)
  if not result then
    error("failed to fetch package index")
    return false
  end


end

return m
