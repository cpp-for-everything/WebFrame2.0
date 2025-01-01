#ifndef PROTOCOL_HANDLER_H
#define PROTOCOL_HANDLER_H

#include <boot_server/protocol_manager.h>

namespace protocol
{
	class ProtocolManager : public abstract::ProtocolManager
	{
	private:
		boot::ClientManager* clientManager;

	public:
		virtual void handle_tcp_client(SOCKET client, sockaddr_in clientAddr) final;
		virtual void handle_udp_client(SOCKET client, sockaddr_in clientAddr) final;
		virtual void bind_to(boot::ClientManager& _clientManager) final;
	};
}  // namespace protocol

#endif  // PROTOCOL_HANDLER_H