#pragma once

template <typename Packet>
struct PacketRegistrar
{
    bool IsRegistered = false;

    PacketRegistrar()
    {
        Packet::Register();

        IsRegistered = true;
    }
};