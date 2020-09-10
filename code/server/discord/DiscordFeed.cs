// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

using System;
using System.Threading.Tasks;
using Discord;
using Discord.WebSocket;

namespace noda
{
    class DiscordFeed : ILogSink
    {
        private readonly DiscordSocketClient _client;
        private readonly Logger _logger;
        private readonly String _token;
        private bool _gatewayReady = false;

        public DiscordFeed(Logger logger, string token)
        {
            _token = token;
            _logger = logger;

            _client = new DiscordSocketClient();
            _client.Log += LogAsync;
            _client.Ready += ReadyAsync;
            _client.MessageReceived += MessageReceivedAsync;
        }

        private async Task MessageReceivedAsync(SocketMessage message)
        {
            if (message.Author.Id == _client.CurrentUser.Id)
                return;

            if (message.Content == "!ping")
                await message.Channel.SendMessageAsync("pong!");

            if (message.Content == "!stats")
                PostStats();
        }

        private async void PostStats()
        {
            var content = "NO UPDATE CONTENT";
            var embed = new EmbedBuilder
            {
                Title = ":regional_indicator_i: Server Stats",
                Description = content,
                Color = Color.Purple,
            };

            embed.AddField(
                ":regional_indicator_p: **", content);

            var projectCountField = "**" + 1337 + "** updates in **" + "a project" + "**\n";
            embed.AddField(":regional_indicator_u: ** VINCE", projectCountField);

            embed.WithAuthor(_client.CurrentUser)
                 .WithFooter(footer => footer.Text = "NODA Stats")
                 .WithCurrentTimestamp();

            await ((IMessageChannel)_client.GetChannel(753606377590489128)).SendMessageAsync("", false, embed.Build());
        }

        public async void Start()
        {
            await _client.LoginAsync(TokenType.Bot, _token);
            await _client.StartAsync();
        }

        private Task LogAsync(LogMessage log)
        {
            LogLevel level = LogLevel.Info;

            switch (log.Severity)
            {
                case LogSeverity.Critical:
                    level = LogLevel.Critical;
                    break;
                case LogSeverity.Error:
                    level = LogLevel.Error;
                    break;
                case LogSeverity.Warning:
                    level = LogLevel.Warning;
                    break;
            }

            _logger.Log(level, log.Message);
            return Task.CompletedTask;
        }

        private Task ReadyAsync()
        {
            _gatewayReady = true;
            return Task.CompletedTask;
        }

        public async void Write(string message, LogLevel level)
        {
            if (_gatewayReady)
            {
                await ((IMessageChannel)_client.GetChannel(753606377590489128)).SendMessageAsync(level.ToString() + ": " + message);
            }
        }
    }
}