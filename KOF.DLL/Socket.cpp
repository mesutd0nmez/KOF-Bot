#include "pch.h"
#include "Socket.h"
#include "Compression.h"

Socket::Socket()
{
    m_iId = -1;
    m_iSeed = 0;
    m_Compression = new Compression();
    m_Cryption = new Cryption();
}

Socket::~Socket()
{
    Close();
}

void Socket::Connect(std::string szIP, uint16_t iPort)
{
    m_tcpSocket.Connect(szIP, iPort, [&]
    {
        m_tcpSocket.isConnected = true;

        m_Cryption->SetEncryptionKey("abcdefghlmnoprs");
        GenerateSeed((1881 * 1923) / 1993 << 16);
        m_Cryption->SetInitialVector(std::to_string(GetSeed()));

        OnConnect();

        m_tcpSocket.onMessageReceived = [=](uint8_t* iStream, size_t iStreamLength)
        {
            ProcessPacket(iStream, iStreamLength);
        };

        m_tcpSocket.onSocketClosed = [=](int32_t iErrorCode)
        {
            m_tcpSocket.isClosed = true;
            m_tcpSocket.isConnected = false;

            OnClose(iErrorCode);
        };
    },
    [=](int32_t iErrorCode)
    {
        m_tcpSocket.isError = true;
        m_tcpSocket.isConnected = false;

        OnError(iErrorCode);
    });
}

void Socket::GenerateSeed(uint32_t x)
{
    uint32_t z = (x += 0x6D2B79F5UL);
    z = (z ^ (z >> 15)) * (z | 1UL);
    z ^= z + (z ^ (z >> 7)) * (z | 61UL);
    m_iSeed = z ^ (z >> 14);
}

void Socket::Close()
{
    m_tcpSocket.Close();
}

void Socket::Send(Packet& pkt, bool bCompress)
{
    Packet newPkt = Packet();

    newPkt << uint16_t(0xaa55);

    Packet encryptionPkt = Packet();

    if (bCompress)
    {
        encryptionPkt
            << uint8_t(1) //compression enabled
            << uint32_t(pkt.size()); //packet size

        size_t iCompressedStreamSize = snappy_max_compressed_length(pkt.size());
        uint8_t* iCompressedStream = new uint8_t[iCompressedStreamSize];

        if (!m_Compression->Compress((const char*)pkt.contents(), pkt.size(), (char*)iCompressedStream, &iCompressedStreamSize))
        {
#ifdef DEBUG
            printf("Snappy compression failed\n");
#endif
            return;
        }

        encryptionPkt << uint32_t(iCompressedStreamSize); //compressed packet size

        encryptionPkt.append(iCompressedStream, iCompressedStreamSize);
    }
    else
    {
        encryptionPkt
            << uint8_t(0) //compression disabled
            << uint32_t(pkt.size()); //packet size

        encryptionPkt.append(pkt.contents(), pkt.size()); //raw data
    }

    std::vector<uint8_t> vecEncryptedPacket;
    size_t iEncryptionPacketLength = m_Cryption->Encryption(encryptionPkt.contents(), encryptionPkt.size(), vecEncryptedPacket);

    if (iEncryptionPacketLength == 0)
    {
#ifdef DEBUG
        printf("Packet encryption failed\n");
#endif
        return;
    }

    newPkt.append(&vecEncryptedPacket[0], vecEncryptedPacket.size());

    newPkt << uint16_t(0x55aa);

    m_tcpSocket.Send(newPkt);
}

void Socket::ProcessPacket(uint8_t* iStream, size_t iStreamLength)
{
    if (iStreamLength < 10)
    {
#ifdef DEBUG
        printf("Process packet failed, packet size need minimum 9\n");
#endif
        return;
    }

    uint16_t iStreamHeader = *(uint16_t*)&iStream[0];

    if (iStreamHeader != 0xaa55)
    {
#ifdef DEBUG
        printf("Process packet failed, iStreamHeader != 0xaa55\n");
#endif
        return;
    }

    uint16_t iStreamFooter = *(uint16_t*)&iStream[iStreamLength - 2];

    if (iStreamFooter != 0x55aa)
    {
#ifdef DEBUG
        printf("Process packet failed, iStreamFooter != 0x55aa\n");
#endif
        return;
    }

    std::vector<uint8_t> vecDecryptedPacket;

    size_t iDecryptedPacketLength = m_Cryption->Decryption(&iStream[2], iStreamLength - 4, vecDecryptedPacket);

    if (iDecryptedPacketLength == 0)
    {
#ifdef DEBUG
        printf("Packet decryption failed\n");
#endif
        return;
    }

    uint8_t iFlag = *(uint8_t*)&vecDecryptedPacket[0];
    uint32_t iPacketLength = *(uint32_t*)&vecDecryptedPacket[1];

    uint8_t* iPacket;

    if (iFlag == 1)
    {
        uint32_t iPacketCompressedLength = *(uint32_t*)&vecDecryptedPacket[5];

        //iFlag(uint8_t) + iPacketLength(uint32_t) + iPacketCompressedLength(uint32_t)
        uint32_t iCalculatedPacketLength = iPacketCompressedLength
            + sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint32_t);

        if (iDecryptedPacketLength != iCalculatedPacketLength)
        {
#ifdef DEBUG
            printf("Invalid packet size: Flag(%d), iDecryptedPacketLength(%d) != iCalculatedPacketLength(%d)\n", iFlag, iDecryptedPacketLength, iCalculatedPacketLength);
#endif
            return;
        }

        iPacket = new uint8_t[iPacketLength];

        if (!m_Compression->UnCompress((const char*)&vecDecryptedPacket[9], iPacketCompressedLength, (char*)iPacket, &iPacketLength))
        {
#ifdef DEBUG
            printf("Snappy decompression failed\n");
#endif
            return;
        }
    }
    else
    {
        //iFlag(uint8_t) + iPacketLength(uint32_t)
        uint32_t iCalculatedPacketLength = iPacketLength
            + sizeof(uint8_t) + sizeof(uint32_t);

        if (iDecryptedPacketLength != iCalculatedPacketLength)
        {
#ifdef DEBUG
            printf("Invalid packet size: Flag(%d), iDecryptedPacketLength(%d) != iCalculatedPacketLength(%d)\n", iFlag, iDecryptedPacketLength, iCalculatedPacketLength);
#endif
            return;
        }

        iPacket = new uint8_t[iPacketLength];
        iPacket = (uint8_t*)&vecDecryptedPacket[5];
    }

    Packet pkt = Packet(iPacket[0], iPacketLength);

    if (iPacketLength > 1)
    {
        pkt.append(&iPacket[1], iPacketLength - 1);
    }

    HandlePacket(pkt);
}