// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

using System;

namespace noda
{
    [Flags]
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
        void Write(string source, string message, LogLevel level);
        bool UseFilter { get; }
    }
}