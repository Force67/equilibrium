
function component(name, options)
    project(name)
    kind("SharedLibrary")
end

function unittest(name, options)
    project(name)
    kind("ConsoleApp")
    links("googlemock")
    includedirs(blu.extdir .. "/googletest/googletest/include")
end