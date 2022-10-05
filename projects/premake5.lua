local function opt_include(path)
    if os.isdir(path) then
      include(path)
    end
end

--opt_include("./5ed")
opt_include("./overload")
opt_include("./nemisis")
opt_include("./retk")
opt_include("./velocity")