// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

using System;

namespace noda
{
    public class ConsoleLogSink : ILogSink
    {
        public void Write(string source, string message, LogLevel level)
        {
            var previousColor = Console.ForegroundColor;

            switch (level)
            {
                case LogLevel.Trace:
                    Console.ForegroundColor = ConsoleColor.Gray;
                    break;
                case LogLevel.Debug:
                    Console.ForegroundColor = ConsoleColor.Cyan;
                    break;
                case LogLevel.Info:
                    Console.ForegroundColor = ConsoleColor.White;
                    break;
                case LogLevel.Warning:
                    Console.ForegroundColor = ConsoleColor.Yellow;
                    break;
                case LogLevel.Error:
                    Console.ForegroundColor = ConsoleColor.Red;
                    break;
                case LogLevel.Critical:
                    Console.ForegroundColor = ConsoleColor.Magenta;
                    break;
            }

            var levelS = level.ToString();
            var sourceS = (source == string.Empty) ? string.Empty : ("[" + source + "]");

            var text = string.Format("[{0}] {1} - {2}", sourceS, levelS, message);

            Console.WriteLine(text);
            Console.ForegroundColor = previousColor;
        }

        public bool UseFilter { get { return true; } }
    }
}