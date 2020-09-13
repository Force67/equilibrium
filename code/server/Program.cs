// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

using System;

namespace noda
{
    class Program
    {
        private SyncServer _server;
        private DiscordFeed _discordFeed;
        private Logger _logger;
        private Config _config;

        public Program()
        {
            _config = Config.Create();

            _logger = new Logger();
            _logger.Sinks.Add(new ConSink());

            _server = new SyncServer(_config, _logger);
            _logger.Info("Starting NODA Server");

            _discordFeed = new DiscordFeed(_logger, _config.DiscordToken);
            _logger.Sinks.Add(_discordFeed);
        }
          
        ~Program()
        {
            _config.Save();
        }

        public void Run()
        { 
            _server.Start();
            _discordFeed.Start();

            while (true)
            {
                var input = Console.ReadLine();
                if (input == "/quit")
                {
                    _server.RequestQuit();
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