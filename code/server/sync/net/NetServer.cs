// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

using ENet;
using System;

namespace noda
{
    class NetServer
    {
        public NetServer(ushort port)
        {
            Library.Initialize();

            _server = new Host();
            Address addr = new Address();

            addr.Host = 0;
            addr.Port = port;
            _server.Create(addr, 10);
        }

        ~NetServer()
        {
            _server.Dispose();
            Library.Deinitialize();
        }

        public virtual void OnConnection() { }
        public virtual void OnDisconnection() { }
        public virtual void OnReceive(IntPtr data, int length) { }

        public void RequestQuit()
        {
            _shouldRun = false;
        }

        public void Run()
        {
            while (_shouldRun)
            {
                if (_server.Service(0, out _netEvent) > 0)
                {
                    switch (_netEvent.Type)
                    {
                        case EventType.Connect:
                            OnConnection();
                            break;
                        case EventType.Disconnect:
                            OnDisconnection();
                            _netEvent.Peer.Reset();
                            break;
                        case EventType.Receive: {
                            OnReceive(_netEvent.Packet.Data, _netEvent.Packet.Length);
                            _netEvent.Packet.Dispose();
                            break;
                        }
                    }
                }
            }
        }

        public void Kick()
        {
            
        }

        public void SendReliable(IntPtr data, int length)
        {
            var packet = new Packet();
            packet.Create(data, length, PacketFlags.Reliable);
            _netEvent.Peer.Send(1, ref packet);
        }

        private bool _shouldRun = true;
        private Host _server;
        protected Event _netEvent;
    }
}