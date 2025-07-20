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

struct NetworkSystemConfig
{
	std::string m_serverIP = "127.0.0.1";    
	uint16_t m_serverPort = 3100;            

	std::string m_playerName = "Hilda";      
	std::string m_opponentName = "";         

	int m_connectionTimeoutMs = 5000;
};

class NetworkSystem
{
public:
	NetworkSystem(NetworkSystemConfig const& config);
	~NetworkSystem();

	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();

	void StartServer(int port);
	void StartClient(std::string const& serverIP, int port);

	void StopServer();
	void StopClient();

	bool SendCommandToRemote(std::string const& command);
	bool SendToServer(std::string const& command);
	bool SendToAllClients(std::string const& command);
	
	void ProcessReceivedMessages();
	void ProcessReceivedMessage(const std::string& receivedCommand);

	bool IsConnected();
	bool HasConnectedClients();

	NetState GetNetState();

	NetworkSystemConfig m_networkConfig;
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