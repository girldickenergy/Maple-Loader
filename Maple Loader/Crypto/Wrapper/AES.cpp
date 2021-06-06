#include "AES.h"

AES::AES()
{
}

void AES::GenerateRandomIV()
{
	genBytes gB;
	std::generate(AES::IV.begin(), AES::IV.end(), std::ref(gB));
}

void AES::GenerateRandomKey()
{
	genBytes gB;
	std::generate(AES::Key.begin(), AES::Key.end(), std::ref(gB));
}

std::vector<unsigned char> AES::DumpIV()
{
	return AES::IV;
}

std::vector<unsigned char> AES::DumpKey()
{
	return AES::Key;
}

std::string AES::Encrypt(std::string input)
{
	using namespace CryptoPP;

	if (AES::IV.empty() || AES::IV.size() <= 0)
		return "IV null";
	if (AES::Key.empty()|| AES::Key.size() <= 0)
		return "Key null";

	AutoSeededRandomPool prng;
	HexEncoder encoder(new FileSink(std::cout));

	SecByteBlock key(reinterpret_cast<const byte*>(&AES::Key[0]), AES::Key.size());
	SecByteBlock iv(reinterpret_cast<const byte*>(&AES::IV[0]), AES::IV.size());

	std::string cipher;

	try
	{
		CBC_Mode< CryptoPP::AES >::Encryption e;
		e.SetKeyWithIV(key, key.size(), iv);

		StringSource s(input, true,
			new StreamTransformationFilter(e,
				new StringSink(cipher)
			)
		);
	}
	catch (CryptoPP::Exception & e)
	{
		return e.what();
	}

	return cipher;
}

std::string AES::Decrypt(std::string input)
{
	using namespace CryptoPP;
	if (AES::IV.empty() || AES::IV.size() <= 0)
		return "IV null";
	if (AES::Key.empty() || AES::Key.size() <= 0)
		return "Key null";

	AutoSeededRandomPool prng;
	HexEncoder encoder(new FileSink(std::cout));

	SecByteBlock key(reinterpret_cast<const byte*>(&AES::Key[0]), AES::Key.size());
	SecByteBlock iv(reinterpret_cast<const byte*>(&AES::IV[0]), AES::IV.size());

	std::string recovered;

	try
	{
		CBC_Mode< AES >::Decryption d;
		d.SetKeyWithIV(key, key.size(), iv);

		StringSource s(input, true,
			new StreamTransformationFilter(d,
				new StringSink(recovered)
			)
		);

	}
	catch (CryptoPP::Exception & e)
	{
		return e.what();
	}

	return recovered;
}
