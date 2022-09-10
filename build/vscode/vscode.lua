-- Copyright (C) 2022 Vincent Hengel.
-- For licensing information see LICENSE at the root of this distribution.
local p = premake

p.modules.vscode = {}
p.modules.vscode._VERSION = p._VERSION

local vscode = p.modules.vscode
local project = p.project
local workspace = p.workspace

function vscode.getcompiler(cfg)
	local toolset = p.tools[_OPTIONS.cc or cfg.toolset or p.CLANG]
	if not toolset then
		error("Invalid toolset '" + (_OPTIONS.cc or cfg.toolset) + "'")
	end
	return toolset
end

function vscode.generateWorkspace(wks)
	p.utf8()
	p.w('{"folders": [')
	local tr = workspace.grouptree(wks)
	tree.traverse(tr, {
		onleaf = function(n)
			local prj = n.project
			-- Build a relative path from the workspace file to the project file
			local prjpath = path.getrelative(prj.workspace.location, prj.location)
			p.w('{')
			p.w('"path": "%s"', prjpath)
			p.w('},')
		end,
	})
	p.w(']}')
	--TODO wks.startproject
end

function vscode.getConfig(prj)
    for cfg in project.eachconfig(prj) do
        if cfg.shortname == _OPTIONS["vscode-config"] then
            return cfg
        end
    end
end

function vscode.mockAction(str)
    return str:gsub(string.format("/%s", _ACTION), string.format("/%s", _OPTIONS["vscode-fakeaction"]))
end

function vscode.onWorkspace(wks)
    p.eol("\r\n")
    p.indent("  ")
    --p.generate(wks, ".code-workspace", vscode.generateWorkspace)

    p.generate(wks, blu.rootdir .. "/.vscode/launch.json", function(wks)
        p.push('{')
        p.w('"version": "0.2.0",')
		p.w('"configurations":')
        p.push('[')

        local i = 0
        for prj in workspace.eachproject(wks) do
            i = i+1

            if prj.kind == "ConsoleApp" then
            p.push('{')

			p.w('"name": "%s",', prj.name)
			p.w('"type": "cppdbg",')
			p.w('"request": "launch",')

            local target_config = vscode.getConfig(prj)
            if target_config == nil then
                print("u done fucked up")
            end

			p.w('"program": "%s/%s",', vscode.mockAction(target_config.buildtarget.directory), prj.name)
			p.w('"args": [%s],', table.concat(target_config.debugargs, " "))
			p.w('"stopAtEntry": false,')
			p.w('"cwd": "%s",', vscode.mockAction(target_config.buildtarget.directory))
			p.w('"externalConsole": false,')
			p.w('"MIMode": "gdb",')

            p.w('"setupCommands":')
            p.push('[')

                p.push('{')
				p.w('"description": "Enable pretty-printing for gdb",')
				p.w('"text": "-enable-pretty-printing",')
				p.w('"ignoreFailures": true')
                p.pop('},')

                p.push('{')
                p.w('"description": "Enable break on all-exceptions",')
				p.w('"text": "catch throw",')
				p.w('"ignoreFailures": true')
                p.pop('}')

            p.pop('],')

            p.w('"preLaunchTask": "make_%s",', prj.name)
			p.w('"miDebuggerPath": "/usr/bin/gdb"')

            if i ~= #wks.projects then
                p.pop('},')
              else
                p.pop('}')
            end

            end
        end

        p.pop(']')
        p.pop('}')
    end)

    -- horrible workaround for me not being able to pass command line arguments in a reasonable way.
    p.generate(wks, blu.rootdir .. "/.vscode/tasks.json", function(wks)
        p.push('{')
        p.w('"version": "2.0.0",')
		p.w('"tasks":')
        p.push('[')

        local i = 0
        for prj in workspace.eachproject(wks) do
            i=i+1

            local target_config = vscode.getConfig(prj)
            if target_config == nil then
                print("u done fucked up")
            end

            p.push('{')
            p.w('"type": "shell",')
            p.w('"label": "make_%s",', prj.name)

            p.w('"options":')
            p.push('{')
            p.w('"cwd": "%s",', vscode.mockAction(target_config.location)) -- so the makefiles can talk to each other
            p.pop('},')

            p.w('"command": "make %s"', prj.name)
            --p.w('"args": [],')

            --p.w('"group":')
            --p.push('{')
            --p.w('"kind": "default",')
            --p.w('"isDefault": true')
            --p.pop('}')

            if i ~= #wks.projects then
                p.pop('},')
              else
                p.pop('}')
            end

        end


        p.pop(']')
        p.pop('}')
    end)
end

newaction({
  trigger = 'vscode',
  description = 'Generate Visual Studio Code targets',
  onWorkspace = vscode.onWorkspace
})

newoption({
  trigger = "vscode-config",
  value = "configuration",
  description = "Choose a particular configuration target e.g Debug, Release, etc. for use",
  default = "all",
})

newoption({
  trigger = "vscode-fakeaction",
  value = "configuration",
  description = "The real action to give out paths as",
  default = "gmake2",
})

return vscode
