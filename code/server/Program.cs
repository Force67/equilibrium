// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

using System;
using System.Collections.Generic;

namespace noda
{
    class Program
    {
        private SyncServer server;
        private Logger logger;
        private Config config;
       // private DiscordFeed discordFeed;
        private List<Action<string>> commands;

        public Program()
        {
            config = Config.Create();

            logger = new Logger();
            logger.Sinks.Add(new ConSink());

            server = new SyncServer(config, logger);
            logger.Info("Starting NODA Server");

           // discordFeed = new DiscordFeed(logger, config.DiscordToken);
          //  logger.Sinks.Add(discordFeed);
        }
          
        ~Program()
        {
            config.Save();
        }

        public void Run()
        { 
            //discordFeed.Start();

            while (true)
            {
                var input = Console.ReadLine();
                if (input == "/quit")
                {
                    server.Kill();
                    break;
                }
            }
        }

        static void Main(string[] args)
        {
            new Program().Run();
        }
    }
}