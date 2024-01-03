#include "pch.h"
#include "Socket.h"
#include "Compression.h"

Socket::Socket()
{
    m_iId = -1;
    m_iSeed = 0;
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

        OnConnect();

        m_tcpSocket.onMessageReceived = [=](uint8_t* iStream, size_t iStreamLength)
        {
            ProcessPacket(iStream, iStreamLength);
        };

        m_tcpSocket.onSocketClosed = [=]()
        {
            m_tcpSocket.isClosed = true;
            m_tcpSocket.isConnected = false;

            OnClose();
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
#ifdef VMPROTECT
    VMProtectBeginMutation("Socket::Send");
#endif

    Packet newPkt = Packet();

    newPkt << uint16_t(0xaa55);

    Packet encryptionPkt = Packet();

    if (bCompress 
         && pkt.size() >= Compression::MinBytes)
    {
        encryptionPkt
            << uint8_t(1) //compression enabled
            << uint32_t(pkt.size()); //packet size

        std::vector<uint8_t> vecOutBuffer = Compression::Compress(pkt.data());

        encryptionPkt.append(vecOutBuffer.data(), vecOutBuffer.size());
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
#ifdef DEBUG_LOG
        Print("Packet encryption failed");
#endif
        return;
    }

    newPkt.append(&vecEncryptedPacket[0], vecEncryptedPacket.size());

    newPkt << uint16_t(0x55aa);

    m_tcpSocket.Send(newPkt);

#ifdef VMPROTECT
    VMProtectEnd();
#endif
}

void Socket::ProcessPacket(uint8_t* iStream, size_t iStreamLength)
{
#ifdef VMPROTECT
    VMProtectBeginMutation("Socket::ProcessPacket");
#endif

    uint16_t iStreamHeader = *(uint16_t*)&iStream[0];

    if (iStreamHeader != 0xaa55)
    {
#ifdef DEBUG_LOG
        Print("Process packet failed, iStreamHeader != 0xaa55");
#endif
        return;
    }

    uint16_t iStreamFooter = *(uint16_t*)&iStream[iStreamLength - 2];

    if (iStreamFooter != 0x55aa)
    {
#ifdef DEBUG_LOG
        Print("Process packet failed, iStreamFooter != 0x55aa");
#endif
        return;
    }

    std::vector<uint8_t> vecDecryptedPacket;

    size_t iDecryptedPacketLength = m_Cryption->Decryption(&iStream[2], iStreamLength - 4, vecDecryptedPacket);

    if (iDecryptedPacketLength == 0)
    {
#ifdef DEBUG_LOG
        Print("Packet decryption failed");
#endif
        return;
    }

    uint8_t iFlag = *(uint8_t*)&vecDecryptedPacket[0];
    uint32_t iPacketOriginalLength = *(uint32_t*)&vecDecryptedPacket[1];

    std::vector<uint8_t> vecPacket(vecDecryptedPacket.begin() + 5, vecDecryptedPacket.end());

    if (iFlag == 1)
    {
        std::vector<uint8_t> vecDecompressedPacket = Compression::Decompress(vecPacket, iPacketOriginalLength);

        if (vecDecompressedPacket.size() > 0)
        {
            Packet pkt = Packet(vecDecompressedPacket[0], vecDecompressedPacket.size());

            if (vecDecompressedPacket.size() > 1)
            {
                pkt.append(&vecDecompressedPacket[1], vecDecompressedPacket.size() - 1);
            }

            HandlePacket(pkt);
        }
        else
        {
#ifdef DEBUG_LOG
            Print("Decompression failed, packet not handled");
#endif
        }
    }
    else
    {
        Packet pkt = Packet(vecPacket[0], vecPacket.size());

        if (vecPacket.size() > 1)
        {
            pkt.append(&vecPacket[1], vecPacket.size() - 1);
        }

        HandlePacket(pkt);
    }

#ifdef VMPROTECT
    VMProtectEnd();
#endif
}