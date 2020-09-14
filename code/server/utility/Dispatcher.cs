// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

using System;
using System.Collections.Generic;
using protocol;

namespace noda
{
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