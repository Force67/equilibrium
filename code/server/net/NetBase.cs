// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

using System;
using protocol;

namespace noda.net
{
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
}