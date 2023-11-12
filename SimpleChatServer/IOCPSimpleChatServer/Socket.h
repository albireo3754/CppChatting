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
	SOCKET		socket;
	WSABUF		wsaBuf;
	IOOperation operation;
};

class Socket {
public:
	static constexpr int MaxReceiveLength = 1024;

	Socket() :
		mWinSockImpl(WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED)),
		mLPAcceptExImpl(nullptr),
		dwBytes(0),
		mReceiveBuffer(""),
		mReadFlag(0)
	{
		std::cout << this << "[Info] New Socket Will Created \n";
		memset(&mOverlappedEx, 0, sizeof(WSAOVERLAPPED));
		mOverlappedEx.wsaBuf.buf = mReceiveBuffer;
		mOverlappedEx.wsaBuf.len = MaxReceiveLength;
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
			sizeof(GuidAcceptEx),
			&mLPAcceptExImpl,
			sizeof(mLPAcceptExImpl),
			&dwBytes,
			NULL,
			NULL);
	}

	bool AcceptEx(const Socket& socket) {
		if (mLPAcceptExImpl == nullptr) {
			return false;
		}
		constexpr int outBufLen = 1024;
		char* lpOutputBuf[outBufLen];
		DWORD dwBytes = 0;
		// MARK: olOverlap을 초기화 해주지 않으면 WSAGetLastError() 6번 발생
		WSAOVERLAPPED olOverlap;
		memset(&olOverlap, 0, sizeof(olOverlap));
		BOOL b = mLPAcceptExImpl(mWinSockImpl, socket.mWinSockImpl, lpOutputBuf,
			outBufLen - ((sizeof(sockaddr_in) + 16) * 2),
			sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
			&dwBytes, &olOverlap);

		// LPOVERLAPPED* over;
		// MARK: 997은 accept 시작, io완료 되지 않음 상태
		if (b == FALSE && WSAGetLastError() != 997)
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
		std::cout << "buf 도착 : " << buf << "\n";

		return setsockopt(mWinSockImpl, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
			(char*)&listenSocket.mWinSockImpl, sizeof(listenSocket.mWinSockImpl)) == 0;
	}

	int OverlappedReceive()
	{
		return WSARecv(mWinSockImpl, &mOverlappedEx.wsaBuf, 1, &lpNumberOfBytesRecvd, &mReadFlag, (LPOVERLAPPED)&mOverlappedEx, NULL);
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

	void OnReceive() {
		auto received = std::string{ mReceiveBuffer };
		std::cout << "onReceive event: " << mOverlappedEx.wsaOverlapped.hEvent << "bytes: " << mReceiveBuffer << " readflag : " << mReadFlag << "string : " << received << "\n";
		//memset(mReceiveBuffer, 0, MaxReceiveLength);
	}

	SOCKET mWinSockImpl;
private:
	LPFN_ACCEPTEX mLPAcceptExImpl;
	DWORD dwBytes;
	char mReceiveBuffer[MaxReceiveLength];
	DWORD mReadFlag;
	DWORD lpNumberOfBytesRecvd;
	OverlappedEx mOverlappedEx;
};
