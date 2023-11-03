// SimpleChatClient.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>
#include <WinSock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

int main(int argc, char ** argv)
{
    int startUpResult;
    WSAData wsaData;
    startUpResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (startUpResult == 0) {
        std::cout << "result 초기화 완료" << std::endl;
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
    if (iResult == SOCKET_ERROR) {
        std::cout << "Unable to connect to server!\n";
        closesocket(connectSocket);
        connectSocket = INVALID_SOCKET;
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    constexpr int recvbuflen = 512;

    const char* sendbuf = "this is a test";
    char recvbuf[recvbuflen];

    iResult = send(connectSocket, sendbuf, (int)strlen(sendbuf), 0);
    if (iResult == SOCKET_ERROR) {
        std::cout << "send failed" << WSAGetLastError() << std::endl;
        closesocket(connectSocket);
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }


    freeaddrinfo(result);

    WSACleanup();
    return 0;
}

// 프로그램 실행: <Ctrl+F5> 또는 [디버그] > [디버깅하지 않고 시작] 메뉴
// 프로그램 디버그: <F5> 키 또는 [디버그] > [디버깅 시작] 메뉴

// 시작을 위한 팁: 
//   1. [솔루션 탐색기] 창을 사용하여 파일을 추가/관리합니다.
//   2. [팀 탐색기] 창을 사용하여 소스 제어에 연결합니다.
//   3. [출력] 창을 사용하여 빌드 출력 및 기타 메시지를 확인합니다.
//   4. [오류 목록] 창을 사용하여 오류를 봅니다.
//   5. [프로젝트] > [새 항목 추가]로 이동하여 새 코드 파일을 만들거나, [프로젝트] > [기존 항목 추가]로 이동하여 기존 코드 파일을 프로젝트에 추가합니다.
//   6. 나중에 이 프로젝트를 다시 열려면 [파일] > [열기] > [프로젝트]로 이동하고 .sln 파일을 선택합니다.
