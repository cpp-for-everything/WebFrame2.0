#ifndef ISERVER_H
#define ISERVER_H

#include <boot_server/platform.h>
#include <utils/queue.h>
#include <boot_server/protocol_manager.h>
#include <chrono>

namespace boot
{
	enum ServerStatus
	{
		NOT_STARTED,
		RUNNING,
		TERMINATING
	};

	enum class ClientStatus
	{
		CLOSING,
		HANDLING,
		WAITING
	};

	class ClientManager
	{
		thread_safe::TSQueue<SOCKET> clients;
		std::unordered_map<SOCKET, ClientStatus> clientStatus;
		std::unordered_map<SOCKET, std::chrono::time_point<std::chrono::system_clock>> clientLastActivity;
		std::mutex m;

	public:
		SOCKET fetch_client()
		{
			clients.wait_for_element();
			SOCKET client = clients.pop();
			return client;
		}
		void update(SOCKET client, ClientStatus status,
		            std::chrono::time_point<std::chrono::system_clock> lastActivity = std::chrono::system_clock::now())
		{
			std::lock_guard lk(m);
			if (status == ClientStatus::CLOSING)
			{
				clientStatus.erase(client);
				clientLastActivity.erase(client);
				CLOSE(client);
			}
			else
			{
				clientStatus[client] = status;
				clientLastActivity[client] = lastActivity;
			}
		}
		std::chrono::time_point<std::chrono::system_clock> get_last_activity(SOCKET client) const
		{
			std::lock_guard lk(m);
			return clientLastActivity.find(client)->second;
		}
		ClientStatus get_status(SOCKET client) const
		{
			std::lock_guard lk(m);
			return clientStatus.find(client)->second;
		}
	};

	class iserver
	{
	protected:
		SOCKET tcp_socket = INVALID_SOCKET;
		SOCKET udp_socket = INVALID_SOCKET;
		ServerStatus status = NOT_STARTED;
		protocol::abstract::ProtocolManager& protocolManager;
		ClientManager clientManager;

	public:
		static void allowNetworkOperations()
		{
#ifdef _WIN32
			WSADATA wsaData;
			int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
			if (iResult != NO_ERROR)
			{
				throw exceptions::io_exception("WSAStartup failed with error");
			}
#endif
		}
		static void endNetworkOperations()
		{
#ifdef _WIN32
			WSACleanup();
#endif
		}
		iserver(protocol::abstract::ProtocolManager& _protocolManager) : protocolManager(_protocolManager)
		{
			protocolManager.bind_to(clientManager);
		}
		virtual ~iserver()
		{
			if (tcp_socket != INVALID_SOCKET) CLOSE(tcp_socket);
			if (udp_socket != INVALID_SOCKET) CLOSE(udp_socket);
		}
		virtual void start() = 0;
		virtual SOCKET wait_for_client() { return clientManager.fetch_client(); }
		virtual void terminate() { status = TERMINATING; }
	};
}  // namespace boot

#endif  // ISERVER_H