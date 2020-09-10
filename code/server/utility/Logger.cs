// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

using System;
using System.Collections.Generic;

namespace noda
{
    [System.Flags]
    public enum LogLevel
    {
        Trace,    //< very detailed and extensive debugging information
        Debug,    //< less detailed debug information
        Info,     //< status information
        Warning,  //< minor or potential problems
        Error,    //< major problems that block further execution of a task
        Critical, //< major problems that block continued execution of everything
        Count     //< total number of log levels
    }

    public interface ILogSink
    {
        public void Write(string message, LogLevel level);
    }

    public class Logger
    {
        private readonly string TimeFormat;
        public List<ILogSink> Sinks;

        public Logger()
        {
            TimeFormat = "[HH:mm]";
            Sinks = new List<ILogSink>();
        }

        public void Error(string text)
        {
            WriteFormattedLog(LogLevel.Error, text);
        }

        public void Critical(string text)
        {
            WriteFormattedLog(LogLevel.Critical, text);
        }

        public void Info(string text)
        {
            WriteFormattedLog(LogLevel.Info, text);
        }

        public void Trace(string text)
        {
            WriteFormattedLog(LogLevel.Trace, text);
        }

        public void Warning(string text)
        {
            WriteFormattedLog(LogLevel.Warning, text);
        }

        public string FormatLogMessage(LogLevel level, string text)
        {
            string prefix = DateTime.Now.ToString(TimeFormat) + " <" +  level.ToString() + "> ";
            return prefix + text;
        }

        private void WriteFormattedLog(LogLevel level, string text)
        {
            var msg = FormatLogMessage(level, text);
            foreach(ILogSink sink in Sinks)
            {
                sink.Write(msg, level);
            }
        }
    }
}