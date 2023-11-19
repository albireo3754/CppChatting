// IOCPSimpleChatServer.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
// https://learn.microsoft.com/ko-kr/windows/win32/winsock/windows-sockets-error-codes-2 에러 목록

#include <iostream>
#include <memory>

#include "pch.h"
#include "IOCPNetwork.h"

const uint16_t SERVER_PORT = 27015;

class EchoServer : public IOCompletionPort {
    
public:
    void virtual OnReceive(const SocketKey key, const char const* buffer, const int bufferLen) {
        std::cout << std::string(buffer, bufferLen) << "\n";
    }
};

int main()
{
    try {
        WSAData wsaData;
        int nResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (nResult) {
            throw std::runtime_error{ "WSAStartup Fail" };
        }
        EchoServer server{};
        server.BindAndListen(SERVER_PORT);
        server.Start();
    }
    catch (std::runtime_error exp) {
        std::cout << "[Error] Server error!" << exp.what() << "\n";
    }

    std::cout << "종료";
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
