/*
 * Name: Katherine Haldane & Jared Lerner
 * Date: April 21, 2014
 * Description: Library that manages the socket by bind()ing the IP address and port
				to the socket server, and has a player try and connect() to your machine on a 
				port you are listen()ing to. The connection is in que to be accept()ed. The 
				client will send() and message across the socket that will be recv() in little
				endian order then sent to the server. After the close() method is called and 
				the sockets are cleaned up while the close() prevents any more reads and writes
				to the socket. 
 */
#include <SocketLib.hpp>

//Sockets class that automatically starts the WSA startup, and creation of the initial socket
Socket::Socket() {
	WSADATA wsaData;
	clientCount = 0;
	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if(iResult != NO_ERROR) {
		std::cerr << "WSAStartup failed!" << std::endl;
	}
	else {
		//Create a socket
		hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if(hSocket==INVALID_SOCKET){
			std::cerr << "Error at socket(): " << WSAGetLastError() << std::endl;
		}
	}
};

//Socket class deconstructor
Socket::~Socket() {
	close();
};


/*
 * Purpose: return the number of connections hosted by the server
 * Parameters: none
 * Return Value: int
 */
int Socket::serverConnections()
{
	return (int)mAccepted.size();
}


/*
 * Purpose: Associate a socket with an IP address & port number (by little endian).
 *			Uses the port by the kernel to maintain inoming packet to the socket descriptor.
 * Parameters: none
 * Return Value: int
 */
int Socket::bind() {
	//create a socket service
	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr("127.0.0.1");
	service.sin_port = htons(1000);
	//Binds the IP address & port number
	if(::bind(this->hSocket, (SOCKADDR*)&service, sizeof(service))==SOCKET_ERROR) {
		std::cerr<< "Bind() failed" << std::endl;
		close();
	}
	return 0;
}


/*
 * Purpose: Accept incoming connection on a listening socket.
 *			A client is trying to connect to your machine on a port
 *			you are listen()ing to.
 * Parameters: none
 * Return Value: int
 */
int Socket::listen() {
	//listen - wait for someone to try and connect - it is like config the accept: bind to this port and listen to it, has listening buffer
	if(::listen(hSocket, 1)==SOCKET_ERROR) {
		std::cerr << "Error listening on socket\n";
		close();
	}
	return 0;
}


/*
 * Purpose: Connects to the socket server through TCP/IP. This allows for 
 *			error handling, returning a summary report, and will continue trying
 *			to connect as long as there is an open connection.
 * Parameters: string
 * Return Value: int
 */
int Socket::connect(std::string ipAddress) {
	//create a socket service
	sockaddr_in service;
	service.sin_family = AF_INET;
	if ( ipAddress == "localhost" ) {
		service.sin_addr.s_addr = inet_addr("127.0.0.1");
	}
	else {
		service.sin_addr.s_addr = inet_addr(ipAddress.c_str());
	}
	service.sin_port = htons(1000);
	
	//need to create a connection first with TCP/IP (want to connect socket to service we specified and check if it errors)
	if(::connect(hSocket, (SOCKADDR*) &service, sizeof(service)) == SOCKET_ERROR){
		std::cerr << "Failed to connect" << std::endl;
		close();
	}
	return 0;
}


/*
 * Purpose: Sends data over the socket between the client and server. 
 * Parameters: string, int
 * Return Value: int 
 */
int Socket::send(std::string msg, int id) {
	char *sendbuf = new char[msg.size()+1];
	sendbuf[msg.size()] = 0;
	memcpy(sendbuf,msg.c_str(),msg.size());
	// Check if the accepted socket is empty
	if (mAccepted.empty()) {
		if(::send(hSocket, sendbuf, (int) msg.size()+1, 0) == SOCKET_ERROR) {
			std::cout << "send hAccepted..not! failed" <<hSocket<< std::endl;
			delete(sendbuf);
			return -1;
		}
	}
	else {
		if (id == -1) {
			broadcast(msg);
		}
		//Check if the message failed to send
		else if(::send(mAccepted[id], sendbuf, (int) msg.size()+1, 0) == SOCKET_ERROR) {
			std::cout << "send to mAccepted[" << id << "] failed" << mAccepted[id]<<std::endl;
			delete(sendbuf);
			return -1;
		}
	}
	delete(sendbuf);
	return 0;
}


/*
 * Purpose: Boradcasts data to the client and the user
 * Parameters: string
 * Return Value: int
 */
int Socket::broadcast(std::string msg) {
	char *sendbuf = new char[msg.size()+1];
	sendbuf[msg.size()] = 0;
	memcpy(sendbuf,msg.c_str(),msg.size());
	if (mAccepted.empty()) {
		//Check if the mesage can be broadcasted to both clients and server
		if(::send(hSocket, sendbuf, (int) msg.size()+1, 0) == SOCKET_ERROR) {
			std::cout << "send hAccepted..not! failed" <<hSocket<< std::endl;
			delete(sendbuf);
			return -1;
		}
	}
	else {
		//Report a failed connection to the clients and server
		for (auto i = mAccepted.begin(); i != mAccepted.end(); ++i) {
			if (::send(i->second, sendbuf, (int) msg.size()+1, 0) == SOCKET_ERROR) {
				std::cout << "send hAccepted! failed" << i->second<<std::endl;
				delete(sendbuf);
				return -1;
			}
		}
	}
	delete(sendbuf);
	return 0;
}


/*
 * Purpose: Accepts incoming connection on a listening socket. When the connection
			is accepted a new file descriptor is created, and the old file descriptor
			is looking for more connections.
 * Parameters: none
 * Return Value: int
 */
int Socket::accept() {
	SOCKET hAccepted = (SOCKET)SOCKET_ERROR;
	while (hAccepted == SOCKET_ERROR) {
		//accepts a connection (if you reuse hSock then your listening socket is lost/overwritten) sits and waits here for connection to occur
		hAccepted = ::accept(hSocket, NULL,NULL); 
		std::cout << "Client connected" << std::endl << " hAccpted: " << hAccepted << " hSocket: " << hSocket << std::endl;
		mAccepted[++clientCount] = hAccepted;
		return clientCount;
	}
	return -1;
}


/*
 * Purpose: Recieves a send over the socket from the client and server using little endian.
 * Parameters: int
 * Return Value: string
 */
std::string Socket::recv(int id) {
	char recvbuf[32] = "";
	if (mAccepted.empty()) {
		::recv(hSocket, recvbuf, 32, 0);
	}
	else {
		if (id == -1) {
			return "Error: No connection id specified";
		}
		//Passed the recieved data to the client and server depending on who is listen()ing
		else if (::recv(mAccepted[id], recvbuf, 32, 0) == SOCKET_ERROR) {
			mAccepted.erase(id);
		}
	}
	return recvbuf;
}

/*
 * Purpose: Prevents any more reads and writes to the socket.
 * Parameters: int
 * Return Value: int
 */
int Socket::close(int id) {
	if ( id >= 0 ) {
		closesocket(mAccepted[id]);
	}
	else {
		for (auto i = mAccepted.begin(); i != mAccepted.end(); ++i) {
			closesocket(i->second);
		}
		if (hSocket != SOCKET_ERROR)
			closesocket(hSocket);
		WSACleanup();
	}
	return 0;
}
