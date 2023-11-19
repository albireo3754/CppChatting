// 참고: https://github.com/jacking75/edu_cpp_IOCP/blob/master/Tutorial/01/
#pragma once

#include "Socket.h"
#include <unordered_map>
#include <memory>
#include <string>
#include <optional>

#define MAX_SOCKBUF 1024	//패킷 크기
#define MAX_WORKERTHREAD 4  //쓰레드 풀에 넣을 쓰레드 수

enum class NetworkEvent {
	OnReceive,
	OnAccept,
	OnSend,
	OnClose
};

typedef ULONG_PTR SocketKey;

class IOCompletionPort {

public:
	IOCompletionPort() :
		mIOCPHandle(CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 4)),
		mListenSocket(std::make_unique<Socket>())
		// MARK: - 이러면 복사 생성자가?
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
				// TODO: - buf를 언제 초기화 시켜줘도되는거지?
				char* buf = new char();
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

		if (!GetQueuedCompletionStatus(mIOCPHandle, &numberOfBytes, &completionKey, (LPOVERLAPPED*)&overlapEx, 10000))
		{
			logError("GetQueuedCompletionStatus");
			return;
		}
		// TODO: - 이게 옳바른 패턴인지 확인 필요
		// TODO: 여기서 무부를 하니깐 포인터가 null로 초기화 ㅋㅋ
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
			clientSockets[completionKey] = candidatedClientSocket;
		}
		else
		{
			// MARK: - optional이긴하지만 값을 못찾을 경우 어떻게 될지 확인을 해야함
			if (auto clientSocket = getClient(completionKey))
			{
				if (numberOfBytes <= 0)
				{
					OnClose(completionKey);
					clientSockets.erase(completionKey);
					std::cout << "socket 종료" << "\n";
				}
				else
				{
					char* buffer = (*clientSocket)->OnReceive(numberOfBytes);
					OnReceive(completionKey, buffer, numberOfBytes);
					(*clientSocket)->OverlappedReceive();
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
	
		// TODO: 에러처리 더 하기, 당장은 소켓 생성에 에러날 일은 없음
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



