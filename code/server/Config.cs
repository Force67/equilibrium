// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

using System;
using System.IO;
using System.Text.Json;

namespace noda
{
    public class Config
    {
        public static Config Load(string fileName)
        {
            if (File.Exists(fileName))
            {
                string jsonContents = File.ReadAllText(fileName);
                return JsonSerializer.Deserialize<Config>(jsonContents);
            }

            return new Config();
        }

        // Server:
        public ushort Port { get; set; } = 4523;
        public string Password { get; set; } = "";
        public string DbLocation { get; set; } = "db";

        // Networking:
        public int NetTimeOut { get; set; } = 3000;

        // Feed:
        public string DiscordToken { get; set; } = "";
    }
}