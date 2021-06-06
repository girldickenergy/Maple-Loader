#pragma once

#include "../Crypto/rsa.h"
#include "../Crypto/osrng.h"
#include "../Crypto/hex.h"
#include "../Crypto/files.h"
#include "../Crypto/pssr.h"
#include "../Crypto/Wrapper/RSADecrypt.h"
#include <filesystem>

using CryptoPP::FileSink;
using namespace CryptoPP;

static void GenerateKey()
{
	AutoSeededRandomPool rng;

	InvertibleRSAFunction params;
	params.SetPublicExponent(65537);
	params.GenerateRandomWithKeySize(rng, 1024 * 3);

	const Integer& n = params.GetModulus();
	const Integer& p = params.GetPrime1();
	const Integer& q = params.GetPrime2();
	const Integer& d = params.GetPrivateExponent();
	const Integer& e = params.GetPublicExponent();

	std::cout << "RSA Parameters:" << std::endl;
	std::cout << " n: " << n << std::endl;
	std::cout << " p: " << p << std::endl;
	std::cout << " q: " << q << std::endl;
	std::cout << " d: " << d << std::endl;
	std::cout << " e: " << e << std::endl;
	std::cout << std::endl;

	RSA::PrivateKey privateKey(params);
	RSA::PublicKey publicKey(params);

	std::string s = std::filesystem::current_path().string() + "/private.key";
	std::string s1 = std::filesystem::current_path().string() + "/public.key";

	privateKey.Save(FileSink(s.c_str(), true).Ref());
	std::cout << " private.key written" << std::endl;
	publicKey.Save(FileSink(s1.c_str(), true).Ref());
	std::cout << " public.key written" << std::endl;
}

static void BM2()
{
	try {

		std::string s = std::filesystem::current_path().string() + "/private.key";
		std::string s1 = std::filesystem::current_path().string() + "/public.key";
		AutoSeededRandomPool rng;

		RSA::PrivateKey privateKey;
		privateKey.Load(FileSource(s.c_str(), true, NULL, true).Ref());

		RSA::PublicKey publicKey;
		publicKey.Load(FileSource(s1.c_str(), true, NULL, true).Ref());
		// Secret to protect


// Signing      
		RSASS<PSSR, SHA256>::Signer signer(privateKey);
		RSASS<PSSR, SHA256>::Verifier verifier(publicKey);

		// Setup
		byte message[] = "maple.software is superior to megumi";
		size_t messageLen = sizeof(message);

		////////////////////////////////////////////////
		// Sign and Encode
		SecByteBlock signature(signer.MaxSignatureLength(messageLen));

		size_t signatureLen = signer.SignMessageWithRecovery(rng, message,
			messageLen, NULL, 0, signature);

		// Resize now we know the true size of the signature
		signature.resize(signatureLen);

		////////////////////////////////////////////////
		// Verify and Recover
		SecByteBlock recovered(
			verifier.MaxRecoverableLengthFromSignatureLength(signatureLen)
		);

		DecodingResult result = verifier.RecoverMessage(recovered, NULL,
			0, signature, signatureLen);

		if (!result.isValidCoding) {
			throw Exception(Exception::OTHER_ERROR, "Invalid Signature");
		}

		////////////////////////////////////////////////
		// Use recovered message
		//  MaxSignatureLength is likely larger than messageLength
		recovered.resize(result.messageLength);

		std::cout << "Recovered plain text" << std::endl;

		std::string cipher, rec;
		cipher.resize(signature.size());
		rec.resize(recovered.size());
		std::memcpy(&cipher[0], &signature[0], signature.size());

		std::memcpy(&rec[0], &recovered[0], rec.size());

		std::cout << "plain: " << message << " | en: " << cipher << std::endl;
		std::cout << " | rec: " << rec << std::endl;

		RSADecrypt rsaD = RSADecrypt();
		std::string r = rsaD.Decode(cipher, signature.size());
		std::cout << std::endl << std::endl << std::endl;
		std::cout << "output from b64 wrapper: ";
		std::cout << r << std::endl;
	}
	catch (CryptoPP::Exception & e) {

		std::cout << "caught ex" << std::endl;
		std::cout << e.what() << std::endl;
	}
}

static void Benchmark()
{
	try {

		std::string s = std::filesystem::current_path().string() + "/private.key";
		std::string s1 = std::filesystem::current_path().string() + "/public.key";
		AutoSeededRandomPool rng;

		RSA::PrivateKey privateKey;
		privateKey.Load(FileSource(s.c_str(), true, NULL, true).Ref());

		RSA::PublicKey publicKey;
		publicKey.Load(FileSource(s1.c_str(), true, NULL, true).Ref());
		// Secret to protect

		std::string secret = "maple.software is superior to megumi";

		SecByteBlock plaintext(reinterpret_cast<const byte*>(&secret[0]), secret.size());

		////////////////////////////////////////////////
		// Encrypt
		RSAES_OAEP_SHA_Encryptor encryptor(publicKey);

		// Now that there is a concrete object, we can validate

		// Create cipher text space
		size_t ecl = encryptor.CiphertextLength(plaintext.size());
		SecByteBlock ciphertext(ecl);

		encryptor.Encrypt(rng, plaintext, plaintext.size(), ciphertext);

		////////////////////////////////////////////////
		// Decrypt
		RSAES_OAEP_SHA_Decryptor decryptor(privateKey);

		// Now that there is a concrete object, we can check sizes

		// Create recovered text space
		size_t dpl = decryptor.MaxPlaintextLength(ciphertext.size());
		SecByteBlock recovered(dpl);

		DecodingResult result = decryptor.Decrypt(rng,
			ciphertext, ciphertext.size(), recovered);


		// At this point, we can set the size of the recovered
		//  data. Until decryption occurs (successfully), we
		//  only know its maximum size
		recovered.resize(result.messageLength);

		std::cout << "Recovered plain text" << std::endl;

		std::string cipher, rec;
		cipher.resize(ciphertext.size());
		rec.resize(recovered.size());
		std::memcpy(&cipher[0], &ciphertext[0], ciphertext.size());
		std::memcpy(&rec[0], &recovered[0], rec.size());

		std::cout << "plain: " << secret << " | en: " << cipher << " | rec: " << rec << std::endl;
	}
	catch (CryptoPP::Exception& e) {
	
		std::cout << "caught ex" << std::endl;
		std::cout << e.what() << std::endl;
	}
}