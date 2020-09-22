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

        private static System.Reflection.Assembly MyResolveEventHandler(object sender, ResolveEventArgs args)
        {
            Console.WriteLine("Resolving... " + args.Name);
            return typeof(Program).Assembly;
        }

        static void Main(string[] args)
        {
            AppDomain currentDomain = AppDomain.CurrentDomain;
            currentDomain.AssemblyResolve += new ResolveEventHandler(MyResolveEventHandler);


            new Program().Run();
        }
    }
}