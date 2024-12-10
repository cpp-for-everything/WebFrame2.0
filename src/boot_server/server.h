#ifndef SERVER_H
#define SERVER_H

#include <stdexcept>
#include <ios>
#include <networking/platform.h>

#include <boot_server/protocol_manager.h>

namespace boot {
	namespace exceptions {
		class io_exception : public std::ios_base::failure {
		public:
			inline io_exception(const std::string& what_arg) : failure(what_arg) {}
		};

		class runtime_exception : public std::runtime_error {
		public:
			inline runtime_exception(const std::string& what_arg) : runtime_error(what_arg) {}
		};
	}  // namespace exceptions

	class server {
		SOCKET tcp_socket, udp_socket;
		protocol::interface::ProtocolManager* protocolManager;

	public:
		server();
		~server();
		void bind_to(size_t port);
		void set_handler(protocol::interface::ProtocolManager* _protocolManager);
		void start();
	};
}  // namespace boot

#endif  // SERVER_H