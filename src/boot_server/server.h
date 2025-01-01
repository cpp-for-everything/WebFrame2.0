#ifndef SERVER_H
#define SERVER_H

#include <stdexcept>
#include <ios>
#include <boot_server/iserver.h>
#include <unordered_map>
#include <chrono>

namespace boot
{
	namespace exceptions
	{
		class io_exception : public std::ios_base::failure
		{
		public:
			inline io_exception(const std::string& what_arg) : failure(what_arg) {}
		};

		class runtime_exception : public std::runtime_error
		{
		public:
			inline runtime_exception(const std::string& what_arg) : runtime_error(what_arg) {}
		};
	}  // namespace exceptions

	class server : public iserver
	{
	public:
		server(protocol::abstract::ProtocolManager& _protocolManager);
		~server();
		void bind_to(size_t port);
		virtual void start() override final;

	private:
		void submit_client_to_handler(SOCKET client, sockaddr_in clientAddr, short type);
	};
}  // namespace boot

#endif  // SERVER_H