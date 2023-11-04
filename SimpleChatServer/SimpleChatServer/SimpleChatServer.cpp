// SimpleChatServer.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>
#include <WinSock2.h>
#include <ws2tcpip.h>

int main(int argc, char** argv)
{
    constexpr int DEFAULT_BUFLEN = 512;
    int iResult;
    WSAData wsaData;
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (iResult == 0) {
        std::cout << "서버 초기화 완료" << std::endl;
    }
    else {
        std::cout << "result 초기화 안됨 reason: " << iResult << std::endl;
        return 1;
    }

    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    #define DEFAULT_PORT "27015"
    SOCKET listenSocket = INVALID_SOCKET;

    // Resolve the server address and port
    iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    ptr = result;

    listenSocket = socket(ptr->ai_family, ptr->ai_socktype,
        ptr->ai_protocol);

    if (listenSocket == INVALID_SOCKET) {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    iResult = bind(listenSocket, result->ai_addr, (int)ptr->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        std::cout << "bind faailed with error" << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    iResult = listen(listenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        std::cout << "Listen failed with error: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    if (listenSocket == INVALID_SOCKET) {
        std::cout << "Unable to connect to server!\n";
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    SOCKET clientSocket = INVALID_SOCKET;

    clientSocket = accept(listenSocket, NULL, NULL);
    if (clientSocket == INVALID_SOCKET) {
        printf("accept");
        closesocket(clientSocket);
        clientSocket = INVALID_SOCKET;
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    char recvbuf[DEFAULT_BUFLEN];
    do {
        iResult = recv(clientSocket, recvbuf, DEFAULT_BUFLEN, 0);
        if (iResult > 0) {
            std::cout << "Bytes Received: " << iResult << std::endl;
            std::cout << "recv buf: " << recvbuf << std::endl;
        }
        else if (iResult == 0) {
            std::cout << "Connection Closed" << std::endl;
        }
        else {
            std::cout << "recv failed: " << WSAGetLastError() << std::endl;
        }
    } while (iResult > 0);

    std::cout << "정상 종료" << std::endl;
    return 0;
}
