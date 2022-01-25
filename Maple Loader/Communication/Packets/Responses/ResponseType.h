#pragma once

enum class ResponseType : unsigned char
{
	FatalError = 0xE0,
	Handshake = 0xA0,
	Login = 0xF3,
	InitialDllStream = 0xB1,
	FinalDllStream = 0xB2
};
