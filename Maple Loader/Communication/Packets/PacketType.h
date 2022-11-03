#pragma once

enum class PacketType : unsigned char
{
    Handshake = 0xA0,
    Login = 0xB0,
    LoaderStream = 0xC0
};