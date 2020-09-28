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
        List<Project> projects;

        public Server2(Config config) : base(config.Port, 1024)
        {
            SetTitle();
            Logger.Info("Server started on port " + config.Port);

            this.config = config;
            LoadProjects();
        }

        private void LoadProjects()
        {
            var prj = new Project("D:\\TEMP_Devel\\NomadGroup\\NODA\\build\\bin\\Debug\\netcoreapp3.1\\db\\test.db");
            prj.InitializeDb("TESTDB", "54645645645645645654");
            prj.AddClient("1234", "Force");

            byte[] data = new byte[10];
            for (int i = 0; i < 10; i++)
                data[i] = 10;

            prj.AddUpdate("54645645645645645654", 30, data);

            projects = new List<Project>();

            // ensure that the db directory exists
            string dbRoot = Directory.GetCurrentDirectory() + "\\" + config.DbLocation;
            if (!Directory.Exists(dbRoot))
                Directory.CreateDirectory(dbRoot);

            string[] dbs = Directory.GetFiles(dbRoot, "*.ndb");

            foreach(var dbDir in dbs)
            {
                var project = new Project(dbDir);
                projects.Add(project);
            }
        }

        public override void OnConnection(Client client)
        {
            Logger.Info("Connection recieved " + client.name);
            SetTitle();

            // mount the project

        }

        public override void OnDisconnection(Client client)
        {
            Logger.Info("Connection ended " + client.name);
            SetTitle();
        }

        public override void ProcessPacket(Client src, Message msg)
        {
            if (msg.MsgType == MsgType.LocalProject)
            {
                var info = msg.Msg<LocalProject>().Value;

                // we are creating a new db
                var proj = projects.FirstOrDefault(it => it.md5 == info.Md5);
                if (proj == null)
                    proj = new Project(proj.name, proj.md5);

                proj.AddClient(src.name, src.guid);
            }

            projects.
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