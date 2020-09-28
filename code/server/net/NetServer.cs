// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

using System;
using ENet;
using protocol;
using System.Threading;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using FlatBuffers;
using System.Linq;

namespace noda.net
{
    class NetServer
    {
        private Host host;
        private Event netEvent;
        private List<Client> clientRegistry;
        private FlatBufferBuilder fbb;
        private Dictionary<MsgType, Action<Client, Message>> handlers;

        public virtual void OnConnection(Client src) { }
        public virtual void OnDisconnection(Client src) { }
        public virtual void ProcessPacket(Client src, Message msg) { }

        public NetServer(ushort port, int netBufferSize)
        {
            Library.Initialize();

            host = new Host();
            var addr = new Address();

            addr.Host = 0;
            addr.Port = port;
            host.Create(addr, 10);

            fbb = new FlatBufferBuilder(netBufferSize);

            clientRegistry = new List<Client>();
            handlers = new Dictionary<MsgType, Action<Client, Message>>();
        }

        ~NetServer()
        {
            host.Dispose();
            Library.Deinitialize();
        }

        public int GetClientCount()
        {
            return clientRegistry.Count;
        }

        public void KickAll()
        {
            // we've chosen death
            clientRegistry.ForEach(it => it.Kick(DisconnectReason.Quit));
        }

        public void BroadCast(BroadcastType type, string text)
        {
            var pack = Broadcast.CreateBroadcast(fbb, type, fbb.CreateString(text));
            clientRegistry.ForEach(it => it.SendMessage(fbb, MsgType.Broadcast, pack));
        }

        public void Tick()
        {
            if (host.Service(0, out netEvent) > 0)
            {
                switch (netEvent.Type)
                {
                    case EventType.Connect:
                        break;
                    case EventType.Disconnect:
                    {
                         var client = clientRegistry.FirstOrDefault(it => it.GetId() == netEvent.Peer.ID);
                         if (client == null)
                            return;

                         BroadCast(BroadcastType.Disconnect, client.name);
                         OnDisconnection(client);

                         clientRegistry.Remove(client);
                         netEvent.Peer.Reset();
                         break;
                    }
                    case EventType.Receive:
                    {
                         var length = netEvent.Packet.Length;
                         // convert the data to managed
                         byte[] managedArray = new byte[length];
                         Marshal.Copy(netEvent.Packet.Data, managedArray, 0, length);

                         ProcessIncomingPacket(netEvent.Peer, managedArray, length);
                         break;
                    }
                }
            }
        }

        private void ProcessIncomingPacket(Peer source, byte[] data, int length)
        {
            var buffer = new ByteBuffer(data);
            var message = Message.GetRootAsMessage(buffer);

            Logger.Trace("Message: " + message.MsgType.ToString() + " | " + length);

            var client = clientRegistry.FirstOrDefault(it => it.GetId() == source.ID);

            switch (message.MsgType)
            {
                case MsgType.HandshakeRequest:
                {
                    client = NegotiateHandshake(source, message);
                    if (client == null)
                    {
                        source.Disconnect(1);
                        return;
                    }

                    BroadCast(BroadcastType.Joined, client.name);
                    clientRegistry.Add(client);
          
                    // forward the event
                    OnConnection(client);
                    return;
                }
            }

            ProcessPacket(client, message);
        }

        private Client? NegotiateHandshake(Peer source, Message message)
        {
            var hs = message.Msg<HandshakeRequest>().Value;

            if (hs.ProtocolVersion < 
                (ushort)protocol.constants.ProtocolVersion.Current)
            {
                source.Disconnect((uint)DisconnectReason.BadConnection);
                return null;
            }

            var client = new Client(source, hs.Guid, hs.Name);

            var ack = HandshakeAck.CreateHandshakeAck(fbb, (int)source.ID, GetClientCount());
            return client.SendMessage(fbb, MsgType.HandshakeAck, ack) ? client : null;
        }

        private void TriggerEvents(Client client, Message message)
        {
            foreach (var sub in handlers)
            {
                if (sub.Key == message.MsgType)
                {
                    sub.Value.Invoke(client, message);
                    break;
                }
            }
        }
    }
}