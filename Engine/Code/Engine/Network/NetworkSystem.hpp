#pragma once
#include <vector>
#include <string>

enum class NetState
{
	NET_STATE_INACTIVE,
	NET_STATE_IDLE,
	NET_STATE_SERVER_LISTENING,
	NET_STATE_CLIENT_CONNECTING,
	NET_STATE_CLIENT_CONNECTED
};

class NetworkSystem
{
public:
	NetworkSystem();
	~NetworkSystem();

	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();

	void StartServer(int port);
	void StartClient(std::string const& serverIP, int port);
	

protected:
private:
	void BeginFrameServerListening();
	void BeginFrameClientConnecting();
	void BeginFrameClentConnected();

private:
	static constexpr int BUFFER_SIZE = 1024;

	NetState m_netState = NetState::NET_STATE_INACTIVE;
	uintptr_t m_listenSocket;
	uintptr_t m_connectionToServer;
	std::vector<uintptr_t> m_clientSockets;

	char m_sendBuffer[1024];
	char m_recvBuffer[1024];
};