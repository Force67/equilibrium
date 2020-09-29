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

#nullable enable
namespace noda.net
{
    class NetServer
    {
        private Host host;
        private Event netEvent;
        private List<Client> clientRegistry;
        protected FlatBufferBuilder fbb;
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

        public void Broadcast(Client exclude, Message message)
        {
            clientRegistry.ForEach(delegate (Client it)
            {
                if (it == exclude)
                    return;

                it.SendReliable(message.ByteBuffer.ToSizedArray());
            });
        }

        public void Announce(AnnounceType type, string data, Client? exclude = null)
        {
            clientRegistry.ForEach(delegate (Client it)
            {
                if (it == exclude)
                    return;

                it.SendMessage(fbb, MsgType.Announcement,  
                    Announcement.CreateAnnouncement(
                    fbb, type, fbb.CreateString(data)));
            });
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
                         var it = clientRegistry.FirstOrDefault(it => it.id == netEvent.Peer.ID);
                         if (it == null)
                            return;

                         clientRegistry.Remove(it);

                         OnDisconnection(it);
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

            var client = clientRegistry.FirstOrDefault(it => it.id == source.ID);

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

            var client = new Client(source, hs.Name, hs.Guid);

            return client.SendMessage(fbb, MsgType.HandshakeAck, HandshakeAck.CreateHandshakeAck(fbb, 
                (int)source.ID, GetClientCount())) ? client : null;
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