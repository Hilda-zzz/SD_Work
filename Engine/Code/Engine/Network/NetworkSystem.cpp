#include "NetworkSystem.hpp"
#include "../Core/ErrorWarningAssert.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#include <WS2TCPIP.h>
#pragma comment(lib, "Ws2_32.lib")


NetworkSystem::NetworkSystem()
{
}

NetworkSystem::~NetworkSystem()
{
}

void NetworkSystem::Startup()
{
	WSADATA data;
	int errorCode = WSAStartup(MAKEWORD(2, 2), &data);
	if (errorCode != 0)
	{
		ERROR_AND_DIE("Cannot startup NetworkSystem!");
	}

	m_netState = NetState::NET_STATE_IDLE;
	m_listenSocket = static_cast<uintptr_t>(INVALID_SOCKET);
	m_connectionToServer = static_cast<uintptr_t>(INVALID_SOCKET);

	m_clientSockets.clear();
	memset(m_sendBuffer, 0, BUFFER_SIZE);
	memset(m_recvBuffer, 0, BUFFER_SIZE);
}

void NetworkSystem::Shutdown()
{
}

void NetworkSystem::BeginFrame()
{
	switch (m_netState)
	{
	case NetState::NET_STATE_INACTIVE:
		break;
	case NetState::NET_STATE_IDLE:
		break;
	case NetState::NET_STATE_SERVER_LISTENING:
		BeginFrameServerListening();
		break;
	case NetState::NET_STATE_CLIENT_CONNECTING:
		BeginFrameClientConnecting();
		break;
	case NetState::NET_STATE_CLIENT_CONNECTED:
		BeginFrameClentConnected();
		break;
	default:
		break;
	}
}

void NetworkSystem::EndFrame()
{
}

void NetworkSystem::StartServer(int port)
{
	// Todo: Need more error handle
	
	// Create a server listen socket 
	SOCKET actualSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	m_listenSocket = static_cast<uintptr_t>(actualSocket);

	// Set listen socket to non - blocking:
	unsigned long blockingMode = 1;									
	ioctlsocket(m_listenSocket, FIONBIO, &blockingMode);	

	// Bind the server listen socket to an incoming port :
	uint32_t myIPAddressU32 = INADDR_ANY;								
	uint16_t myListenPortU16 = static_cast<unsigned short>(port);
	sockaddr_in addr;											
	addr.sin_family = AF_INET;										
	addr.sin_addr.S_un.S_addr = htonl(myIPAddressU32);					
	addr.sin_port = htons(myListenPortU16);							
	int result = bind(m_listenSocket, (sockaddr*)&addr, (int)sizeof(addr)); 

	// Listen for new incoming connections on this socket:
	listen(m_listenSocket, SOMAXCONN);			

	m_netState = NetState::NET_STATE_SERVER_LISTENING;
}

void NetworkSystem::StartClient(std::string const& serverIP, int port)
{
	// Create a client socket :
	SOCKET actualSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	m_connectionToServer = static_cast<uintptr_t>(actualSocket);

	//Set client connect socket to non - blocking:
	unsigned long blockingMode = 1;									
	ioctlsocket(actualSocket, FIONBIO, &blockingMode);

	// Begin connecting to a server :
	IN_ADDR inAddr;												
	int result = inet_pton(AF_INET, serverIP.c_str(), &inAddr);
	uint32_t serverIPAddressU32 = ntohl(inAddr.S_un.S_addr);
	uint16_t serverPortU16 = static_cast<unsigned short>(port);

	sockaddr_in addr;											
	addr.sin_family = AF_INET;										
	addr.sin_addr.S_un.S_addr = htonl(serverIPAddressU32);			
	addr.sin_port = htons(serverPortU16);							
	connect(actualSocket, (sockaddr*)(&addr), (int)sizeof(addr));

	m_netState = NetState::NET_STATE_CLIENT_CONNECTING;
}

void NetworkSystem::BeginFrameServerListening()
{
	SOCKET listenSocket = static_cast<SOCKET>(m_listenSocket);
	SOCKET newClientSocket = accept(listenSocket, NULL, NULL);  
	if (newClientSocket != INVALID_SOCKET)
	{
		unsigned long blockingMode = 1;
		int ioctlResult = ioctlsocket(newClientSocket, FIONBIO, &blockingMode);
		if (ioctlResult == SOCKET_ERROR)
		{
			closesocket(newClientSocket);
		}
		else
		{
			uintptr_t clientSocketHandle = static_cast<uintptr_t>(newClientSocket);
			m_clientSockets.push_back(clientSocketHandle);
			// TODO: connection event
		}
	}
	else
	{
		int error = WSAGetLastError();
		if (error != WSAEWOULDBLOCK)  
		{
			ERROR_AND_DIE("Error in Network BeginFrameServerListening!");
		}
	}

	bool hasDataToSend = (strlen(m_sendBuffer) > 0);
	bool allSendSuccessful = true;

	for (auto it = m_clientSockets.begin(); it != m_clientSockets.end(); )
	{
		SOCKET clientSocket = static_cast<SOCKET>(*it);
		bool shouldRemoveClient = false;

		if (hasDataToSend)
		{
			int sendResult = send(clientSocket, m_sendBuffer, (int)strlen(m_sendBuffer) + 1, 0);
			if (sendResult == SOCKET_ERROR)
			{
				int error = WSAGetLastError();
				if (error == WSAEWOULDBLOCK)
				{
					allSendSuccessful = false;
				}
				else
				{
					shouldRemoveClient = true;
				}
			}
		}

		if (!shouldRemoveClient)
		{
			int recvResult = recv(clientSocket, m_recvBuffer, BUFFER_SIZE - 1, 0);
			if (recvResult > 0)
			{
				m_recvBuffer[recvResult] = '\0';
				// TODO: ProcessReceivedMessage(m_recvBuffer);
				memset(m_recvBuffer, 0, BUFFER_SIZE);
			}
			else if (recvResult == 0)
			{
				shouldRemoveClient = true;
			}
			else
			{
				int error = WSAGetLastError();
				if (error != WSAEWOULDBLOCK)
				{
					shouldRemoveClient = true;
				}
			}
		}

		if (shouldRemoveClient)
		{
			closesocket(clientSocket);
			it = m_clientSockets.erase(it); 
		}
		else
		{
			++it; 
		}
	}

	if (hasDataToSend && allSendSuccessful && !m_clientSockets.empty())
	{
		memset(m_sendBuffer, 0, BUFFER_SIZE);
	}
}

void NetworkSystem::BeginFrameClientConnecting()
{
}

void NetworkSystem::BeginFrameClentConnected()
{
}
