#ifndef PROTOCOL_MANAGER_H
#define PROTOCOL_MANAGER_H

#include <boot_server/platform.h>

namespace protocol {
	namespace abstract {
		class ProtocolManager {
		public:
			virtual void handle_client(SOCKET client, short server_type) = 0;
		};
	}  // namespace abstract
}  // namespace protocol

#endif  // PROTOCOL_MANAGER_H