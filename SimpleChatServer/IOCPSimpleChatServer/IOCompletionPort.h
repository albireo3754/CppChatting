// ����: https://github.com/jacking75/edu_cpp_IOCP/blob/master/Tutorial/01/
#pragma once

#include "Socket.h"
#include <unordered_map>
#include <memory>
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
	
	bool Add(Socket& socket) {
		return !CreateIoCompletionPort((HANDLE)socket.mWinSockImpl, mIOCPHandle, (ULONG_PTR)&socket, 0);
	}

	bool Start(Socket& listenSocket) {
		if (Add(listenSocket)) {
			wprintf(L"[Error] CreateIOCompletionPort on ListenSocket: %u\n", WSAGetLastError());
			WSACleanup();
			return false;
		}
	
		// TODO: ����ó�� �� �ϱ�, ������ ���� ������ ������ ���� ����
		std::shared_ptr<Socket> candidatedClientSocket = std::make_shared<Socket>();
		listenSocket.AcceptEx(*candidatedClientSocket);
		clientSockets[candidatedClientSocket.get()] = candidatedClientSocket;

		OVERLAPPED_ENTRY x[1024];
		memset(x, 0, sizeof(OVERLAPPED_ENTRY) * 1024);
		ULONG t = 0;
		while (true) 
		{
			BOOL r = GetQueuedCompletionStatusEx(mIOCPHandle, x, 1024, &t, 100000, FALSE);
			if (!r)
			{
				std::cout << "Fail! GetQueuedCompletionStatusEx\n reason: " << GetLastError() << " + " << WSAGetLastError() << "\n";
			}
			else {
				for (int i = 0; i < t; ++i) {
					auto event = x[i];
					// Listen���� �������� 0������ �ʱ�ȭ ��
					if (x->lpCompletionKey == (ULONG_PTR)&listenSocket) 
					{
						if (candidatedClientSocket->UpdateAcceptContext(listenSocket)) 
						{
							Add(*candidatedClientSocket);

							if (candidatedClientSocket->OverlappedReceive() != 0
								&& WSAGetLastError() != ERROR_IO_PENDING)
							{
								std::cout << "[Error] overLappedReceive Fail with : " << WSAGetLastError() << "\n";
								clientSockets.erase(candidatedClientSocket.get());
							}
						}
						else 
						{
							std::cout << "AcceptedSocket Update Fail with : " << WSAGetLastError() << "\n";
							clientSockets.erase(candidatedClientSocket.get());
						}
						candidatedClientSocket = std::make_shared<Socket>();
						listenSocket.AcceptEx(*candidatedClientSocket);
						clientSockets[candidatedClientSocket.get()] = candidatedClientSocket;
					}
					else 
					{
						Socket* key = (Socket*)(x->lpCompletionKey);
						if (clientSockets.find(key) == clientSockets.end())
						{
							std::cout << "[Info] Already closed Socket: " << (Socket*)(x->lpCompletionKey) << "\n";
						} 
						else if (event.dwNumberOfBytesTransferred <= 0) 
						{
							auto clientSocket = clientSockets[key];
							clientSockets.erase(key);
							std::cout << "socket ����" << "\n";
						}
						else 
						{
							std::cout << "event ����: " << event.dwNumberOfBytesTransferred << "\n";
							auto clientSocket = clientSockets[key];
							clientSocket->OnReceive();
							clientSocket->OverlappedReceive();
						}
					}
				}
				std::cout << "Listen Event count: " << t << "\n";
			}

		}

		return true;
	}
private:
	HANDLE mIOCPHandle;
	std::unordered_map<Socket*, std::shared_ptr<Socket>> clientSockets;
};



