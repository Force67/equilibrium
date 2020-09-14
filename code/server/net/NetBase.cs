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
        public void ProcessPacket(Peer peer, byte[] data, int length);
    }

    // type annotation
    [AttributeUsage(AttributeTargets.Method)]
    public class MessageType : Attribute
    {
        public MessageType(Data data)
        {
            Id = data;
        }

        public Data GetData()
        {
            return Id;
        }

        Data Id;
    }

    public class NetDispatcher<T>
    {
        private Dictionary<Data, Action<T, MessageRoot>> _subscribers;

        public NetDispatcher()
        {
            _subscribers = new Dictionary<Data, Action<T, MessageRoot>>();
        }

        public void connect(Action<T, MessageRoot> method)
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

        public void trigger(T source, MessageRoot message)
        {
            foreach (var sub in _subscribers)
            {
                if (sub.Key == message.DataType)
                {
                    sub.Value.Invoke(source, message);
                    break;
                }
            }
        }
    }
}