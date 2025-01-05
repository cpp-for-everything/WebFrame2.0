#include <protocol_handler/protocol_handler.h>
#include <string>
#include <iostream>
#include <chrono>
#include <boot_server/platform.h>
#include <boot_server/server.h>
#include <protocol_handler/http/1.0.h>
#include <protocol_handler/http/1.1.h>
#include <protocol_handler/http/2.0.h>

namespace protocol
{
	void ProtocolManager::bind_to(boot::ClientManager& _clientManager) { clientManager = &_clientManager; }

	void ProtocolManager::handle_tcp_client(SOCKET client)
	{
		clientManager->update(client, boot::ClientStatus::HANDLING);

		std::string request, headline;
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
			request += std::string(data, data + n);
		} while (n > 0);
		std::cout << ">\n" << request << "\n<" << std::endl;
		if (HTTP2.check(request))
		{
			std::string response =
			    "HEADERS\r\n:status: 200\r\ncontent-type: text/plain\r\ncontent-length: "
			    "13\r\nEND_HEADERS\r\nDATA\r\nHello, World!\r\nEND_STREAM";
			SEND(client, response.data(), response.size(), 0);
			clientManager->update(client, boot::ClientStatus::WAITING);
		}
		else if (HTTP11.check(request))  // http 1.1
		{
			std::cout << "----------------------------------\n" << request << std::endl;
			if (request.find("Connection: close\r\n") != std::string::npos)
			{
				std::unordered_map<std::string, std::string> headers = {
				    {"Date", "Mon, 27 Jul 2009 12:28:53 GMT"},
				    {"Server", "Apache/2.2.14 (Win32)"},
				    {"Last-Modified", "Wed, 22 Jul 2009 19:15:56 GMT"},
				    {"Content-Length", "0"},
				    {"Content-Type", "text/plain"},
				    {"Alt-Svc", "h2c=\":8081\"; ma=3600"},
				    {"Connection", "close"},
				    {"Transfer-Encoding", "chunked"},
				};
				HTTP11.sendData(client, HTTP11.STATUS_CODE, (void*)200);
				HTTP11.sendData(client, HTTP11.HEADERS, &headers);
				HTTP11.sendData(client, HTTP11.BODY, nullptr);
				clientManager->update(client, boot::ClientStatus::CLOSING);
				return;
			}
			else
			{
				std::string response =
				    "<link rel=\"icon\" type=\"image/x-icon\" href=\"/images/favicon.ico\">\nHello, World!";
				std::unordered_map<std::string, std::string> headers = {
				    {"Date", "Mon, 27 Jul 2009 12:28:53 GMT"},
				    {"Server", "Apache/2.2.14 (Win32)"},
				    {"Last-Modified", "Wed, 22 Jul 2009 19:15:56 GMT"},
				    {"Content-Length", std::to_string(response.size())},
				    {"Content-Type", "text/html"},
				    {"Alt-Svc", "h2c=\":8081\"; ma=3600"},
				    {"Connection", "keep-alive"},
				    {"Transfer-Encoding", "chunked"},
				};
				HTTP11.sendData(client, HTTP11.STATUS_CODE, (void*)200);
				HTTP11.sendData(client, HTTP11.HEADERS, &headers);
				HTTP11.sendData(client, HTTP11.BODY, &response);
				clientManager->update(client, boot::ClientStatus::WAITING);
			}
		}
		else if (HTTP1.check(request))  // http 1.0
		{
			std::string response =
			    "HTTP/1.0 200 OK\r\nDate: Mon, 27 Jul 2009 12:28:53 GMT\r\nServer: Apache/2.2.14 "
			    "(Win32)\r\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\nContent-Length: "
			    "88\r\nContent-Type: "
			    "text/html\r\nAlt-Svc: h2c=\":8081\"; ma=3600\r\n\r\nHello, world!\r\n";
			HTTP1.sendData(client, HTTP1.WHOLE_RESPONSE, &response);
			clientManager->update(client, boot::ClientStatus::CLOSING);
		}
		else
		{
			std::string response =
			    "HTTP/1.0 400 Bad Request\r\nDate: Mon, 27 Jul 2009 12:28:53 GMT\r\nServer: Apache/2.2.14 "
			    "(Win32)\r\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\nContent-Length: "
			    "88\r\nContent-Type: "
			    "text/html\r\nAlt-Svc: h2c=\":8081\"; ma=3600\r\n\r\n\r\n";
			HTTP1.sendData(client, HTTP1.WHOLE_RESPONSE, &response);
			clientManager->update(client, boot::ClientStatus::CLOSING);
		}
	}

	void ProtocolManager::handle_udp_client(SOCKET client) {}

}  // namespace protocol
