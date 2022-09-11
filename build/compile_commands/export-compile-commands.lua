local p = premake

p.modules.export_compile_commands = {}
local m = p.modules.export_compile_commands

local workspace = p.workspace
local project = p.project

function m.getToolset(cfg)
  return p.tools[cfg.toolset or 'gcc']
end

function m.getCommonFlags(prj, cfg)
  -- some tools that consumes compile_commands.json have problems with relative include paths
  relative = project.getrelative
  project.getrelative = function(prj, dir) return dir end

  local toolset = m.getToolset(cfg)
  local flags = toolset.getcppflags(cfg)
  flags = table.join(flags, toolset.getdefines(cfg.defines))
  flags = table.join(flags, toolset.getundefines(cfg.undefines))
  flags = table.join(flags, toolset.getincludedirs(cfg, cfg.includedirs, cfg.sysincludedirs))
  flags = table.join(flags, toolset.getforceincludes(cfg))
  if project.iscpp(prj) then
    flags = table.join(flags, toolset.getcxxflags(cfg))
  elseif project.isc(prj) then
    flags = table.join(flags, toolset.getcflags(cfg))
  end
  flags = table.join(flags, cfg.buildoptions)
  project.getrelative = relative
  return flags
end

function m.getObjectPath(prj, cfg, node)
  return path.join(cfg.objdir, path.appendExtension(node.objname, '.o'))
end

function m.getDependenciesPath(prj, cfg, node)
  return path.join(cfg.objdir, path.appendExtension(node.objname, '.d'))
end

function m.getFileFlags(prj, cfg, node)
  return table.join(m.getCommonFlags(prj, cfg), {
    '-o', m.getObjectPath(prj, cfg, node),
    '-MF', m.getDependenciesPath(prj, cfg, node),
    '-c', node.abspath
  })
end

function m.generateCompileCommand(prj, cfg, node)
  return {
    directory = prj.location,
    file = node.abspath,
    --command = (path.iscfile(node.abspath) and 'cc ' or 'cxx ') .. table.concat(m.getFileFlags(prj, cfg, node), ' ')

    -- c lion is too stupid to resolve these.
    command = (path.iscfile(node.abspath) and 'gcc ' or 'g++ ') .. table.concat(m.getFileFlags(prj, cfg, node), ' ')
  }
end

function m.includeFile(prj, node, depth)
  return path.iscppfile(node.abspath) or path.iscfile(node.abspath)
end

function m.getProjectCommands(prj, cfg)
  local tr = project.getsourcetree(prj)
  local cmds = {}
  p.tree.traverse(tr, {
    onleaf = function(node, depth)
      if not m.includeFile(prj, node, depth) then
        return
      end

      local fcfg = p.fileconfig.getconfig(node, cfg)
      if fcfg and not fcfg.flags.ExcludeFromBuild then
        table.insert(cmds, m.generateCompileCommand(prj, cfg, node))
      end
    end
  })
  return cmds
end

-- we follow the https://clang.llvm.org/docs/JSONCompilationDatabase.html specification
function m.onWorkspace(wks)
  local cfgCmds = {}
  for prj in workspace.eachproject(wks) do
    for cfg in project.eachconfig(prj) do
      local cfgKey = string.format('%s', cfg.shortname)
      if not cfgCmds[cfgKey] then
        cfgCmds[cfgKey] = {}
      end
      cfgCmds[cfgKey] = table.join(cfgCmds[cfgKey], m.getProjectCommands(prj, cfg))
    end
  end

  local requested_config = _OPTIONS["export-compile-config"]
  for cfgKey,cmds in pairs(cfgCmds) do
    local not_all = requested_config ~= "all"
    -- if requested config mode is not set to all, we only export the target config
    if requested_config ~= cfgKey and not_all then
      goto skip_to_next
    end 

    local outfile = string.format('%s/compile_commands.json', not_all and blu.rootdir or cfgKey)
    p.generate(wks, outfile, function(wks)
      p.push('[')
      for i = 1, #cmds do
        local item = cmds[i]
        p.push('{')
        local command = string.format(
        [["directory": "%s",
        "file": "%s",
        "command": "%s"]],
        item.directory,
        item.file,
        item.command:gsub('\\', '\\\\'):gsub('"', '\\"'))
        p.w(command)
        if i ~= #cmds then
          p.pop('},')
        else
          p.pop('}')
        end
      end
      p.pop(']')
    end)

    ::skip_to_next::
  end
end

newaction({
  trigger = 'export-compile-commands',
  description = 'Export compiler commands in JSON Compilation Database Format',
  onWorkspace = m.onWorkspace
})

newoption({
  trigger     = "export-compile-config",
  value       = "configuration",
  description = "Choose a particular clang compile commands file for use",
  default     = "all",
})

return m
