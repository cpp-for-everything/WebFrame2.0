#pragma once

#include <platform.h>

namespace protocol {
	namespace handlers {
		class HandlerInterface {
		public:
			virtual void handle(SOCKET client) { CLOSE(client); }
		};
	}  // namespace handlers
}  // namespace protocol
