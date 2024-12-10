#pragma once

#include <winsock2.h>
#include <string>
#include <vector>
#include <thread>

class TCPClient
{
	static const int32_t constexpr HeaderSize = 8 * sizeof(uint32_t);
	static const uint32_t constexpr HeaderSignature = 0xdeadbeef;
	static const uint32_t constexpr SeedXor = 0x1032399B;
	static const uint32_t constexpr RandomSequenceElementCount = 5;
	static const int32_t constexpr MaxSegmentLength = 8192;

	bool m_Connected = false;
	SOCKET m_socket;
	std::thread* m_receiveThread = nullptr;

	typedef void (*fn_receiveCallback)(const std::vector<uint8_t>& data);
	fn_receiveCallback m_ReceiveCallback = nullptr;

	typedef void (*fn_disconnectedCallback)();
	fn_disconnectedCallback m_DisconnectedCallback = nullptr;

	std::vector<uint8_t> m_ReceiveStreamData;
	bool m_IsReceiving = false;
	int m_ReceiveStreamLength;
	int m_ReceiveStreamRemainingLength;

	void receiveThread();
	void receive()
public:
	TCPClient() = default;
	TCPClient(fn_receiveCallback receiveCallback, fn_disconnectedCallback disconnectedCallback);
	~TCPClient();

	bool Connect(const std::string& host, const std::string& port);
	void Disconnect();
	void Send(const std::vector<uint8_t>& data);
};
