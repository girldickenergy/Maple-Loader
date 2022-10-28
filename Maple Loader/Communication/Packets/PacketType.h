#pragma once

enum class PacketType : unsigned char
{
    Handshake = 0xA0,
    Login = 0xB0,
    LoaderStream = 0xC0,
    ImageStream_StageOne = 0xD0,
    ImageStream_StageTwo = 0xB1,
    Heartbeat = 0xE0
};