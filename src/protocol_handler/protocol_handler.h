#ifndef PROTOCOL_HANDLER_H
#define PROTOCOL_HANDLER_H

#include <boot_server/protocol_manager.h>
#include <protocol_handler/http/common/status_code.h>
#include <unordered_map>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <format>

namespace protocol
{
	class ProtocolManager : public abstract::ProtocolManager
	{
	private:
		boot::ClientManager* clientManager;
		std::vector<abstract::Protocol> supported_protocols;

	public:
		virtual void handle_tcp_client(SOCKET client) final override;
		virtual void handle_udp_client(SOCKET client) final override;
		virtual void bind_to(boot::ClientManager& _clientManager) final;
	};
}  // namespace protocol

#endif  // PROTOCOL_HANDLER_H