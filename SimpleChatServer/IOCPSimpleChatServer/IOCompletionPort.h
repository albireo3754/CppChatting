// ����: https://github.com/jacking75/edu_cpp_IOCP/blob/master/Tutorial/01/
#pragma once

#include "Socket.h"

#define MAX_SOCKBUF 1024	//��Ŷ ũ��
#define MAX_WORKERTHREAD 4  //������ Ǯ�� ���� ������ ��

enum class IOOperation
{
	RECV,
	SEND
};

class IOCompletionPort {

public:
	IOCompletionPort() :
		mIOCPHandle(CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1))
		// MARK: - �̷��� ���� �����ڰ�?
		// mListenSocket(Socket{})
	{
		if (mIOCPHandle == NULL)
		{
			std::cout << "[Error] Fail CreateIOCompoletionPort() reason: " << GetLastError() << "\n";
			throw std::runtime_error("CreateIOCompletionPort Fail");
		}
	}

	~IOCompletionPort() {}

	bool InitSocket() {}
	
	bool Add(Socket& listenSocket) {
		return !CreateIoCompletionPort((HANDLE)listenSocket.mWinSockImpl, mIOCPHandle, (ULONG_PTR)& listenSocket, 0);
	}

	bool Start(Socket& listenSocket) {
		if (Add(listenSocket)) {
			wprintf(L"[Error] CreateIOCompletionPort on ListenSocket: %u\n", WSAGetLastError());
			WSACleanup();
			return false;
		}
	
		// TODO: ����ó�� �� �ϱ�, ������ ���� ������ ������ ���� ����
		Socket candidatedClientSocket{};

		listenSocket.AcceptEx(candidatedClientSocket);
		
		OVERLAPPED_ENTRY x[1024];
		ULONG t = 0;
		while (true) 
		{
			BOOL r = GetQueuedCompletionStatusEx(mIOCPHandle, x, 1024, &t, 1000, FALSE);
			if (!r)
			{
				std::cout << "Fail! GetQueuedCompletionStatusEx\n reason: " << GetLastError() << " + " << WSAGetLastError() << "\n";
			}
			else {
				for (int i = 0; i < t; ++i) {
					// Listen���� �������� 0������ �ʱ�ȭ ��
					if (x->lpCompletionKey == (ULONG_PTR)&listenSocket) {
						if (candidatedClientSocket.UpdateAcceptContext(listenSocket)) {
							Add(candidatedClientSocket);
							candidatedClientSocket.OverlappedReceive();
						}
						else {
							std::cout << "AcceptedSocket Update Fail with : " << WSAGetLastError() << "\n";
						}
						// TODO: - Ŭ���̾�Ʈ ������ �߰��ϱ� ���� �ڷᱸ���� �ϳ� �ʿ��� ex hash?
						//Socket candidatedClientSocket{};
						//if (listenSocket.AcceptEx(candidatedClientSocket)) {
						//	std::cout << "new Overlap!\n";
						//}
						//else {
						//	std::cout << "new Overlap fail with: " << WSAGetLastError() << "\n";
						//}
					}
					else {
						candidatedClientSocket.onReceive();
						candidatedClientSocket.OverlappedReceive();
					}
				}
				std::cout << "Listen Event count: " << t << "\n";
			}

		}

		return true;
	}
private:
	HANDLE mIOCPHandle;
};



