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
		auto mIOCPHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);
		if (mIOCPHandle == NULL) 
		{
			std::cout << "[Error] Fail CreateIOCompoletionPort() reason: " << GetLastError() << "\n";
			return false;
		}

		LPFN_ACCEPTEX lpfnAcceptEx = nullptr;
		GUID GuidAcceptEx = WSAID_ACCEPTEX;
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

		if (!CreateIoCompletionPort((HANDLE)mListenSocket, mIOCPHandle, (ULONG_PTR)1, 0)) {
			wprintf(L"[Error] CreateIOCompletionPort on ListenSocket: %u\n", WSAGetLastError());
			closesocket(mListenSocket);
			WSACleanup();
			return false;
		}

		SOCKET acceptSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
		if (acceptSocket == INVALID_SOCKET) {
			wprintf(L"Create accept socket failed with error: %u\n", WSAGetLastError());
			closesocket(mListenSocket);
			WSACleanup();
			return 1;
		}

		constexpr int outBufLen = 1024;
		char* lpOutputBuf[outBufLen];
		// MARK: olOverlap을 초기화 해주지 않으면 WSAGetLastError() 6번 발생
		WSAOVERLAPPED olOverlap;
		memset(&olOverlap, 0, sizeof(olOverlap));
		BOOL b = lpfnAcceptEx(mListenSocket, acceptSocket, lpOutputBuf,
			outBufLen - ((sizeof(sockaddr_in) + 16) * 2),
			sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
			&dwBytes, &olOverlap);
		
		// LPOVERLAPPED* over;
		// MARK: 997은 accept 시작, io완료 되지 않음 상태
		if (b == FALSE && WSAGetLastError() != 997) 
		{
			std::cout << "[Error] AcceptEx failed with error: " << WSAGetLastError() << "\n";
			closesocket(mListenSocket);
			closesocket(acceptSocket);
			WSACleanup();
			return false;
		}
		
		std::cout << "111";
		OVERLAPPED_ENTRY x[1024];
	
		ULONG t = 0;

		while (true) 
		{
			BOOL r = GetQueuedCompletionStatusEx(mIOCPHandle, x, 1024, &t, INFINITE, FALSE);
			if (!r)
			{
				std::cout << "Fail! GetQueuedCompletionStatusEx\n reason: " << GetLastError() << " + " << WSAGetLastError() << "\n";
			}
			else {
				for (int i = 0; i < t; ++i) {
					// Listen소켓 오버랩시 0번으로 초기화 함
					if (x->lpCompletionKey == 0) {
						std::cout << "accept 시도\n";
					}
					else {
						std::cout << "Recv\n";
					}
				}
				std::cout << "Listen Event count: " << t << "\n";
			}

		}

		return true;
	}
private:
	SOCKET mListenSocket;
};


