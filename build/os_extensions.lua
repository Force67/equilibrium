-- For Linux/MacOS
os.commandTokens.posix.copyfileifnotexist = function(args)
    local src, dest = args:match("([^%s]+)%s+([^%s]+)")
    local normalizedSrc = path.normalize(src)
    local normalizedDest = path.normalize(dest)
    return string.format("if [ ! -f %s ]; then cp -f %s %s; fi", normalizedDest, normalizedSrc, normalizedDest)
end

-- For Windows
-- Needs further testing here..
os.commandTokens.cmd.copyfileifnotexist = function(args)
    local src, dest = args:match("([^%s]+)%s+([^%s]+)")
    local normalizedSrc = path.translate(path.normalize(src))
    local normalizedDest = path.translate(path.normalize(dest))
    return string.format("IF NOT EXIST %s copy /B /Y %s %s", normalizedDest, normalizedSrc, normalizedDest)
end
