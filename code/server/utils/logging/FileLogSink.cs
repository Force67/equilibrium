// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

using System.Globalization;
using System.IO;

namespace noda
{
    public class FileLogSink : ILogSink
    {
        private StreamWriter writer;

        public FileLogSink(string filename, bool appendMode)
        {
            try
            {
                writer = new StreamWriter(filename, appendMode);
            }
            catch (IOException)
            {
                writer = null;
            }
        }

        public void Write(string source, string message, LogLevel level)
        {
            if (writer == null)
                return;

            var date = System.DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss", CultureInfo.InvariantCulture);
            var levelS = level.ToString().ToUpper();
            var sourceS = (source == string.Empty) ? string.Empty : ("[" + source + "]");

            var text = string.Format("{0} - {1} - {2}: {3}", date, sourceS, levelS, message);
            writer.WriteLine(text);
            writer.Flush();
        }

        public bool UseFilter { get { return true; } }
    }
}