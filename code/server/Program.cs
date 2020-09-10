// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

using System;

// https://stackoverflow.com/questions/53706526/confused-at-control-flow-of-async-await-of-c-sharp

namespace noda
{
    class Program
    {
        static void Main(string[] args)
        {
            new Program().Run();
        }

        public Program()
        {
            _config = Config.Create();

            _logger = new Logger();
            _logger.Sinks.Add(new ConSink());

            _server = new SyncServer(_config, _logger);
            _logger.Info("Starting NODA Server");
        }

        ~Program()
        {
            _config.Save();
        }

        public void Run()
        {
            _server.Run();
        }

        private SyncServer _server;
        private Logger _logger;
        private Config _config;
    }
}