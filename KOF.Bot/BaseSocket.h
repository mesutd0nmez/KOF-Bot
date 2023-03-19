#pragma once

#if defined(__linux__) || defined(__APPLE__)
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#elif _WIN32
#include <winsock2.h>
#include <Ws2ipdef.h>
#include <Ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")
#endif

#include <string>
#include <functional>
#include <cerrno>

#define FDR_UNUSED(expr){ (void)(expr); } 
#define FDR_ON_ERROR std::function<void(int)> onError = [](int errorCode){FDR_UNUSED(errorCode)}

class BaseSocket
{
    // Definitions
public:
    enum SocketType
    {
        TCP = SOCK_STREAM,
        UDP = SOCK_DGRAM
    };
    const uint32_t BUFFER_SIZE = 262144;
    sockaddr_in address;

    bool isConnected = false;
    bool isError = false;
    bool isClosed = false;

protected:
    SOCKET sock;
    static std::string ipToString(sockaddr_in addr)
    {
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(addr.sin_addr), ip, INET_ADDRSTRLEN);

        return std::string(ip);
    }

    BaseSocket(FDR_ON_ERROR, SocketType sockType = TCP, int socketId = -1)
    {
        WSADATA data;
        WSAStartup(MAKEWORD(2, 2), &data);

        if (socketId < 0)
        {
            if ((this->sock = socket(AF_INET, sockType, 0)) < 0)
            {
                onError(errno);
            }
        }
        else
        {
            this->sock = socketId;
        }
    }

    // Methods
public:
    virtual void Close() {
        if (!isConnected || isClosed) return;

        isClosed = true;
        isConnected = false;
        shutdown(this->sock, SD_BOTH);
        closesocket(this->sock);
    }

    std::string remoteAddress() { return ipToString(this->address); }
    int remotePort() { return ntohs(this->address.sin_port); }
    SOCKET fileDescriptor() const { return this->sock; }
};