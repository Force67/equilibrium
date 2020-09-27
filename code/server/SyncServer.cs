// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

using FlatBuffers;
using protocol;
using System;
using System.Collections.Generic;
using System.Linq;

namespace noda
{
    class SyncServer : net.NetDispatcher<User>, net.NetDelegate
    {
        private Config config;
        private Logger logger;
        private net.NetServer server;
        private List<User> clients;
        private FlatBufferBuilder fbb;

        public SyncServer(Config config, Logger logger)
        {
            this.config = config;
            this.logger = logger;

            server = new net.NetServer(this, config.ServerPort);
            clients = new List<User>();

            fbb = new FlatBufferBuilder(1024);
        }

        public void Kill()
        {
            // we've chosen death
            clients.ForEach(it => it.Kick(DisconnectReason.Quit));
            server.RequestDeath();
        }

        public void OnConnection(ENet.Peer peer)
        {
            logger.Trace("Connection request " + peer.ID);
        }

        public void OnDisconnection(ENet.Peer peer)
        {
            // yeet user
            var user = clients.FirstOrDefault(it => it.GetId() == peer.ID);
            if (user == null)
            {
                logger.Error("Recieved disconnection for invalid user index " + peer.ID);
                return;
            }

            string name = user.name;

            logger.Info("User disconnected: " + user.name);
            clients.Remove(user);

            var msg = Broadcast.CreateBroadcast(fbb, 
                BroadcastType.Disconnect, fbb.CreateString(name));

            clients.ForEach(it => it.SendMessage(fbb, MsgType.Broadcast, msg));
        }

        public async void ProcessPacket(ENet.Peer peer, byte[] data, int length)
        {
            ByteBuffer buffer = new ByteBuffer(data);
            var message = Message.GetRootAsMessage(buffer);

            logger.Trace("Recieved packet " + message.MsgType.ToString() + " | length: " + length);

            var user = clients.FirstOrDefault(it => it.GetId() == peer.ID);
            if (message.MsgType == MsgType.HandshakeRequest)
            {
                var hs = message.Msg<HandshakeRequest>().Value;
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

                    // tell everyone about the guy who left
                    var pack = Broadcast.CreateBroadcast(fbb, 
                        BroadcastType.Joined, fbb.CreateString(hs.User));
                    clients.ForEach(it => it.SendMessage(fbb, MsgType.Broadcast, pack));

                    user = new User(peer);
                    user.hwid = hs.Hwid;
                    user.name = hs.User;
                    clients.Add(user);

                    var ack = HandshakeAck.CreateHandshakeAck(fbb,
                        fbb.CreateString(hs.User),
                        fbb.CreateString("AProject"),
                        1337);
                    user.SendMessage(fbb, MsgType.HandshakeAck, ack);

                    logger.Info("User joined: " + hs.User);
                }
            }

            //clients.ForEach(it => )

            // we still trigger the event though..
            trigger(user, message);
        }
    }
}