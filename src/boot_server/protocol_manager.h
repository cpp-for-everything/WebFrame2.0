#ifndef PROTOCOL_MANAGER_H
#define PROTOCOL_MANAGER_H

#include <boot_server/platform.h>
#include <string>

namespace boot
{
	class ClientManager;
}

namespace protocol
{
	namespace abstract
	{
		class Protocol
		{
		public:
			virtual bool check(const std::string& request) { return false; }
			virtual void sendData(SOCKET client, size_t flags, void* request) { return; }
		};

		class ProtocolManager
		{
		public:
			virtual void handle_tcp_client(SOCKET client) { CLOSE(client); }
			virtual void handle_udp_client(SOCKET server) { }
			virtual void bind_to(boot::ClientManager&) = 0;
		};
	}  // namespace abstract
}  // namespace protocol

#endif  // PROTOCOL_MANAGER_H