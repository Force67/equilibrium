// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

using System;
using System.IO;
using System.Collections.Generic;
using protocol;
using System.Linq;

namespace noda
{
    class Server2 : net.NetServer
    {
        Config config;
        string ndbPath;
        List<Project> projects;

        public Server2(Config config) : base(config.Port, 1024)
        {
            projects = new List<Project>();

            SetTitle();
            Logger.Info("Server started on port " + config.Port);

            this.config = config;
            LoadProjects();
        }

        private void LoadProjects()
        {
            ndbPath = Path.Combine(Directory.GetCurrentDirectory(), config.DbFolder);
            if (Directory.Exists(ndbPath))
            {
                var files = Directory.GetFiles(ndbPath, "*.ndb");
                foreach (var file in files)
                {
                    var project = new Project(file);
                    Logger.Info("Loaded project : " + project.name);

                    projects.Add(new Project(file));
                }

                return;
            }

            Directory.CreateDirectory(ndbPath);
        }

        private void CreateTestDb()
        {
                /*
            var prj = new Project("D:\\TEMP_Devel\\NomadGroup\\NODA\\build\\bin\\Debug\\netcoreapp3.1\\db\\test.db");
            prj.InitializeDb("TESTDB", "54645645645645645654");
            prj.AddClient("1234", "Force");

            byte[] data = new byte[10];
            for (int i = 0; i < 10; i++)
                data[i] = 10;

            prj.AddUpdate("54645645645645645654", 30, data);
            */
        }

        public override void OnConnection(Client client)
        {
            Logger.Info("Connection recieved " + client.name);
            SetTitle();
        }

        public override void OnDisconnection(Client client)
        {
            Logger.Info("Connection ended " + client.name);
            SetTitle();

            Announce(AnnounceType.Disconnect, client.name);
        }

        public override void ProcessPacket(Client src, Message message)
        {
            if (message.MsgType == MsgType.LocalProject)
            {
                var info = message.Msg<LocalProject>().Value;

                // we are creating a new db
                var project = projects.FirstOrDefault(it => it.md5 == info.Md5);
                if (project == null)
                {
                    Logger.Trace("IN: " + info.Md5);

                    project = new Project(ndbPath, info.Name, info.Md5);
                    project.InitStorage();

                    Logger.Info(src.name + " created new project: " + project.name);

                    project.AddClient(src.name, src.guid);

                    // update version 0
                    src.SendMessage(fbb, MsgType.RemoteProject,
                        RemoteProject.CreateRemoteProject(fbb,
                        0, fbb.CreateString(info.Name)));

                    projects.Add(project);

                } else
                {
                    Logger.Info(src.name + " joined project: " + project.name);
                }

                Announce(AnnounceType.Joined, src.name, src);
                src.project = project;
                return;
            }

            // store message
            var proj = src.project;
            proj.Update(src.guid, message);

            Broadcast(src, message);
        }

        public bool IsListening()
        {
            return true;
        }

        public void SetTitle()
        {
            Console.Title = "NODA Server - " + GetClientCount() + " users";
        }
    }
}