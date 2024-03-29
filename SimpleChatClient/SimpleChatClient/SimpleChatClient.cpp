﻿// SimpleChatClient.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

int main(int argc, char ** argv)
{
    int startUpResult;
    WSAData wsaData;
    startUpResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (startUpResult == 0) {
        std::cout << "클라이언트 초기화 완료" << std::endl;
    }
    else {
        std::cout << "rfsflt 초기화 안됨 reason: " << startUpResult << std::endl;
        return 1;
    }

    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    #define DEFAULT_PORT "27015"
    //#define DEFAULT_PORT "5555"
    SOCKET connectSocket = INVALID_SOCKET;
    
    // Resolve the server address and port
    int iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    ptr = result;

    connectSocket = socket(ptr->ai_family, ptr->ai_socktype,
        ptr->ai_protocol);

    if (connectSocket == INVALID_SOCKET) {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    iResult = connect(connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
    std::cout << "connect completed\n";
    if (iResult == SOCKET_ERROR) {
        std::cout << "Unable to connect to server! with: " << WSAGetLastError() << "\n";
        closesocket(connectSocket);
        connectSocket = INVALID_SOCKET;
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    constexpr int recvbuflen = 512;
    int count = 0;
    char sendbuf[512] = "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book. It has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged.\n";
    char recvbuf[recvbuflen];
    //std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    //std::cout << std::this_thread::get_id() << "\n";
    while (count < 10) 
    {
        count++;
        sendbuf[count] = '1';
        iResult = send(connectSocket, sendbuf, (int)strlen(sendbuf), 0);
        if (iResult == SOCKET_ERROR) {
            std::cout << "send failed" << WSAGetLastError() << std::endl;
            closesocket(connectSocket);
            freeaddrinfo(result);
            WSACleanup();
            return 1;
        }
        else 
        {
            iResult = recv(connectSocket, recvbuf, recvbuflen, 0);
            std::cout << "recv success\n";
            std::cout << std::string(recvbuf, recvbuflen) << "success\n";
        }
    }

    iResult = shutdown(connectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed: %d\n", WSAGetLastError());
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);
    closesocket(connectSocket);
    WSACleanup();
    return 0;
}