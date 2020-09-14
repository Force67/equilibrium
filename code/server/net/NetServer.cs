// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

using System;
using System.Threading;
using System.Runtime.InteropServices;

namespace noda.net
{
    class NetServer
    {
        private bool shouldDie = false;
        private ENet.Host host;
        protected ENet.Event netEvent;
        private NetDelegate netDelegate;
        private Thread thread;

        public NetServer(NetDelegate ndelegate, ushort port)
        {
            netDelegate = ndelegate;

            ENet.Library.Initialize();

            host = new ENet.Host();
            var addr = new ENet.Address();

            addr.Host = 0;
            addr.Port = port;
            host.Create(addr, 10);

            thread = new Thread(Update);
            thread.Start();
        }

        ~NetServer()
        {
            host.Dispose();
            ENet.Library.Deinitialize();
        }

        public void RequestDeath()
        {
            shouldDie = true;
        }

        private void Update()
        {
            while (!shouldDie)
            {
                if (host.Service(0, out netEvent) > 0)
                {
                    switch (netEvent.Type)
                    {
                        case ENet.EventType.Connect:
                            //OnConnection(_netEvent.Peer);
                            break;
                        case ENet.EventType.Disconnect:
                            //OnDisconnection(_netEvent.Peer);
                            netEvent.Peer.Reset();
                            break;
                        case ENet.EventType.Receive: {
                            var length = netEvent.Packet.Length;
                            // convert the data to managed
                            byte[] managedArray = new byte[length];
                            Marshal.Copy(netEvent.Packet.Data, managedArray, 0, length);

                            netDelegate.ProcessPacket(netEvent.Peer, managedArray, length);
                            break;
                        }
                    }
                }

                Thread.Sleep(1);
            }
        }

        /*
         * auto msgRoot = protocol::CreateMessageRoot(mb, type, data.Union());
		mb.Finish(msgRoot);

		return SendReliable(mb.GetBufferPointer(), mb.GetSize());
         */

        public void SendReliable(IntPtr data, int length)
        {
            var packet = new ENet.Packet();
            packet.Create(data, length, ENet.PacketFlags.Reliable);
            netEvent.Peer.Send(1, ref packet);
        }
    }
}