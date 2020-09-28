// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

using System.Data.SQLite;
using System.IO;

namespace noda
{ 
    // TEST:

    class Project
    {
        private SQLiteConnection connection;

        public string name { get; set; }
        public string md5 { get; set; }

        // new project
        public Project(string name, string md5)
        {
            connection.Open();
            using (var cmd = new SQLiteCommand(connection))
            {
                // create the project database layout
                cmd.CommandText =
                    @"CREATE TABLE info (infoid INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL, 
                            value TEXT NOT NULL);

                        CREATE TABLE updates (updateid INTEGER PRIMARY KEY AUTOINCREMENT, user_guid TEXT NOT NULL,
                            created TIMESTAMP DEFAULT CURRENT_TIMESTAMP, message_type BIGINT, data BLOB);

                        CREATE TABLE users (userid INTEGER PRIMARY KEY AUTOINCREMENT, user_guid TEXT NOT NULL, user_name TEXT NOT NULL);";

                cmd.ExecuteNonQuery();

                InitializeDb(name, md5);
            }
        }

        public Project(string name)
        {
            var cs = @"URI=file:" + path;
            connection = new SQLiteConnection(cs);

            if (!File.Exists(path))
            {
                connection.Open();
                using(var cmd = new SQLiteCommand(connection))
                {
                    // create the project database layout
                    cmd.CommandText =
                        @"CREATE TABLE info (infoid INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL, 
                            value TEXT NOT NULL);

                        CREATE TABLE updates (updateid INTEGER PRIMARY KEY AUTOINCREMENT, user_guid TEXT NOT NULL,
                            created TIMESTAMP DEFAULT CURRENT_TIMESTAMP, message_type BIGINT, data BLOB);

                        CREATE TABLE users (userid INTEGER PRIMARY KEY AUTOINCREMENT, user_guid TEXT NOT NULL, user_name TEXT NOT NULL);";

                    cmd.ExecuteNonQuery();
                }
            } else
                connection.OpenAsync();
        }

        public void AddUpdate(string guid, int type, byte[] data)
        {
            using (var cmd = new SQLiteCommand(connection))
            {
                cmd.CommandText = @"INSERT INTO updates (user_guid, message_type, data) VALUES(@user_guid, @message_type, @data);";

                cmd.Parameters.AddWithValue("@user_guid", guid);
                cmd.Parameters.AddWithValue("@message_type", type);
                cmd.Parameters.AddWithValue("@data", data);
                cmd.Prepare();

                cmd.ExecuteNonQuery();
            }
        }

        public void AddClient(string x, string y)
        {
            using (var cmd = new SQLiteCommand(connection))
            {
                cmd.CommandText = @"INSERT INTO users (user_guid, user_name) VALUES(@user_guid, @user_name);";

                cmd.Parameters.AddWithValue("@user_guid", x);
                cmd.Parameters.AddWithValue("@user_name", y);
                cmd.Prepare();

                cmd.ExecuteNonQuery();
            }
        }

        public void InitializeDb(string name, string id)
        {
            using (var cmd = new SQLiteCommand(connection))
            {
                cmd.CommandText = @"INSERT INTO info (name, value) VALUES(""idb_name"", @v1);
                                    INSERT INTO info (name, value) VALUES(""idb_md5"", @v2);";

                cmd.Parameters.AddWithValue("@v1", name);
                cmd.Parameters.AddWithValue("@v2", id);
                cmd.Prepare();

                cmd.ExecuteNonQuery();
            }
        }
    }
}