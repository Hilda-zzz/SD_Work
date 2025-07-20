#include "NetworkSystem.hpp"
#include "../Core/ErrorWarningAssert.hpp"
#include "Engine/Core/DevConsole.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#include <WS2TCPIP.h>
#include "../Core/DevConsole.hpp"
#include "../Core/EngineCommon.hpp"
#pragma comment(lib, "Ws2_32.lib")

extern DevConsole* g_theDevConsole;

NetworkSystem::NetworkSystem(NetworkSystemConfig const& config)
	:m_networkConfig(config)
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
	switch (m_netState)
	{
	case NetState::NET_STATE_SERVER_LISTENING:
		StopServer();  
		break;

	case NetState::NET_STATE_CLIENT_CONNECTING:
	case NetState::NET_STATE_CLIENT_CONNECTED:
		StopClient(); 
		break;

	default:
		break;
	}

	memset(m_sendBuffer, 0, BUFFER_SIZE);
	memset(m_recvBuffer, 0, BUFFER_SIZE);

	int errorCode = WSACleanup();
	if (errorCode != 0)
	{
		DebuggerPrintf("WSACleanup failed with error: %d\n", errorCode);
	}

	m_netState = NetState::NET_STATE_INACTIVE;
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
	UNUSED(result);
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
	UNUSED(result);
	uint32_t serverIPAddressU32 = ntohl(inAddr.S_un.S_addr);
	uint16_t serverPortU16 = static_cast<unsigned short>(port);

	sockaddr_in addr;											
	addr.sin_family = AF_INET;										
	addr.sin_addr.S_un.S_addr = htonl(serverIPAddressU32);			
	addr.sin_port = htons(serverPortU16);							
	connect(actualSocket, (sockaddr*)(&addr), (int)sizeof(addr));

	m_netState = NetState::NET_STATE_CLIENT_CONNECTING;
}

void NetworkSystem::StopServer()
{
	if (m_netState != NetState::NET_STATE_SERVER_LISTENING)
	{
		return;  
	}

	if (m_listenSocket != static_cast<uintptr_t>(INVALID_SOCKET))
	{
		SOCKET listenSocket = static_cast<SOCKET>(m_listenSocket);
		closesocket(listenSocket);
		m_listenSocket = static_cast<uintptr_t>(INVALID_SOCKET);
	}

	for (auto clientHandle : m_clientSockets)
	{
		SOCKET clientSocket = static_cast<SOCKET>(clientHandle);
		closesocket(clientSocket);
	}
	m_clientSockets.clear();

	memset(m_sendBuffer, 0, BUFFER_SIZE);
	memset(m_recvBuffer, 0, BUFFER_SIZE);

	m_netState = NetState::NET_STATE_IDLE;
}

void NetworkSystem::StopClient()
{
	if (m_netState != NetState::NET_STATE_CLIENT_CONNECTING &&
		m_netState != NetState::NET_STATE_CLIENT_CONNECTED)
	{
		return; 
	}

	if (m_connectionToServer != static_cast<uintptr_t>(INVALID_SOCKET))
	{
		SOCKET serverSocket = static_cast<SOCKET>(m_connectionToServer);
		closesocket(serverSocket);
		m_connectionToServer = static_cast<uintptr_t>(INVALID_SOCKET);
	}

	memset(m_sendBuffer, 0, BUFFER_SIZE);
	memset(m_recvBuffer, 0, BUFFER_SIZE);

	m_netState = NetState::NET_STATE_IDLE;
}

bool NetworkSystem::SendCommandToRemote(std::string const& command)
{
	//std::string	cmdToRemote = command + "remote=true";
	if (m_netState == NetState::NET_STATE_CLIENT_CONNECTED)
	{
		return SendToServer(command);
	}
	else if (m_netState == NetState::NET_STATE_SERVER_LISTENING && HasConnectedClients())
	{
		return SendToAllClients(command);
	}
	else
	{
		return false;
	}
}

bool NetworkSystem::SendToServer(std::string const& command)
{
	if (m_netState != NetState::NET_STATE_CLIENT_CONNECTED)
		return false;

	// 将命令复制到发送缓冲区
	size_t commandLength = command.length();
	if (commandLength >= BUFFER_SIZE - 1) // 留空间给null terminator
		return false;

	strcpy_s(m_sendBuffer, BUFFER_SIZE, command.c_str());

	// 使用WinSock send函数发送到服务器
	int result = send((SOCKET)m_connectionToServer, m_sendBuffer, (int)commandLength + 1, 0);

	return (result != SOCKET_ERROR && result != WSAEWOULDBLOCK);
}

bool NetworkSystem::SendToAllClients(std::string const& command)
{
	if (m_netState != NetState::NET_STATE_SERVER_LISTENING || m_clientSockets.empty())
		return false;

	size_t commandLength = command.length();
	if (commandLength >= BUFFER_SIZE - 1)
		return false;

	strcpy_s(m_sendBuffer, BUFFER_SIZE, command.c_str());

	bool allSuccess = true;

	// 发送给所有连接的客户端
	for (size_t i = 0; i < m_clientSockets.size(); ++i)
	{
		SOCKET clientSocket = (SOCKET)m_clientSockets[i];
		int result = send(clientSocket, m_sendBuffer, (int)commandLength + 1, 0);

		if (result == SOCKET_ERROR || result == WSAEWOULDBLOCK)
		{
			allSuccess = false;
			// 可以选择移除断开的客户端
			// RemoveDisconnectedClient(i);
		}
	}

	return allSuccess;
}

