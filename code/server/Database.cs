// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

using Npgsql;
using System.Threading.Tasks;

namespace noda
{
    class Database
    {
        private NpgsqlConnection connection;
        private Logger logger;

        public Database(Logger logger, Config config)
        {
            this.logger = logger;

            var connString = 
                "Host=" + config.DbHost + 
                ";Username=" + config.DbUser + 
                ";Password=" + config.DbPass + 
                ";Database=" + config.DbName;

            connection = new NpgsqlConnection(connString);
        }

        public async Task Open()
        {
            await connection.OpenAsync();
        }

        public void ExecuteQuery(string sqlStatement)
        {
            using var cmd = new NpgsqlCommand(sqlStatement, connection);
            var version = cmd.ExecuteScalar().ToString();

            logger.Info($"PostgreSQL version: {version}");
        }
    }
}