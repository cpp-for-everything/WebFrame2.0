#ifndef SERVER_H
#define SERVER_H

#include <stdexcept>
#include <ios>
#include <boot_server/iserver.h>
#include <unordered_map>
#include <chrono>

namespace boot
{
	class server : public iserver
	{
	public:
		server(protocol::abstract::ProtocolManager& _protocolManager);
		~server();
		void bind_to(size_t port);
		virtual void start() override final;

	private:
		void submit_client_to_handler(SOCKET client, short type);
	};
}  // namespace boot

#endif  // SERVER_H