#pragma once

#include <vector>

#include "rsa.h"
#include "secblock.h"
#include "../Singleton/Singleton.h"

#include "Mha256.h"

class CryptoProvider : public Singleton<CryptoProvider>
{
	CryptoPP::RSA::PrivateKey m_RsaPrivateKey;
	CryptoPP::SecByteBlock m_AesKeyBlock;
	CryptoPP::SecByteBlock m_AesIvBlock;
	Mha256 m_Mha256;
public:
	CryptoProvider(singletonLock);

	void InitializeAES(const std::vector<unsigned char>& key, const std::vector<unsigned char>& iv);

	std::string Base64Encode(const std::vector<unsigned char>& data);
	std::vector<unsigned char> Base64Decode(const std::string& encoded);
	std::vector<unsigned char> RSADecrypt(const std::vector<unsigned char>& ciphertext);
	std::vector<unsigned char> AESEncrypt(const std::vector<unsigned char>& cleartext);
	std::vector<unsigned char> AESDecrypt(const std::vector<unsigned char>& ciphertext);

	std::vector<uint8_t> ApplyCryptoTransformations(const std::vector<uint8_t>& buffer, uint32_t key1, uint32_t key2, uint32_t key3, bool reverse = false);
	std::vector<uint8_t> ApplyRollingXor(const std::vector<uint8_t>& buffer, const std::vector<uint8_t>& key);

	__forceinline std::array<uint8_t, 32> ComputeHashMha256(const std::vector<uint8_t>& buffer) // DO NOT use outside vm macros
	{
		auto result = m_Mha256.ComputeHash(buffer);

		return result;
	}
};
