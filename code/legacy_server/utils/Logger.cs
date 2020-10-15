// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace noda
{

    public static class Logger
    {
        private static List<ILogSink> sinks;

        public static void Init()
        {
            sinks = new List<ILogSink>();
        }

        public static void AddSink(ILogSink sink)
        {
            Debug.Assert(sink != null);

            sinks.Add(sink);
        }

        public static void Error(string text)
        {
            Log(LogLevel.Error, text);
        }

        public static void Critical(string text)
        {
            Log(LogLevel.Critical, text);
        }

        public static void Info(string text)
        {
            Log(LogLevel.Info, text);
        }

        public static void Trace(string text)
        {
            Log(LogLevel.Trace, text);
        }

        public static void Warning(string text)
        {
            Log(LogLevel.Warning, text);
        }

        public static void Log(LogLevel level, string text)
        {
            foreach (ILogSink sink in sinks)
            {
                sink.Write("", text, level);
            }
        }
    }
}