#pragma once
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1

#include <cryptlib.h>
#include <sha.h>
#include <md5.h>
#include <modes.h>
#include <osrng.h>

class Cryption
{
public:
	void SetEncryptionKey(std::string szEncryptionKey)
	{
		CryptoPP::SHA256().CalculateDigest(m_encryptionKey, (uint8_t*)szEncryptionKey.data(), szEncryptionKey.size());
	}

	void SetInitialVector(std::string szInitialVector)
	{
		std::string szTmpInitialVector(szInitialVector
			+ skCryptDec(".")
			+ skCryptDec("e6d1663e8b6db24d93fab5a69f64bbc0432c674fa20ee13592e7a432ac61c0ab"));
		uint8_t byTmpInitialVector[CryptoPP::SHA256::DIGESTSIZE];
		CryptoPP::SHA256().CalculateDigest(byTmpInitialVector, (uint8_t*)szTmpInitialVector.data(), szTmpInitialVector.size());
		CryptoPP::Weak::MD5().CalculateDigest(m_initialVector, byTmpInitialVector, CryptoPP::SHA256::DIGESTSIZE);
	}

	int Encryption(uint8_t* datain, size_t length, std::vector<uint8_t>& dataout)
	{
		try
		{
			std::string szOutput = "";
			CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption e(m_encryptionKey, CryptoPP::SHA256::DIGESTSIZE, m_initialVector);
			CryptoPP::ArraySource encryptor(&datain[0], length, true,
				new CryptoPP::StreamTransformationFilter(e,
					new CryptoPP::StringSink(szOutput), CryptoPP::BlockPaddingSchemeDef::DEFAULT_PADDING)
			);

			std::copy(szOutput.begin(), szOutput.end(), std::back_inserter(dataout));

			return dataout.size();
		}
		catch (const CryptoPP::Exception& e)
		{
#ifdef DEBUG
			printf("%s\n", e.what());
#else
			DBG_UNREFERENCED_PARAMETER(e);
#endif
		}

		return 0;
	}

	int Decryption(uint8_t* datain, size_t length, std::vector<uint8_t>& dataout)
	{
		try
		{
			std::string szOutput = "";
			CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption d(m_encryptionKey, CryptoPP::SHA256::DIGESTSIZE, m_initialVector);
			CryptoPP::ArraySource encryptor(&datain[0], length, true,
				new CryptoPP::StreamTransformationFilter(d,
					new CryptoPP::StringSink(szOutput), CryptoPP::BlockPaddingSchemeDef::DEFAULT_PADDING)
			);

			std::copy(szOutput.begin(), szOutput.end(), std::back_inserter(dataout));

			return dataout.size();
		}
		catch (const CryptoPP::Exception& e)
		{
#ifdef DEBUG
			printf("%s\n", e.what());
#else
			DBG_UNREFERENCED_PARAMETER(e);
#endif
		}

		return 0;
	}

private:
	uint8_t m_encryptionKey[CryptoPP::SHA256::DIGESTSIZE];
	uint8_t m_initialVector[CryptoPP::Weak::MD5::DIGESTSIZE];


};

