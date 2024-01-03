#pragma once

#include "BaseSocket.h"
#include "Packet.h"

class TCPSocket : public BaseSocket
{
public:

    std::function<void(uint8_t*, int32_t)> onMessageReceived;
    std::function<void()> onSocketClosed;

    explicit TCPSocket(FDR_ON_ERROR, int socketId = -1) : BaseSocket(onError, TCP, socketId) {}

    void Connect(std::string host, uint16_t port, std::function<void()> onConnected = []() {}, FDR_ON_ERROR)
    {
        struct addrinfo hints, * res, * it;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        int status;
        if ((status = getaddrinfo(host.c_str(), NULL, &hints, &res)) != 0) {
            onError(errno);
            return;
        }

        for (it = res; it != NULL; it = it->ai_next)
        {
            if (it->ai_family == AF_INET) { // IPv4
                memcpy((void*)(&this->address), (void*)it->ai_addr, sizeof(sockaddr_in));
                break;
            }
        }

        freeaddrinfo(res);

        this->Connect((uint32_t)this->address.sin_addr.s_addr, port, onConnected, onError);
    }
    void Connect(uint32_t ipv4, uint16_t port, std::function<void()> onConnected = []() {}, FDR_ON_ERROR)
    {
        this->address.sin_family = AF_INET;
        this->address.sin_port = htons(port);
        this->address.sin_addr.s_addr = ipv4;

        u_long iBlocking = 0;
        ioctlsocket(this->sock, FIONBIO, &iBlocking);

        setsockopt(this->sock, SOL_SOCKET, SO_SNDBUF, (const char*)&BUFFER_SIZE, sizeof(BUFFER_SIZE));
        setsockopt(this->sock, SOL_SOCKET, SO_RCVBUF, (const char*)&BUFFER_SIZE, sizeof(BUFFER_SIZE));

        this->setTimeout(5);

        if (connect(this->sock, (const sockaddr*)&this->address, sizeof(sockaddr_in)) < 0)
        {
            onError(errno);
            this->setTimeout(0);
            return;
        }

        this->setTimeout(0);

        onConnected();

        this->Listen();
    }

    void Listen()
    {
        new std::thread([this]() { TCPSocket::Receive(this); });
    }

    void setAddressStruct(sockaddr_in addr) { this->address = addr; }
    sockaddr_in getAddressStruct() const { return this->address; }

    bool deleteAfterClosed = false;

private:
    static void Receive(TCPSocket* socket)
    {
#ifdef VMPROTECT
        VMProtectBeginMutation("TcpSocket::Receive");
#endif
        try
        {
            std::vector<char> vecRecieveBuffer;
            std::vector<char> vecStreamBuffer(socket->BUFFER_SIZE);

            int iMessageLength;
            while ((iMessageLength = recv(socket->sock, &vecStreamBuffer[0], vecStreamBuffer.size(), 0)) > 0)
            {
                if (socket->isClosed)
                    break;

                if (iMessageLength > socket->BUFFER_SIZE)
                {
#ifdef DEBUG_LOG
                    Print("Received message to long, iMessageLength(%u) > BUFFER_SIZE(%u)", iMessageLength, socket->BUFFER_SIZE);
#endif
                    vecStreamBuffer.clear();
                    vecStreamBuffer.resize(socket->BUFFER_SIZE);
                    continue;
                }

                vecStreamBuffer.resize(iMessageLength);
                vecRecieveBuffer.insert(std::end(vecRecieveBuffer), std::begin(vecStreamBuffer), std::end(vecStreamBuffer));

                if (vecRecieveBuffer.size() > (size_t)socket->BUFFER_SIZE)
                {
#ifdef DEBUG_LOG
                    Print("Received buffer message to long, vecRecieveBufferSize(%u) > BUFFER_SIZE(%u)", vecRecieveBuffer.size(), socket->BUFFER_SIZE);
#endif
                    vecRecieveBuffer.clear();
                    vecStreamBuffer.clear();
                    vecStreamBuffer.resize(socket->BUFFER_SIZE);
                    continue;
                }

                if ((uint8_t)vecStreamBuffer[iMessageLength - 2] == 0xAA
                    && (uint8_t)vecStreamBuffer[iMessageLength - 1] == 0x55)
                {
                    if (socket->onMessageReceived)
                        socket->onMessageReceived((uint8_t*)vecRecieveBuffer.data(), vecRecieveBuffer.size());

                    vecRecieveBuffer.clear();
                }

                vecStreamBuffer.clear();
                vecStreamBuffer.resize(socket->BUFFER_SIZE);
            }

            if (socket != nullptr)
            {
                socket->Close();

                if (socket->onSocketClosed)
                    socket->onSocketClosed();

                if (socket->deleteAfterClosed && socket != nullptr)
                    delete socket;
            }
        }
        catch (const std::exception& e)
        {
#ifdef DEBUG_LOG
            Print("%s", e.what());
#else
            DBG_UNREFERENCED_PARAMETER(e);
#endif
        }

#ifdef VMPROTECT
        VMProtectEnd();
#endif
    }

    void setTimeout(int seconds)
    {
        struct timeval tv;
        tv.tv_sec = seconds;
        tv.tv_usec = 0;

        setsockopt(this->sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv));
        setsockopt(this->sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, sizeof(tv));
    }

public:
    int Send(const char* bytes, size_t byteslength)
    {
        if (this->isClosed)
            return -1;

        int sent = 0;
        if ((sent = send(this->sock, bytes, byteslength, 0)) < 0)
        {
            perror(skCryptDec("send"));
        }
        return sent;
    }

    int Send(Packet vecBuffer) { return this->Send((char*)vecBuffer.contents(), vecBuffer.size()); }
};