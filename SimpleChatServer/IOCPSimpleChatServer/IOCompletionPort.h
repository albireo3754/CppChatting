// 참고: https://github.com/jacking75/edu_cpp_IOCP/blob/master/Tutorial/01/
#pragma once
#pragma comment(lib, "ws2_32")

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <stdexcept>
#include <iostream>
#include <MSWSock.h>
#define MAX_SOCKBUF 1024	//패킷 크기
#define MAX_WORKERTHREAD 4  //쓰레드 풀에 넣을 쓰레드 수

enum class IOOperation
{
	RECV,
	SEND
};

class IOCompletionPort {

public:
	IOCompletionPort() :
		mListenSocket(INVALID_SOCKET)
	{
		WSAData wsaData;

		int nResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (nResult) {
			throw std::runtime_error{ "WSAStartup Fail" };
		}

		mListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
		if (mListenSocket == INVALID_SOCKET) {
			throw std::runtime_error{ "ListenSocket Start Fail" };
		}
	}
	~IOCompletionPort() {}

	bool InitSocket() {}
	bool Bind(uint16_t port) 
	{
		SOCKADDR_IN atServer;
		atServer.sin_family = AF_INET;
		atServer.sin_port = htons(port);
		atServer.sin_addr.s_addr = htonl(INADDR_ANY);

		int nResult = bind(mListenSocket, (SOCKADDR*)&atServer, sizeof(SOCKADDR_IN));
		if (nResult != 0) 
		{
			std::cout << "[Error] Fail bind() reason: " << WSAGetLastError() << "\n";
			return false;
		}
		return true;
	}

	bool Listen() {
		int nResult = listen(mListenSocket, 5);
		if (nResult != 0)
		{
			std::cout << "[Error] Fail listen() reason: " << WSAGetLastError() << "\n";
			return false;
		}
		return true;
	}

	bool Start() {
		auto mIOCPHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, MAX_WORKERTHREAD);
		if (mIOCPHandle == NULL) 
		{
			std::cout << "[Error] Fail CreateIOCompoletionPort() reason: " << GetLastError() << "\n";
			return false;
		}

		LPFN_ACCEPTEX lpfnAcceptEx = nullptr;
		GUID GuidAcceptEx = WSAID_ACCEPTEX;
		WSAOVERLAPPED olOverlap;
		DWORD dwBytes;

		int iResult = WSAIoctl(mListenSocket, 
			SIO_GET_EXTENSION_FUNCTION_POINTER,
			&GuidAcceptEx, 
			sizeof(GuidAcceptEx),
			&lpfnAcceptEx, 
			sizeof(lpfnAcceptEx),
			&dwBytes, 
			NULL, 
			NULL);

		SOCKET acceptSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (acceptSocket == INVALID_SOCKET) {
			wprintf(L"Create accept socket failed with error: %u\n", WSAGetLastError());
			closesocket(mListenSocket);
			WSACleanup();
			return 1;
		}

		constexpr int outBufLen = 1024;
		char* lpOutputBuf[outBufLen];
		

		iResult = lpfnAcceptEx(mListenSocket, acceptSocket, lpOutputBuf,
			outBufLen - ((sizeof(sockaddr_in) + 16) * 2),
			sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
			&dwBytes, &olOverlap);

		if (iResult == 0) {
			std::cout << "[Error] AcceptEx failed with error: " << WSAGetLastError() << "\n";
			closesocket(mListenSocket);
			closesocket(acceptSocket);
			WSACleanup();
			return false;
		}

		return true;
	}
private:
	SOCKET mListenSocket;
};


