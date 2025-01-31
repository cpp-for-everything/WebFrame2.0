#include <protocol_handler/protocol_handler.h>
#include <string>
#include <iostream>
#include <chrono>
#include <boot_server/platform.h>
#include <boot_server/server.h>
#include <protocol_handler/http/1.0.h>
#include <protocol_handler/http/1.1.h>
// #include <protocol_handler/http/2.0.h>

namespace protocol
{
	void ProtocolManager::bind_to(boot::ClientManager& _clientManager) { clientManager = &_clientManager; }

	void ProtocolManager::handle_tcp_client(SOCKET client)
	{
		clientManager->update(client, boot::ClientStatus::HANDLING);

		std::list<std::shared_ptr<abstract::Protocol::ProtocolParser>> parsers;

		for (auto& protocol : this->supported_protocols)
		{
			parsers.push_back(protocol.get_praser());
		}

		constexpr std::size_t capacity = 1024;
		size_t ind;
		char data[capacity];
		int total_recv = 0;
		int n = 0;
		do
		{
			n = RECV(client, data, capacity, 0);
			if (n <= 0)
			{
				break;
			}

			total_recv += n;
			for (auto it = parsers.begin(); it != parsers.end();)
			{
				auto& parser = *it;
				parser->process_chunk(std::string_view(data, n));
				if (parser->get_state() == abstract::Protocol::ProtocolParser::ParsingState::VERIFICATION_FAILED)
				{
					it = parsers.erase(it);
				}
				else
				{
					it++;
				}
			}
		} while (n > 0);

		std::string response =
		    "HTTP/1.0 400 Bad Request\r\nDate: Mon, 27 Jul 2009 12:28:53 GMT\r\nServer: Apache/2.2.14 "
		    "(Win32)\r\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\nContent-Length: "
		    "88\r\nContent-Type: "
		    "text/html\r\nAlt-Svc: h2c=\":8081\"; ma=3600\r\n\r\n\r\n";
		HTTP1().send_data(client, HTTP1::WHOLE_RESPONSE, nullptr);
		clientManager->update(client, boot::ClientStatus::CLOSING);
	}

	void ProtocolManager::handle_udp_client(SOCKET client) {}

}  // namespace protocol
