// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

using System;
using System.IO;
using System.Text.Json;

namespace noda
{
    public class Config
    {
        public ushort ServerPort { get; set; } = 4523;
        public string ServerPassword { get; set; } = "";
        public int NetTimeOut { get; set; } = 3000;
        public string DiscordToken { get; set; } = "";

        public static Config Create()
        {
            if (File.Exists("NODAConfig.json"))
            {
                string jsonContents = File.ReadAllText("NODAConfig.json");
                return JsonSerializer.Deserialize<Config>(jsonContents);
            }

            var config = new Config();
            config.Save();

            return config;
        }

        public void Save()
        {
            var options = new JsonSerializerOptions
            {
                WriteIndented = true
            };

            string modelJson = JsonSerializer.Serialize<Config>(this, options);
            Console.WriteLine(modelJson);
            File.WriteAllText("NODAConfig.json", modelJson);
        }
    }
}