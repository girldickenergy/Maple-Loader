#include "RSADecrypt.h"

using namespace CryptoPP;

void RSADecrypt::decodePubKeyBase64()
{
	Base64Decoder decoder;
	decoder.Put((byte*)RSADecrypt::pubKey.data(), RSADecrypt::pubKey.size());
	decoder.MessageEnd();

	word64 size = decoder.MaxRetrievable();
	if (size && size <= SIZE_MAX)
	{
		RSADecrypt::pubKeyDecoded.resize(size);
		decoder.Get((byte*)& RSADecrypt::pubKeyDecoded[0], RSADecrypt::pubKeyDecoded.size());
	}
}

RSADecrypt::RSADecrypt() {
	RSADecrypt::pubKey = "MIIBoDANBgkqhkiG9w0BAQEFAAOCAY0AMIIBiAKCAYEArCeOWsbcPM/PC9cUtvqGvamAzcoO+D7rPscX4g0H+dyeqXkdeoNXLmZldWH0ismmtI9nqocYHjqdJH8U1wWTmPQ4tCiwznICyT9g20x+xp3SJckpCQCq666dKT4uG3Vb/Qt5PmuMZbpMYr1TTU+Xb4RM2TgpOR7xPdS4RtlUGhFbCAJs3iqnhbgyYiOy8B/XJ2LtX4/WLXmtcDcbOhi1wdXfniMiTFjYXlxqBMlaGyoiSWExHGpqUIr9q83z83LByLHoiAf41SCdpqPSRi/mCHHT4JTCrEYqAN8ya18fPYJI28JbzwHRKR4ezymF2bOb0IjBBMpjTP86OgDpHYNcuXr6ifDZNm+pcK07riWjSXj4rbyvO006I490Kn3+hHe+rlyw0j8KWyrqENPPb7Cr8kcLMOoECNc4IggB+kP+e9G3XGgwIZqes7UqdAeWDTIXC+jxBQhbpbhBu2JQhJLIEOY1fxZ9dkxQSupMjhRJTSPT2AguaChs5AnKU8pvgkZJAgER";

	RSADecrypt::decodePubKeyBase64();
}

std::vector<unsigned char> RSADecrypt::Decode(std::vector<unsigned char> input, int signatureLen)
{
	RSA::PublicKey publicKey;
	publicKey.Load(StringSource(RSADecrypt::pubKeyDecoded, true).Ref());

	RSASS<PSSR, SHA256>::Verifier verifier(publicKey);
	SecByteBlock recovered(
		verifier.MaxRecoverableLengthFromSignatureLength(signatureLen)
	);

	SecByteBlock signature(input.data(), input.size());

	DecodingResult result = verifier.RecoverMessage(recovered, NULL,
		0, signature, signatureLen);

	if (!result.isValidCoding) {
		throw Exception(Exception::OTHER_ERROR, "Invalid Signature");
	}

	recovered.resize(result.messageLength);

	std::vector<unsigned char> rec;
	rec.resize(recovered.size());
	std::memcpy(&rec[0], &recovered[0], rec.size());

	return rec;
}