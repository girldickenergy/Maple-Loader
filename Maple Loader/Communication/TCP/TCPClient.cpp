#include "TCPClient.h"

#include <random>
#include <ws2tcpip.h>

void TCPClient::receiveThread()
{
    while (m_Connected)
    {
        char buffer[MaxSegmentLength];
        const int bytesReceived = recv(m_socket, buffer, MaxSegmentLength, 0);
        if (bytesReceived <= 0 || (!m_IsReceiving && bytesReceived < HeaderSize))
        {
            Disconnect();

            if (m_DisconnectedCallback)
                m_DisconnectedCallback();

            return;
        }

        if (!m_IsReceiving)
        {
            auto signature = *reinterpret_cast<uint32_t*>(buffer);
            auto seed = *reinterpret_cast<uint32_t*>(buffer + sizeof(uint32_t));

            if ((signature ^ seed) != HeaderSignature)
            {
                Disconnect();

                if (m_DisconnectedCallback)
                    m_DisconnectedCallback();

                return;
            }

            std::mt19937 random(static_cast<int32_t>(seed ^ SeedXor));
            std::uniform_int_distribution dist(INT_MIN, INT_MAX - 1);

            for (auto i = 0; i < RandomSequenceElementCount; i++)
            {
                if (*reinterpret_cast<uint32_t*>(buffer + sizeof(uint32_t) * (2 + i)) != static_cast<uint32_t>(dist(random)))
                {
                    Disconnect();

                    if (m_DisconnectedCallback)
                        m_DisconnectedCallback();

                    return;
                }
            }

            m_IsReceiving = true;
            m_ReceiveStreamLength = *reinterpret_cast<int32_t*>(buffer + HeaderSize - sizeof(int32_t)) + HeaderSize;
            m_ReceiveStreamRemainingLength = m_ReceiveStreamLength;
        }

        if (!m_IsReceiving)
            continue;

        m_ReceiveStreamRemainingLength -= (std::min)(bytesReceived, m_ReceiveStreamLength);
        m_ReceiveStreamData.insert(m_ReceiveStreamData.end(), buffer, buffer + (std::min)(bytesReceived, m_ReceiveStreamLength));

        if (m_ReceiveStreamRemainingLength != 0)
            continue;

        m_ReceiveStreamData.erase(m_ReceiveStreamData.begin(), m_ReceiveStreamData.begin() + HeaderSize);

        if (m_ReceiveCallback)
            m_ReceiveCallback(m_ReceiveStreamData);

        m_ReceiveStreamLength = 0;
        m_ReceiveStreamRemainingLength = 0;
        m_IsReceiving = false;
        m_ReceiveStreamData.clear();
    }
}

TCPClient::TCPClient(const std::function<void(const std::vector<unsigned char>&)>& receiveCallback, const std::function<void()>& disconnectedCallback)
{
	this->m_ReceiveCallback = receiveCallback;
    this->m_DisconnectedCallback = disconnectedCallback;
}

TCPClient::~TCPClient()
{
    Disconnect();
}

bool TCPClient::Connect(const std::string& host, const std::string& port)
{
    WSADATA wsaData;
    struct addrinfo* result = NULL, *ptr = NULL, hints;

    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
        return false;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
	
    iResult = getaddrinfo(host.c_str(), port.c_str(), &hints, &result);
    if (iResult != 0)
    {
        WSACleanup();

        return false;
    }
	
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
    {
        m_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (m_socket == INVALID_SOCKET)
        {
            WSACleanup();

            return false;
        }
		
        iResult = connect(m_socket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR)
        {
            closesocket(m_socket);
            m_socket = INVALID_SOCKET;

            continue;
        }

        break;
    }

    freeaddrinfo(result);

    if (m_socket == INVALID_SOCKET)
    {
        WSACleanup();

        return false;
    }

    m_receiveThread = new std::thread(&TCPClient::receiveThread, this);

    m_Connected = true;

    return true;
}

void TCPClient::Disconnect()
{
    if (m_Connected)
    {
        m_Connected = false;

        if (m_receiveThread)
        {
            m_receiveThread->detach();

            delete m_receiveThread;
            m_receiveThread = nullptr;
        }

        if (m_socket != INVALID_SOCKET)
            closesocket(m_socket);

        WSACleanup();
    }
}

void TCPClient::Send(const std::vector<unsigned char>& data)
{
    std::vector<uint8_t> packet;

    auto rd = std::random_device();

    uint32_t seed = rd();
    uint32_t signature = HeaderSignature ^ seed;
    int32_t bufferLength = data.size();

    std::mt19937 random(static_cast<int32_t>(seed ^ SeedXor));
    std::uniform_int_distribution dist(INT_MIN, INT_MAX - 1);

    std::vector<uint32_t> randomSequence;
    for (auto i = 0; i < RandomSequenceElementCount; i++)
        randomSequence.push_back(static_cast<uint32_t>(dist(random)));

    packet.insert(packet.end(), reinterpret_cast<uint8_t*>(&signature), reinterpret_cast<uint8_t*>(&signature) + sizeof(uint32_t));
    packet.insert(packet.end(), reinterpret_cast<uint8_t*>(&seed), reinterpret_cast<uint8_t*>(&seed) + sizeof(uint32_t));
    packet.insert(packet.end(), reinterpret_cast<uint8_t*>(randomSequence.data()), reinterpret_cast<uint8_t*>(randomSequence.data()) + randomSequence.size() * sizeof(uint32_t));
    packet.insert(packet.end(), reinterpret_cast<uint8_t*>(&bufferLength), reinterpret_cast<uint8_t*>(&bufferLength) + sizeof(int32_t));

    if (packet.size() != HeaderSize)
    {
        Disconnect();

        if (m_DisconnectedCallback)
            m_DisconnectedCallback();

        return;
    }

    packet.insert(packet.end(), data.begin(), data.end());

    int remainingBytes = packet.size();
    int offset = 0;
    while (remainingBytes > 0)
    {
	    const int bytesToSend = (std::min)(MaxSegmentLength, remainingBytes);

		send(m_socket, reinterpret_cast<char*>(packet.data() + offset), bytesToSend, 0);

        offset += bytesToSend;
        remainingBytes -= bytesToSend;
    }
}
