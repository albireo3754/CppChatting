#pragma once
#pragma comment(lib, "ws2_32")
#pragma comment(lib,"mswsock")

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <stdexcept>
#include <iostream>
#include <MSWSock.h>
#include <string>

enum class IOOperation
{
	RECV,
	SEND
};

struct OverlappedEx
{
	WSAOVERLAPPED wsaOverlapped;
	WSABUF		wsaBuf;
	IOOperation operation;
};

class Socket {
public:
	static constexpr int MaxBufLength = 1024;

	Socket() :
		mWinSockImpl(WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED)),
		mLPAcceptExImpl(nullptr),
		dwBytes(0),
		mReadFlag(0)
	{
		std::cout << this << "[Info] New Socket Will Created \n";
		memset(&mReceivedOverlappedEx, 0, sizeof(WSAOVERLAPPED));
		mReceivedOverlappedEx.wsaBuf.len = MaxBufLength;
		mReceivedOverlappedEx.wsaBuf.buf = (char*)malloc(1024);

		memset(&mSendOverlappedEx, 0, sizeof(WSAOVERLAPPED));
		mSendOverlappedEx.wsaBuf.len = MaxBufLength;
		mSendOverlappedEx.wsaBuf.buf = (char*)malloc(1024);

		acceptBuf = (char*)malloc(1024);

		if (mWinSockImpl == INVALID_SOCKET) {
			throw std::runtime_error("INVALID SOCKET");
		}
	}
	
	~Socket() {
		if (Close())
		{
			std::cout << this << "[Info] Socket Did Closed\n";
		}
		else 
		{
			std::cout << this << "[Error] Socket Did Not Closed with: " << WSAGetLastError() << "\n";
		}
		free(mReceivedOverlappedEx.wsaBuf.buf);
		free(mSendOverlappedEx.wsaBuf.buf);
		// TODO: accept Overlapped중일땐 소켓을 끊으면 곤란
		free(acceptBuf);
	}

	bool Listen() {
		if (listen(mWinSockImpl, 5) != 0)
		{
			std::cout << "[Error] Listen() Fail with: " << WSAGetLastError() << "\n";
			return false;
		}
		GUID GuidAcceptEx = WSAID_ACCEPTEX;
		return WSAIoctl(mWinSockImpl,
			SIO_GET_EXTENSION_FUNCTION_POINTER,
			&GuidAcceptEx,
			sizeof(GUID),
			&mLPAcceptExImpl,
			sizeof(mLPAcceptExImpl),
			&dwBytes,
			NULL,
			NULL) == 0;
	}

	bool AcceptEx(const Socket& socket) {
		WSAOVERLAPPED olOverlap;
		// MARK: olOverlap을 초기화 해주지 않으면 WSAGetLastError() 6번 발생
		memset(&olOverlap, 0, sizeof(olOverlap));
		BOOL acceptResult = mLPAcceptExImpl(
			mWinSockImpl, 
			socket.mWinSockImpl, 
			acceptBuf,
			// accept전용 buff size, 0으로 초기화 하면 accept 즉시 반환
			0,
			sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
			nullptr,
			&olOverlap
		);
		if (acceptResult == FALSE && WSAGetLastError() != WSA_IO_PENDING)
		{
			std::cout << "[Error] AcceptEx failed with error: " << WSAGetLastError() << "\n";
			socket.Close();
			return false;
		}
		return true;
	}

	bool UpdateAcceptContext(Socket& listenSocket)
	{
		sockaddr_in* lpLocalSockaddr;
		int lpLocalSockaddrlen = 0;
		sockaddr_in* lpRemoteSockaddr;
		int lpRemoteSockaddrlen = 0;
		constexpr int buflen = 1024;

		char buf[buflen] = {};

		GetAcceptExSockaddrs(
			buf,
			buflen - ((sizeof(SOCKADDR_IN) + 16) * 2),
			sizeof(SOCKADDR_IN) + 16,
			sizeof(SOCKADDR_IN) + 16,
			(sockaddr**)&lpLocalSockaddr,
			&lpLocalSockaddrlen,
			(sockaddr**)&lpRemoteSockaddr,
			&lpRemoteSockaddrlen
		);

		return setsockopt(mWinSockImpl, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
			(char*)&listenSocket.mWinSockImpl, sizeof(listenSocket.mWinSockImpl)) == 0;
	}

	int OverlappedReceive()
	{
		return WSARecv(mWinSockImpl, &mReceivedOverlappedEx.wsaBuf, 1, &lpNumberOfBytesRecvd, &mReadFlag, (LPOVERLAPPED)&mReceivedOverlappedEx, NULL);
	}

	bool OverlappedSend(char* buffer, int numberOfBytes)
	{
		mSendOverlappedEx.wsaBuf.buf = buffer;
		mSendOverlappedEx.wsaBuf.len = numberOfBytes;
		DWORD* send = nullptr;
		return WSASend(mWinSockImpl, &mSendOverlappedEx.wsaBuf, 1, send, mReadFlag, (LPOVERLAPPED)&mSendOverlappedEx, NULL) == 0 || WSAGetLastError() != ERROR_IO_PENDING;
	}

	bool Close() const {
		return closesocket(mWinSockImpl) == 0;
	}

	bool Bind(uint16_t port)
	{
		SOCKADDR_IN atServer;
		atServer.sin_family = AF_INET;
		atServer.sin_port = htons(port);
		atServer.sin_addr.s_addr = htonl(INADDR_ANY);

		int nResult = bind(mWinSockImpl, (SOCKADDR*)&atServer, sizeof(SOCKADDR_IN));
		if (nResult != 0)
		{
			std::cout << "[Error] Fail bind() reason: " << WSAGetLastError() << "\n";
			return false;
		}
		return true;
	}

	char* OnReceive(int length) {
		 //std::cout << "onReceive event: " << std::string(mReceivedOverlappedEx.wsaBuf.buf, length) << "\nbytes: " << length << " readflag : " << mReadFlag << "\n";
		 //mSendOverlappedEx.wsaBuf.buf = (CHAR *)std::string(mReceivedOverlappedEx.wsaBuf.buf, length).c_str();
		 //OverlappedSend();
		 return mReceivedOverlappedEx.wsaBuf.buf;
	}

	SOCKET mWinSockImpl;
private:
	LPFN_ACCEPTEX mLPAcceptExImpl;
	DWORD dwBytes;
	DWORD mReadFlag;
	DWORD lpNumberOfBytesRecvd;
	char* acceptBuf;
	OverlappedEx mReceivedOverlappedEx;
	OverlappedEx mSendOverlappedEx;
};
