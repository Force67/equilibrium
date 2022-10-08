matches = os.matchdirs("*")
for i,v in ipairs(matches) do
  if os.isfile("./" .. v .. "/premake5.lua") then
    family(v)
    include("./" .. v)
    family("")
  end
end