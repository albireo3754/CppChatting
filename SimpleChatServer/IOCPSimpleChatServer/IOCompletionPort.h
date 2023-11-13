// ����: https://github.com/jacking75/edu_cpp_IOCP/blob/master/Tutorial/01/
#pragma once

#include "Socket.h"
#include <unordered_map>
#include <memory>
#include <string>
#include <optional>

#define MAX_SOCKBUF 1024	//��Ŷ ũ��
#define MAX_WORKERTHREAD 4  //������ Ǯ�� ���� ������ ��

class IOCompletionPort {

public:
	IOCompletionPort() :
		mIOCPHandle(CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1))
		// MARK: - �̷��� ���� �����ڰ�?
		// mListenSocket(Socket{})
	{
		if (mIOCPHandle == NULL)
		{
			logError("Initial CreateIOCompoletionPort");
			throw std::runtime_error("CreateIOCompletionPort Fail");
		}
	}

	~IOCompletionPort() {}

	bool InitSocket() {}
	
	bool Add(Socket& socket) {
		return !CreateIoCompletionPort((HANDLE)socket.mWinSockImpl, mIOCPHandle, (ULONG_PTR)&socket, 0);
	}

	void Work(Socket& listenSocket, std::shared_ptr<Socket>& candidatedClientSocket) {
		OverlappedEx* overlapEx = nullptr;
		ULONG_PTR completionKey = 0;
		DWORD numberOfBytes = 0;

		if (!GetQueuedCompletionStatus(mIOCPHandle, &numberOfBytes, &completionKey, (LPOVERLAPPED*)&overlapEx, 10000))
		{
			logError("GetQueuedCompletionStatus");
			return;
		}
		Socket* key = (Socket*)completionKey;
		if (key == &listenSocket)
		{
			if (candidatedClientSocket->UpdateAcceptContext(listenSocket))
			{
				Add(*candidatedClientSocket);

				if (candidatedClientSocket->OverlappedReceive() != 0
					&& WSAGetLastError() != ERROR_IO_PENDING)
				{
					logError("OverlappedReceive");
					clientSockets.erase(candidatedClientSocket.get());
				}
			}
			else
			{
				logError("UpdateAcceptContext");
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
					std::cout << "socket ����" << "\n";
				}
				else
				{
					//std::cout << "event ����: " << numberOfBytes << "\n";
					//std::cout << "mOverlappedEx Test: " << overlapEx->wsaBuf.buf << "\n";
					(*clientSocket)->OnReceive();
					(*clientSocket)->OverlappedReceive();
				}
			}
			else {
				std::cout << "[Info] Already closed Socket: " << key << "\n";
			}
		}
	}

	bool Start(Socket& listenSocket) {
		if (Add(listenSocket)) {
			logError("CreateIOCompletionPort on ListenSocket");
			WSACleanup();
			return false;
		}
	
		// TODO: ����ó�� �� �ϱ�, ������ ���� ������ ������ ���� ����
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
	
	std::optional<std::shared_ptr<Socket>> getClient(Socket* const key) {
		if (clientSockets.find(key) == clientSockets.end())
			return std::nullopt;
		return std::optional<std::shared_ptr<Socket>>(clientSockets[key]);
	}

	void logError(const std::string& name)
	{
		std::cout << "[Error] " << name << " Fail with : " << WSAGetLastError() << "\n";
	}
};



