
local dotnetbase = premake.vstudio.dotnetbase

premake.override(dotnetbase, 'assemblyName', function(base, cfg)
    base(cfg)

    if dotnetbase.isNewFormatProject(cfg) then
        _p(2,'<AssemblyName>%s</AssemblyName>', cfg.buildtarget.basename)
    end
end)