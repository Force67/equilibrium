// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

using System;

namespace noda
{
    class Program
    {
        static void Main(string[] args)
        {
            Console.WriteLine("Starting NODA Server");

            new Program().Run();
        }

        public Program()
        {
            _server = new NetServer(4523);
        }

        public void Run()
        {
            _server.Run();
        }

    private
        NetServer _server;
    }
}