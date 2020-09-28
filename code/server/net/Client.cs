// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

using protocol;
using FlatBuffers;
using System;

namespace noda
{
    class Client
    {
        public Client(ENet.Peer peer, string name, string guid)
        {
            this.peer = peer;
            this.name = name;
            this.guid = guid;
        }

        public string name { get; set; }
        public string guid { get; set; }

        private ENet.Peer peer;

        public uint GetId()
        {
            return peer.ID;
        }

        public void Kick(DisconnectReason reason)
        {
            peer.Disconnect((uint)reason);
        }

        public bool SendReliable(byte[] data)
        {
            var packet = new ENet.Packet();
            packet.Create(data, ENet.PacketFlags.Reliable);

            return peer.Send(1, ref packet);
        }

        public bool SendMessage<T>(FlatBufferBuilder fbb, MsgType msgType, Offset<T> data) where T : struct
        {
            var msgRoot = Message.CreateMessage(fbb, msgType, data.Value);
            fbb.Finish(msgRoot.Value);

            bool result = SendReliable(fbb.DataBuffer.ToSizedArray() /*Do not use toFullArray*/);
            fbb.Clear();

            return result;
        }
    }
}