#ifndef PROTOCOL_HANDLER_H
#define PROTOCOL_HANDLER_H

#include <boot_server/protocol_manager.h>

namespace protocol {
	class ProtocolManager : public abstract::ProtocolManager {
	private:
	public:
		virtual void handle_client(SOCKET client, short type) final;
	};
}  // namespace protocol

#endif  // PROTOCOL_HANDLER_H