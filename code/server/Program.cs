// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

namespace noda
{
    class Program
    {
        static void Main(string[] args)
        {
            bool disableFileLogger = false;

            for (int i = 0; i < args.Length; i++)
            {
                if (args[i] == "-nofilelog")
                    disableFileLogger = true;
            }

            Config config = Config.Load("NODA.config");

            Logger.Init();
            Logger.AddSink(new ConsoleLogSink());
            if (!disableFileLogger)
                Logger.AddSink(new FileLogSink("NODAServer.log", false));

            Logger.Info("Initializing NODA");
            Logger.Info("Localizing the hardware pope");

            try
            {
                var server = new Server2(config);

                while (server.IsListening())
                    server.Tick();
            }
            catch(System.Exception ex)
            {
                Logger.Error("Caught exception: " + ex.Message);
            }
        }
    }
}