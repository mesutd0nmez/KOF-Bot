#pragma once

#include "BaseSocket.h"
#include "Packet.h"
#include <vector>
#include <string>
#include <string.h>
#include <functional>
#include <thread>

class TCPSocket : public BaseSocket
{
public:
    // Event Listeners:
    std::function<void(uint8_t*, int32_t)> onMessageReceived;
    std::function<void(int32_t)> onSocketClosed;

    explicit TCPSocket(FDR_ON_ERROR, int socketId = -1) : BaseSocket(onError, TCP, socketId) {}

    // Send TCP Packages
    int Send(const char* bytes, size_t byteslength)
    {
        if (this->isClosed)
            return -1;

        int sent = 0;
        if ((sent = send(this->sock, bytes, byteslength, 0)) < 0)
        {
            perror("send");
        }
        return sent;
    }

    int Send(Packet vecBuffer) { return this->Send((char*)vecBuffer.contents(), vecBuffer.size()); }

    void Connect(std::string host, uint16_t port, std::function<void()> onConnected = []() {}, FDR_ON_ERROR)
    {
        struct addrinfo hints, * res, * it;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        // Get address info from DNS
        int status;
        if ((status = getaddrinfo(host.c_str(), NULL, &hints, &res)) != 0) {
            onError(errno);
            return;
        }

        for (it = res; it != NULL; it = it->ai_next)
        {
            if (it->ai_family == AF_INET) { // IPv4
                memcpy((void*)(&this->address), (void*)it->ai_addr, sizeof(sockaddr_in));
                break; // for now, just get first ip (ipv4).
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

        uint32_t iTcpNoDelay = 1;
        setsockopt(this->sock, 0x6, TCP_NODELAY, (const char*)&iTcpNoDelay, sizeof(iTcpNoDelay));

        u_long iBlocking = 0;
        ioctlsocket(this->sock, FIONBIO, &iBlocking);

        int iKeepAlive = 1;
        setsockopt(this->sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&iTcpNoDelay, sizeof(iTcpNoDelay));

        this->setTimeout(5);

        // Try to connect.
        if (connect(this->sock, (const sockaddr*)&this->address, sizeof(sockaddr_in)) < 0)
        {
            onError(errno);
            this->setTimeout(0);
            return;
        }

        this->setTimeout(0);

        // Connected to the server, fire the event.
        onConnected();

        // Start listening from server:
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
        std::vector<char> vecRecieveBuffer;
        std::vector<char> vecStreamBuffer(socket->BUFFER_SIZE);

        size_t iMessageLength;
        while ((iMessageLength = recv(socket->sock, &vecStreamBuffer[0], vecStreamBuffer.size(), 0)) > 0)
        {
            if (socket->isClosed)
                return;

            vecStreamBuffer.resize(iMessageLength);
            vecRecieveBuffer.insert(std::end(vecRecieveBuffer), std::begin(vecStreamBuffer), std::end(vecStreamBuffer));

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

        socket->Close();

        if (socket->onSocketClosed)
            socket->onSocketClosed(errno);

        if (socket->deleteAfterClosed && socket != nullptr)
            delete socket;
    }

    void setTimeout(int seconds)
    {
        struct timeval tv;
        tv.tv_sec = seconds;
        tv.tv_usec = 0;

        setsockopt(this->sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv));
        setsockopt(this->sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, sizeof(tv));
    }
};