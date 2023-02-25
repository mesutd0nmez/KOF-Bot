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
		CryptoPP::SHA256().CalculateDigest(m_encryptionKey, (CryptoPP::byte*)szEncryptionKey.data(), szEncryptionKey.size());
	}

	void SetInitialVector(std::string szInitialVector)
	{
		std::string szTmpInitialVector(szInitialVector + ".abcdefgh124");
		CryptoPP::byte byTmpInitialVector[CryptoPP::SHA256::DIGESTSIZE];
		CryptoPP::SHA256().CalculateDigest(byTmpInitialVector, (CryptoPP::byte*)szTmpInitialVector.data(), szTmpInitialVector.size());
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
					new CryptoPP::StringSink(szOutput), CryptoPP::BlockPaddingSchemeDef::NO_PADDING)
			);

			std::copy(szOutput.begin(), szOutput.end(), std::back_inserter(dataout));

			return dataout.size();
		}
		catch (const CryptoPP::Exception& e)
		{
			DBG_UNREFERENCED_PARAMETER(e);
#ifdef DEBUG
			printf("Encryption failed: %s\n", e.what());
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
					new CryptoPP::StringSink(szOutput), CryptoPP::BlockPaddingSchemeDef::NO_PADDING)
			);

			std::copy(szOutput.begin(), szOutput.end(), std::back_inserter(dataout));

			return dataout.size();
		}
		catch (const CryptoPP::Exception& e)
		{
			DBG_UNREFERENCED_PARAMETER(e);
#ifdef DEBUG
			printf("Decryption failed: %s\n", e.what());
#endif
		}

		return 0;
	}

private:
	CryptoPP::byte m_encryptionKey[CryptoPP::SHA256::DIGESTSIZE];
	CryptoPP::byte m_initialVector[CryptoPP::Weak::MD5::DIGESTSIZE];


};

