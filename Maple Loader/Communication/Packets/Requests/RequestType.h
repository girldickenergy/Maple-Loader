#pragma once

enum class RequestType
{
	Handshake = 0xA0,
	Login = 0xF3,
	InitialDllStream = 0xB1,
	FinalDllStream = 0xB2
};