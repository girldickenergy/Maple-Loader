#pragma once

#include "../Crypto/rsa.h"
#include "../Crypto/osrng.h"
#include "../Crypto/hex.h"
#include "../Crypto/files.h"

using namespace CryptoPP;

static void GenerateKey()
{
	AutoSeededRandomPool rng;

	InvertibleRSAFunction params;
	params.GenerateRandomWithKeySize(rng, 3072);

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

	HexEncoder privFile(new FileSink("private.key"));
	privateKey.Save(privFile);
	privFile.MessageEnd();

	std::cout << " private.key written" << std::endl;

	HexEncoder pubFile(new FileSink("public.key"));
	publicKey.Save(pubFile);
	pubFile.MessageEnd();

	std::cout << " public.key written" << std::endl;
}