// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

using System;
using System.Collections.Generic;
using protocol;

namespace noda
{
    // type annotation
    [AttributeUsage(System.AttributeTargets.Method)]
    public class MessageType : System.Attribute
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

    public class RDispatcher
    {
        private Dictionary<Data, Action<MessageRoot>> _subscribers;

        public RDispatcher()
        {
            _subscribers = new Dictionary<Data, Action<MessageRoot>>();
        }

        public void connect(Action<MessageRoot> method)
        {
            var methodType = method.GetType();
            Attribute[] attrs = Attribute.GetCustomAttributes(methodType);

            foreach(var attr in attrs)
            {
                if (attr is MessageType)
                {
                    MessageType m = (MessageType)attr;
                    _subscribers.Add(m.GetData(), method);
                    break;
                }
            }
        }

        public void trigger(MessageRoot root)
        {
            foreach(var sub in _subscribers)
            {
                if (sub.Key == root.DataType)
                {
                    sub.Value.Invoke(root);
                    break;
                }
            }
        }
    }

    public class Dispatcher<T>
    {
        private List<Action<T>> _subscribers;

        public Dispatcher()
        {
            _subscribers = new List<Action<T>>();
        }

        public void connect(Action<T> act)
        {
            _subscribers.Add(act);
        }

        public void trigger(T data)
        {
            foreach(var sub in _subscribers)
            {
                sub.Invoke(data);
            }
        }
    }
}