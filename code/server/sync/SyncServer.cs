// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

using FlatBuffers;
using netmsg;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace noda
{
    class SyncServer : NetServer
    {
        public SyncServer() : base(4523) {
            _pending = new List<Tuple<uint, uint>>();
        }

        public override void OnConnection()
        {
            Console.WriteLine("[Sync] Connection request - ID: " + _netEvent.Peer.ID);

            // mark the client as pending
            _pending.Add(Tuple.Create(_netEvent.Peer.ID, 0u));
        }

        public override void OnDisconnection()
        {
            Console.WriteLine("[Sync] Client disconnected - ID: " + _netEvent.Peer.ID);
        }

        public override void OnReceive(IntPtr data, int length)
        {
            byte[] managedArray = new byte[length];
            Marshal.Copy(data, managedArray, 0, length);

            ByteBuffer bb = new ByteBuffer(managedArray);
            var message = MessageRoot.GetRootAsMessageRoot(bb);
            if (message.DataType == Data.Handshake)
            {
                var hs = message.Data<Handshake>();
                if (hs != null)
                {
                    Console.WriteLine("[Sync] User Accepted: - " + hs.Value.User);
                }
            }
            
        }

        private List<Tuple<uint, uint>> _pending;
    }
}