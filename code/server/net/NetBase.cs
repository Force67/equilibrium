// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

using ENet;
using System;
using System.Collections.Generic;
using protocol;

namespace noda.net
{
    public interface NetDelegate
    {
        public void OnConnection(Peer peer);
        public void OnDisconnection(Peer peer);
        public void ProcessPacket(Peer peer, byte[] data, int length);
    }

    // type annotation
    [AttributeUsage(AttributeTargets.Method)]
    public class MessageType : Attribute
    {
        public MessageType(MsgType data)
        {
            type = data;
        }

        public MsgType GetData()
        {
            return type;
        }

        MsgType type;
    }

    public class NetDispatcher<T>
    {
        

        public NetDispatcher()
        {
            _subscribers = new Dictionary<MsgType, Action<T, Message>>();
        }

        public void connect(Action<T, Message> method)
        {
            var methodType = method.GetType();
            Attribute[] attrs = Attribute.GetCustomAttributes(methodType);

            foreach (var attr in attrs)
            {
                if (attr is MessageType)
                {
                    MessageType m = (MessageType)attr;
                    _subscribers.Add(m.GetData(), method);
                    break;
                }
            }
        }

        public void trigger(T source, Message message)
        {
            foreach (var sub in _subscribers)
            {
                if (sub.Key == message.MsgType)
                {
                    sub.Value.Invoke(source, message);
                    break;
                }
            }
        }
    }
}