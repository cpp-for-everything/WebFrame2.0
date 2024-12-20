#ifndef ISERVER_H
#define ISERVER_H

#include <boot_server/platform.h>
#include <utils/queue.h>
#include <boot_server/protocol_manager.h>

namespace boot {
	enum server_status { NOT_STARTED, RUNNING, TERMINATING };
	class iserver {
	protected:
		SOCKET tcp_socket = INVALID_SOCKET;
		SOCKET udp_socket = INVALID_SOCKET;
		server_status status = NOT_STARTED;
		thread_safe::TSQueue<SOCKET> clients;
		protocol::abstract::ProtocolManager& protocolManager;

	public:
		static void allowNetworkOperations() {
#ifdef _WIN32
			WSADATA wsaData;
			int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
			if (iResult != NO_ERROR) {
				throw exceptions::io_exception("WSAStartup failed with error");
			}
#endif
		}
		static void endNetworkOperations() {
#ifdef _WIN32
			WSACleanup();
#endif
		}
		iserver(protocol::abstract::ProtocolManager& _protocolManager) : protocolManager(_protocolManager) {}
		virtual ~iserver() {
#ifdef _WIN32
			if (tcp_socket != INVALID_SOCKET) closesocket(tcp_socket);
			if (udp_socket != INVALID_SOCKET) closesocket(udp_socket);
#else
			if (tcp_socket != INVALID_SOCKET) close(tcp_socket);
			if (udp_socket != INVALID_SOCKET) close(udp_socket);
#endif
		}
		virtual void start() = 0;
		virtual void wait_for_client() { clients.wait_for_element(); }
		virtual void terminate() { status = TERMINATING; }
	};
}  // namespace boot

#endif  // ISERVER_H