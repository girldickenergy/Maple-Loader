#pragma once

#include <functional>
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

	std::function<void(const std::vector<unsigned char>&)> m_ReceiveCallback = nullptr;
	std::function<void()> m_DisconnectedCallback = nullptr;

	std::vector<uint8_t> m_ReceiveStreamData;
	bool m_IsReceiving = false;
	int m_ReceiveStreamLength;
	int m_ReceiveStreamRemainingLength;

	void receiveThread();
public:
	TCPClient() = default;
	TCPClient(const std::function<void(const std::vector<unsigned char>&)>& receiveCallback, const std::function<void()>& disconnectedCallback);
	~TCPClient();

	bool Connect(const std::string& host, const std::string& port);
	void Disconnect();
	void Send(const std::vector<uint8_t>& data);
};
