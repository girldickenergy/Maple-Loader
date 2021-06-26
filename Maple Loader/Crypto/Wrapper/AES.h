#pragma once

#include <vector>
#include <random>
#include <climits>
#include <algorithm>
#include <functional>
#include "../base64.h"
#include "../rsa.h"
#include "../osrng.h"
#include "../hex.h"
#include "../files.h"
#include "../pssr.h"
#include "../modes.h"

//had to change unsigned char to unsigned long, otherwise it wouldn't compile. no idea how it compiled before, feel free to tell me why!
using genBytes = std::independent_bits_engine<std::default_random_engine, CHAR_BIT, unsigned long>;

class AES {
private:
	std::vector<unsigned char> IV = std::vector<unsigned char>(16);
	std::vector<unsigned char> Key = std::vector<unsigned char>(32);
public:
	AES();
	void GenerateRandomIV();
	void GenerateRandomKey();

	std::vector<unsigned char> DumpIV();
	std::vector<unsigned char> DumpKey();

	std::string Encrypt(std::string input);
	std::string Decrypt(std::string input);
};