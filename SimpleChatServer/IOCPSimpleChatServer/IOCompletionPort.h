// ����: https://github.com/jacking75/edu_cpp_IOCP/blob/master/Tutorial/01/
#pragma once

#include "Socket.h"
#include <unordered_map>
#include <memory>
#include <string>
#include <optional>
#include <queue>

#define MAX_SOCKBUF 1024	//��Ŷ ũ��
#define MAX_WORKERTHREAD 4  //������ Ǯ�� ���� ������ ��

enum class NetworkEvent {
	OnReceive,
	OnAccept,
	OnSend,
	OnClose
};

struct SendTask {
	SocketKey key;
	std::unique_ptr<char> buffer;
	int bufferLen;
};

typedef ULONG_PTR SocketKey;

class IOCompletionPort {

public:
	IOCompletionPort() :
		mIOCPHandle(CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 4)),
		mListenSocket(std::make_unique<Socket>())
		// MARK: - �̷��� ���� �����ڰ�?
		// mListenSocket(Socket{})
	{
		if (mIOCPHandle == NULL)
		{
			logError("Initial CreateIOCompoletionPort");
			throw std::runtime_error("CreateIOCompletionPort Fail");
		}
		else {
			std::cout << "Success CreateIOCompletionPort\n";
		}
	}

	~IOCompletionPort() {}

	void virtual OnConnect(const SocketKey key) {}
	void virtual OnReceive(const SocketKey key, const char const* buffer, const int numberOfBytes) {}
	void virtual OnClose(const SocketKey key) {}

	bool Send(const SocketKey key, const char const* buffer, const int numberOfBytes) {
		if (auto clientSocket = getClient(key))
		{
			if (numberOfBytes <= 0)
			{
				return false;
			}
			else
			{
				// TODO: - buf�� ���� �ʱ�ȭ �����൵�Ǵ°���?
				char* buf = new char[numberOfBytes];
				memcpy(buf, buffer, numberOfBytes);
				return (*clientSocket)->OverlappedSend(buf, numberOfBytes);
			}
		}
		return false;
	}

	bool BindAndListen(int port) {
		if (!mListenSocket->Bind(port) || !mListenSocket->Listen()) {
			return 1;
		}
	}

	bool Add(Socket& socket) {
		return !CreateIoCompletionPort((HANDLE)socket.mWinSockImpl, mIOCPHandle, (ULONG_PTR)&socket, 0);
	}

	void Work(std::shared_ptr<Socket>& candidatedClientSocket) {
		OverlappedEx* overlapEx = nullptr;
		ULONG_PTR completionKey = 0;
		DWORD numberOfBytes = 0;
		
		if (!GetQueuedCompletionStatus(mIOCPHandle, &numberOfBytes, &completionKey, (LPOVERLAPPED*)&overlapEx, INT32_MAX))
		{
			logError("GetQueuedCompletionStatus");
			return;
		}

		if (completionKey == makeSocketKey(mListenSocket))
		{
			if (candidatedClientSocket->UpdateAcceptContext(*mListenSocket))
			{
				Add(*candidatedClientSocket);
				OnConnect(makeSocketKey(candidatedClientSocket));
				if (candidatedClientSocket->OverlappedReceive() != 0
					&& WSAGetLastError() != ERROR_IO_PENDING)
				{
					logError("OverlappedReceive");
					clientSockets.erase(makeSocketKey(candidatedClientSocket));
				}
			}
			else
			{
				logError("UpdateAcceptContext");
				clientSockets.erase(makeSocketKey(candidatedClientSocket));
			}
			candidatedClientSocket = std::make_shared<Socket>();
			mListenSocket->AcceptEx(*candidatedClientSocket);
			clientSockets[makeSocketKey(candidatedClientSocket)] = candidatedClientSocket;
		}
		else
		{
			if (auto clientSocket = getClient(completionKey))
			{
				if (numberOfBytes <= 0)
				{
					OnClose(completionKey);
					clientSockets.erase(completionKey);
					std::cout << completionKey << "socket ����" << "\n";
				}
				else
				{
					switch (overlapEx->operation) {
					case IOOperation::RECV: {
						char* buffer = (*clientSocket)->OnReceive(numberOfBytes);
						OnReceive(completionKey, buffer, numberOfBytes);
						(*clientSocket)->OverlappedReceive();
						break;
					}
					case IOOperation::SEND: {
						// sendQueue�� ���ư��� �����尡 �ְ� �ش� �����忡���� ��Ŀ�� ������� �����ϸ� ���� �� ����. �׷��� ������ sendQeueu�� �ִ� ������ �ϳ��� cpu�� ��� �����ϸ鼭 ���ư��߸� �Ѵٴ� ������ �ִ�.
						//sendQueue.pop();
						break;
					}
					default:
						logError("Unknown Overlapped IOOperation");
						break;
					}
				}
			}
			else 
			{
				std::cout << "[Info] Already closed Socket: " << candidatedClientSocket << "\n";
			}
		}
	}

	bool Start() {
		if (Add(*mListenSocket)) {
			logError("CreateIOCompletionPort on ListenSocket");
			WSACleanup();
			return false;
		}
	
		// TODO: ����ó�� �� �ϱ�, ������ ���� ������ ������ ���� ����
		std::shared_ptr<Socket> candidatedClientSocket = std::make_shared<Socket>();
		if (!mListenSocket->AcceptEx(*candidatedClientSocket))
		{
			logError("AcceptEx");
		}
		clientSockets[makeSocketKey(candidatedClientSocket)] = candidatedClientSocket;

		OVERLAPPED_ENTRY events[1024];
		memset(events, 0, sizeof(OVERLAPPED_ENTRY) * 1024);
		ULONG t = 0;
		while (true) 
		{
			Work(candidatedClientSocket);
		}

		return true;
	}

private:
	HANDLE mIOCPHandle;
	std::unique_ptr<Socket> mListenSocket;
	std::unordered_map<SocketKey, std::shared_ptr<Socket>> clientSockets;
	std::queue<SendTask> sendQueue;
	
	std::optional<std::shared_ptr<Socket>> getClient(const SocketKey& key) {
		if (clientSockets.find(key) == clientSockets.end())
			return std::nullopt;
		return std::optional<std::shared_ptr<Socket>>(clientSockets[key]);
	}

	void logError(const std::string& name)
	{
		std::cout << "[Error] " << name << " Fail with : " << WSAGetLastError() << "\n";
	}

	SocketKey makeSocketKey(const std::shared_ptr<Socket>& socket) {
		return (ULONG_PTR)socket.get();
	}

	SocketKey makeSocketKey(const std::unique_ptr<Socket>& socket) {
		return (ULONG_PTR)socket.get();
	}
};



