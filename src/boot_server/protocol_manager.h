#ifndef PROTOCOL_MANAGER_H
#define PROTOCOL_MANAGER_H

#include <networking/platform.h>

namespace protocol {
	namespace interface {
		class ProtocolManager {
		public:
			virtual void handle_client(SOCKET client, short server_type) = 0;
		};
	}  // namespace interface
}  // namespace protocol

#endif  // PROTOCOL_MANAGER_H