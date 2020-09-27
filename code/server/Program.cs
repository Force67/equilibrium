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
        private Database db;
       // private DiscordFeed discordFeed;

        public Program()
        {
            // https://social.msdn.microsoft.com/Forums/en-US/1b1b316a-8648-4243-a651-84de51fd2508/reference-native-dll-from-managed-c-project?forum=vssmartdevicesvbcs

            try
            {
            config = Config.Create();

                logger = new Logger();
                logger.Sinks.Add(new ConSink());

                db = new Database(logger, config);
                db.Open();

                server = new SyncServer(config, logger);
                logger.Info("Starting NODA Server");

            }
            catch (Exception exc)
            {
                Console.WriteLine(exc.ToString());
            }

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