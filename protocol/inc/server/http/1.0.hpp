#include <server/interface.h>

namespace protocol {
	namespace http {
		class HTTP1_0 : public protocol::Server {
			HTTP1_0(const std::string& _port, logger::InfoLogger _info, logger::WarningLogger _warning,
			        logger::ErrorLogger _error, handlers::HandlerInterface* _handler);
			SOCKET listener;
			std::string port;
			virtual void start_server() final;
			virtual generator<SOCKET> get_client() final;
			virtual void end_server() final;
		};
	}  // namespace http

}  // namespace protocol
