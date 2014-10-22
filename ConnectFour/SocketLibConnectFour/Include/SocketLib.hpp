#if !defined( GUARD_MY_LIB )
#define GUARD_MY_LIB

#if defined(_DEBUG) && !defined(_DLL)
#pragma comment (lib, "SocketLib-mt-s-gd.lib")
#elif defined(_DEBUG) && defined(_DLL)
#pragma comment (lib, "SocketLib-mt-gd.lib")
#elif !defined(_DEBUG) && !defined(_DLL)
#pragma comment (lib, "SocketLib-mt-s.lib")
#elif !defined(_DEBUG) && defined(_DLL)
#pragma comment (lib, "SocketLib-mt.lib")
#endif


#include <array>
#include <map>
#include <string>
#include <iostream>
#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment (lib, "ws2_32.lib")

class Socket {
private:
	SOCKET hSocket;
	std::map<int,SOCKET> mAccepted;
	int clientCount;
public:
	Socket();
	~Socket();
	int bind();
	int connect(std::string ipAddress = "localhost");
	int accept();
	int listen();
	int close(int id = -1);
	std::string recv(int id = -1);
	int send(std::string msg, int id = -1);
	std::string getNameInfo(int id = -1);
	int broadcast(std::string msg);
	int serverConnections();
};

#endif