// 참고: https://github.com/jacking75/edu_cpp_IOCP/blob/master/Tutorial/01/
#pragma once

#include "Socket.h"
#include <unordered_map>
#include <memory>
#include <string>
#include <optional>

#define MAX_SOCKBUF 1024	//패킷 크기
#define MAX_WORKERTHREAD 4  //쓰레드 풀에 넣을 쓰레드 수

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
	}

	~IOCompletionPort() {}

	bool InitSocket() {}

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
		Socket* key = (Socket*)completionKey;
		if (key == mListenSocket.get())
		{
			if (candidatedClientSocket->UpdateAcceptContext(*mListenSocket))
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
			mListenSocket->AcceptEx(*candidatedClientSocket);
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
					//std::cout << "event 도착: " << numberOfBytes << "\n";
					//std::cout << "mOverlappedEx Test: " << overlapEx->wsaBuf.buf << "\n";
					(*clientSocket)->OnReceive(numberOfBytes);
					(*clientSocket)->OverlappedReceive();
				}
			}
			else 
			{
				std::cout << "[Info] Already closed Socket: " << key << "\n";
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
		clientSockets[candidatedClientSocket.get()] = candidatedClientSocket;

		OVERLAPPED_ENTRY events[1024];
		memset(events, 0, sizeof(OVERLAPPED_ENTRY) * 1024);
		ULONG t = 0;
		while (true) 
		{
			Work(candidatedClientSocket);
		}

		return true;
	}

	//// TODO: Event헨들러처리하기
	//// 이런식으로 이벤트를 추상화해서 가상함수에다가 찔러주면 이 객체를 상속받고 있는 객체는 오버라이드만 해주면 됨
	//// 에코서버에서는 onReceived를 보고 해당클라이언트에다가 리시브 반환을 해주면 되는거고
	//// IF 세션이 끊길때 Overlapped Send중이였다면 sendBuffer를 초기화 시키지 않는 것이 중요함. 일단 보내고 그다음에 Socket을 destruct해야함.
	//void PostEvent(IOCompetionPortEvent) {
	//	switch event {
	//	case onReceive:
	//		postOnReceive();
	//		default;
	//	case onAccept:
	//		postNewSession(Accept);
	//	case onClose:
	//		postOnClose();
	//	case onReceived:
	//		messageDidReceived();
	//	}
	//}
private:
	HANDLE mIOCPHandle;
	std::unique_ptr<Socket> mListenSocket;
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



