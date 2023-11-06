// 참고: https://github.com/jacking75/edu_cpp_IOCP/blob/master/Tutorial/01/
#pragma comment(lib, "ws2_32")

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdexcept>
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
		listenSocket(INVALID_SOCKET)
	{
		WSAData wsaData;

		int nResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (nResult) {
			throw std::runtime_error{ "WSAStartup Fail" };
		}

		listenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
		if (listenSocket == INVALID_SOCKET) {
			throw std::runtime_error{ "ListenSocket Start Fail" };
		}
	}
	~IOCompletionPort() {}

	bool InitSocket() {}
	bool Bind(uint16_t port) {
		
	}
	bool Listen() {}
	bool Start() {}
private:
	SOCKET listenSocket;
};


