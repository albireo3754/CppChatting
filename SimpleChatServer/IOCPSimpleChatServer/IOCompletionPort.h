// 참고: https://github.com/jacking75/edu_cpp_IOCP/blob/master/Tutorial/01/
#pragma once

#include "Socket.h"
#include <unordered_map>
#include <memory>
#include <optional>

#define MAX_SOCKBUF 1024	//패킷 크기
#define MAX_WORKERTHREAD 4  //쓰레드 풀에 넣을 쓰레드 수

class IOCompletionPort {

public:
	IOCompletionPort() :
		mIOCPHandle(CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1))
		// MARK: - 이러면 복사 생성자가?
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

	std::optional<std::shared_ptr<Socket>> getClient(Socket* const key) {
		if (clientSockets.find(key) == clientSockets.end())
			return std::nullopt;
		return std::optional<std::shared_ptr<Socket>>(clientSockets[key]);
	}

	void Work(Socket& listenSocket, std::shared_ptr<Socket>& candidatedClientSocket) {
		OverlappedEx* overlapEx = nullptr;
		ULONG_PTR completionKey = 0;
		DWORD numberOfBytes = 0;

		BOOL iResult = GetQueuedCompletionStatus(mIOCPHandle, &numberOfBytes, &completionKey, (LPOVERLAPPED*)&overlapEx, 10000);
		if (iResult)
		{
			Socket* key = (Socket*)completionKey;
			if (key == &listenSocket)
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
				if (auto clientSocket = getClient(key))
				{
					if (numberOfBytes <= 0)
					{
						clientSockets.erase(key);
						std::cout << "socket 종료" << "\n";
					}
					else
					{
						std::cout << "event 도착: " << numberOfBytes << "\n";
						(*clientSocket)->OnReceive();
						(*clientSocket)->OverlappedReceive();
						std::cout << "mOverlappedEx Test: " << overlapEx->wsaBuf.buf << "\n";
					}
				}
				else {

					std::cout << "[Info] Already closed Socket: " << key << "\n";
				}
			}
		}
		else
		{
			std::cout << "Fail! GetQueuedCompletionStatus\n reason: " << GetLastError() << " + " << WSAGetLastError() << "\n";
		}
	}

	bool Start(Socket& listenSocket) {
		if (Add(listenSocket)) {
			wprintf(L"[Error] CreateIOCompletionPort on ListenSocket: %u\n", WSAGetLastError());
			WSACleanup();
			return false;
		}
	
		// TODO: 에러처리 더 하기, 당장은 소켓 생성에 에러날 일은 없음
		std::shared_ptr<Socket> candidatedClientSocket = std::make_shared<Socket>();
		listenSocket.AcceptEx(*candidatedClientSocket);
		clientSockets[candidatedClientSocket.get()] = candidatedClientSocket;

		OVERLAPPED_ENTRY events[1024];
		memset(events, 0, sizeof(OVERLAPPED_ENTRY) * 1024);
		ULONG t = 0;
		while (true) 
		{
			Work(listenSocket, candidatedClientSocket);
		}

		return true;
	}
private:
	HANDLE mIOCPHandle;
	std::unordered_map<Socket*, std::shared_ptr<Socket>> clientSockets;
};