void NetworkSystem::ProcessReceivedMessages()
{
	std::string receivedData(m_recvBuffer);

	if (!receivedData.empty())
	{
		// 移除可能的null终止符
		size_t nullPos = receivedData.find('\0');
		if (nullPos != std::string::npos)
		{
			receivedData = receivedData.substr(0, nullPos);
		}

		ProcessReceivedMessage(receivedData);
	}
}

void NetworkSystem::ProcessReceivedMessage(const std::string& receivedCommand)
{
	std::string fullCommand = receivedCommand + " remote=true";
	g_theDevConsole->Execute(fullCommand);
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
				ProcessReceivedMessages();
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
	SOCKET connectionSocket = static_cast<SOCKET>(m_connectionToServer);

	fd_set writeSockets;		// a list of sockets that can be written-to	
	fd_set exceptSockets;		// a list of sockets with errors	

	FD_ZERO(&writeSockets);		// like std::vector.clear()					
	FD_ZERO(&exceptSockets);	// like std::vector.clear()			

	FD_SET(connectionSocket, &writeSockets);	// like .push_back()		
	FD_SET(connectionSocket, &exceptSockets);	// like .push_back()

	timeval waitTime = { 0, 0 };
	int selectResult = select(0, NULL, &writeSockets, &exceptSockets, &waitTime);

	if (selectResult == SOCKET_ERROR)
	{
		// int error = WSAGetLastError();
		// TODO
		return;
	}
	if (selectResult == 0)
	{
		return;
	}
	if (FD_ISSET(connectionSocket, &exceptSockets))
	{
		closesocket(connectionSocket);
		m_connectionToServer = static_cast<uintptr_t>(INVALID_SOCKET);
		// TODO
		return;
	}
	if (FD_ISSET(connectionSocket, &writeSockets))
	{
		m_netState = NetState::NET_STATE_CLIENT_CONNECTED;
		// TODO
		return;
	}
}

void NetworkSystem::BeginFrameClentConnected()
{
	SOCKET serverSocket = static_cast<SOCKET>(m_connectionToServer);
	bool shouldDisconnect = false;

	if (strlen(m_sendBuffer) > 0)
	{
		int sendResult = send(serverSocket, m_sendBuffer, (int)strlen(m_sendBuffer) + 1, 0);
		if (sendResult == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			if (error == WSAEWOULDBLOCK)
			{
				// 发送会阻塞，稍后重试（保持数据在缓冲区中）
				// 这是正常情况，不需要特殊处理
			}
			else
			{
				shouldDisconnect = true;
			}
		}
		else
		{
			// success
			memset(m_sendBuffer, 0, BUFFER_SIZE);
		}
	}

	if (!shouldDisconnect)
	{
		int recvResult = recv(serverSocket, m_recvBuffer, BUFFER_SIZE - 1, 0);

		if (recvResult > 0)
		{
			// success
			m_recvBuffer[recvResult] = '\0';  
			ProcessReceivedMessages();
			// TODO: 处理接收到的数据
			//ProcessReceivedMessage(m_recvBuffer);

			memset(m_recvBuffer, 0, BUFFER_SIZE);
		}
		else if (recvResult == 0)
		{
			shouldDisconnect = true;
		}
		else
		{
			// recvResult == SOCKET_ERROR
			int error = WSAGetLastError();
			if (error == WSAEWOULDBLOCK)
			{
				// 没有数据可接收，这是正常情况
				// 不需要特殊处理
			}
			else
			{
				// 真正的接收错误，可能是连接断开
				shouldDisconnect = true;
			}
		}
	}

	if (shouldDisconnect)
	{
		closesocket(serverSocket);
		m_connectionToServer = static_cast<uintptr_t>(INVALID_SOCKET);

		m_netState = NetState::NET_STATE_IDLE;

		// TODO: 可以触发断开连接事件
		// TODO: 可以尝试自动重连
	}
}

bool NetworkSystem::IsConnected()
{
	switch (m_netState)
	{
	case NetState::NET_STATE_INACTIVE:
	case NetState::NET_STATE_IDLE:
		return false;
	case NetState::NET_STATE_SERVER_LISTENING:
		return HasConnectedClients();
	case NetState::NET_STATE_CLIENT_CONNECTING:
	case NetState::NET_STATE_CLIENT_CONNECTED:
		return true;
	default:
		return false;
	}
}

bool NetworkSystem::HasConnectedClients()
{
	if (m_netState != NetState::NET_STATE_SERVER_LISTENING)
		return false;

	return !m_clientSockets.empty();
}

NetState NetworkSystem::GetNetState()
{
	return m_netState;
}