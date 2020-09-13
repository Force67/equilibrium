// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

using FlatBuffers;
using protocol;
using System;
using ENet;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Threading.Tasks;

namespace noda
{
    class SyncServer : NetServer
    {
        private List<Peer> _pendingUsers;
        private List<Peer> _connectedUsers;
        private RDispatcher _dispatcher;
        private readonly Config _config;
        private Logger _logger;

        public SyncServer(Config config, Logger logger) : 
            base(config.ServerPort)
        {
            _config = config;
            _logger = logger;
            _pendingUsers = new List<Peer>();
            _connectedUsers = new List<Peer>();
            _dispatcher = new RDispatcher();

            _dispatcher.connect(HandleHandshake);
        }

        // blame not having multiple inheritances for this
        public void Connect(Action<MessageRoot> method)
        {
            _dispatcher.connect(method);
        }

        public async override void OnConnection(Peer source)
        {
            _pendingUsers.Add(source);

            // verify that we - in fact - got a handshake
            await Task.Delay(_config.NetTimeOut);

            if (_pendingUsers.FindIndex(x => x.ID == source.ID) != -1)
            {
                _logger.Warning("Kicked client " + source.ID + " for not responding to handshake");
                Kick(source, DisconnectReason.TimedOut);
            }
        }

        public override void OnDisconnection(Peer source)
        {
            _logger.Info("Client disconnected - ID: " + _netEvent.Peer.ID);

            _connectedUsers.Remove(source);

            // TODO: broadcast disconnect
        }

        [MessageType(Data.Handshake)]
        private void HandleHandshake(MessageRoot message)
        {
            var hs = message.Data<Handshake>();
            if (hs.Value.Pass != _config.ServerPassword)
            {
                Kick(DisconnectReason.BadPassword);
                return;
            }

            // mark the peer as "connected"
            _pendingUsers.Remove(_netEvent.Peer);
            _connectedUsers.Add(_netEvent.Peer);

            // TODO: broadcast

            _logger.Info("User joined: - " + hs.Value.User);
        }

        public override void OnReceive(IntPtr data, int length)
        {
            byte[] managedArray = new byte[length];
            Marshal.Copy(data, managedArray, 0, length);

            ByteBuffer bb = new ByteBuffer(managedArray);
            var message = MessageRoot.GetRootAsMessageRoot(bb);
            _dispatcher.trigger(message);
        }
    }
}