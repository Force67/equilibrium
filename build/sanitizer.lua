-- Taken from https://github.com/premake/premake-core/issues/1595
-- here until the issue gets closed.
-- Register api command
premake.api.register({
    name = "enableASAN",
    scope = "config",
    kind = "string",
    allowed = { "true", "false" }
})

premake.override(premake.vstudio.vc2010, "configurationProperties", function(base, cfg)
    local m = premake.vstudio.vc2010
    m.propertyGroup(cfg, "Configuration")
    premake.callArray(m.elements.configurationProperties, cfg)
    if cfg.enableASAN then
        m.element("EnableASAN", nil, cfg.enableASAN)
    end
    premake.pop('</PropertyGroup>')
end)