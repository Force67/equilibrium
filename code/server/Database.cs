// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

using System.Threading.Tasks;
using System.Data.SQLite;

namespace noda
{
    class Database
    {
        private SQLiteConnection connection;
        private Logger logger;

        public Database(Logger logger, Config config)
        {
            var cs = @"URI=file:" + config.DbName;

            this.logger = logger;

            connection = new SQLiteConnection(cs);
        }

        public async Task Open()
        {
            await connection.OpenAsync();
        }

        public void ExecuteQuery(string sqlStatement)
        {
            using var cmd = new SQLiteCommand(sqlStatement, connection);
            var version = cmd.ExecuteScalar().ToString();

            logger.Info($"SQL version: {version}");
        }
    }
}