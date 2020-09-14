// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

using FlatBuffers;
using protocol;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Threading.Tasks;

namespace noda
{
    class SyncServer : net.NetDispatcher<User>, net.NetDelegate
    {
        private Config config;
        private Logger logger;
        private net.NetServer server;
        private List<User> userList;

        public SyncServer(Config config, Logger logger)
        {
            this.config = config;
            this.logger = logger;

            server = new net.NetServer(this, config.ServerPort);
            userList = new List<User>();
        }

        public void Kill()
        {
            // we've chosen death
            userList.ForEach(it => it.Kick(DisconnectReason.Quit));
            server.RequestDeath();
        }

        public void ProcessPacket(ENet.Peer peer, byte[] data, int length)
        {
            ByteBuffer buffer = new ByteBuffer(data);
            var message = MessageRoot.GetRootAsMessageRoot(buffer);

            logger.Trace("Recieved packet " + message.DataType.ToString() + " | length: " + length);

            var user = userList.FirstOrDefault(it => it.GetId() == peer.ID);

            if (message.DataType == Data.HandshakeRequest)
            {
                var hs = message.Data<HandshakeRequest>().Value;
                if (user == null)
                {
                    if (hs.ClientVersion < (ushort)protocol.constants.ProtocolVersion.V_1_0)
                    {
                        peer.Disconnect((uint)DisconnectReason.BadConnection);
                        return;
                    }

                    if (hs.Pass != config.ServerPassword)
                    {
                        peer.Disconnect((uint)DisconnectReason.BadPassword);
                        return;
                    }

                    user = new User(peer);
                    user.hwid = hs.Hwid;
                    user.name = hs.User;
                    userList.Add(user);

                    FlatBufferBuilder fbb = new FlatBufferBuilder(256);
                    var ack = HandshakeAck.CreateHandshakeAck(fbb,
                        fbb.CreateString(hs.User),
                        fbb.CreateString("AProject"),
                        1337);
                    user.SendMessage(fbb, Data.HandshakeAck, ack);

                    logger.Info("User joined: " + hs.User);
                }
            }

            // we still trigger the event though..
            trigger(user, message);
        }
    }
}