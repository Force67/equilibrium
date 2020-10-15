// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

using protocol;
using System.Data.SQLite;
using System.IO;
using System.Threading.Tasks;

namespace noda
{ 
    // TEST:

    class Project
    {
        private SQLiteConnection connection;

        public string name { get; set; }
        public string md5 { get; set; }

        // new project
        public Project(string dbRoot, string name, string md5)
        {
            this.name = name;
            this.md5 = md5;

            // store the DB with md5 name to avoid collisions
            // TODO: what if there are several versions of the same _input_ file
            var cs = @"URI=file:" + Path.Combine(dbRoot, md5 + ".ndb");

            Logger.Trace("CS create: " + cs);
            connection = new SQLiteConnection(cs);
            connection.Open();
        }

        // existing project
        public Project(string fileName)
        {
            var cs = @"URI=file:" + fileName;
            connection = new SQLiteConnection(cs);
            connection.Open();

            // query the project name
            using (var cmd = new SQLiteCommand(
                "SELECT (value) FROM info WHERE name='idb_name'", connection))
            {
                using var reader = cmd.ExecuteReader();
                reader.Read();
                name = reader.GetString(0);
            }

            using (var cmd = new SQLiteCommand(
                "SELECT (value) FROM info WHERE name='idb_md5'", connection))
            {
                using var reader = cmd.ExecuteReader();
                reader.Read();
                md5 = reader.GetString(0);
            }
        }

        public void InitStorage()
        {
            using (var cmd = new SQLiteCommand(connection))
            {
                // create the project database layout
                cmd.CommandText =
                    @"CREATE TABLE info (
                            infoid INTEGER PRIMARY KEY AUTOINCREMENT, 
                            name TEXT NOT NULL, 
                            value TEXT NOT NULL
                      );
                      CREATE TABLE updates (
                            updateid INTEGER PRIMARY KEY AUTOINCREMENT, 
                            user_guid TEXT NOT NULL,
                            created TIMESTAMP DEFAULT CURRENT_TIMESTAMP, 
                            message_type BIGINT, 
                            data BLOB
                      );
                      CREATE TABLE users (
                            userid INTEGER PRIMARY KEY AUTOINCREMENT, 
                            user_guid TEXT NOT NULL, 
                            user_name TEXT NOT NULL
                      );

                      INSERT INTO info (name, value) VALUES('idb_name', @name);
                      INSERT INTO info (name, value) VALUES('idb_md5', @md5);";

                cmd.Parameters.AddWithValue("@name", name);
                cmd.Parameters.AddWithValue("@md5", md5);
                cmd.Prepare();

                cmd.ExecuteNonQuery();
            }
        }

        public void Update(string guid, Message msg)
        {
            using (var cmd = new SQLiteCommand(connection))
            {
                cmd.CommandText = @"INSERT INTO updates (user_guid, message_type, data) VALUES(@user_guid, @message_type, @data);";

                cmd.Parameters.AddWithValue("@user_guid", guid);
                cmd.Parameters.AddWithValue("@message_type", msg.MsgType);
                cmd.Parameters.AddWithValue("@data", msg.ByteBuffer.ToSizedArray());
                cmd.Prepare();

                cmd.ExecuteNonQuery();
            }
        }

        public void AddClient(string name, string guid)
        {
            using (var cmd = new SQLiteCommand(connection))
            {
                cmd.CommandText = @"INSERT INTO users (user_guid, user_name) VALUES(@user_guid, @user_name);";

                cmd.Parameters.AddWithValue("@user_guid", guid);
                cmd.Parameters.AddWithValue("@user_name", name);
                cmd.Prepare();

                cmd.ExecuteNonQuery();
            }
        }
    }
}