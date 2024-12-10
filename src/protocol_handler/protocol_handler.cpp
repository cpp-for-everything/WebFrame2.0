#include <protocol_handler/protocol_handler.h>
#include <string>
#include <iostream>

namespace protocol {
	void ProtocolManager::handle_client(SOCKET client, short type) {
		if (type == SOCK_DGRAM) {  // UDP aka HTTP 3

		} else {  // TCP aka HTTP 0.9, 1.0, 1.1, or 2
			std::string request, headline;
			constexpr std::size_t capacity = 1024;
			size_t ind;
			char data[capacity];
			int total_recv = 0;
			int n = 0;
			do {
				n = RECV(client, data, capacity, 0);
				if (n < 0) {
					break;
				}

				total_recv += n;
				request += std::string(data, data + n);

				if ((ind = request.find('\r')) != std::string::npos) {
					headline = request.substr(0, ind);
					// break;
				}
			} while (n > 0);
			std::cout << headline << "\n----------\n" << request << std::endl;
			if (headline == "PRI * HTTP/2.0") {  // http2
				std::string response =
				    "HEADERS\r\n:status: 200\r\ncontent-type: text/plain\r\ncontent-length: "
				    "13\r\nEND_HEADERS\r\nDATA\r\nHello, World!\r\nEND_STREAM";
				SEND(client, response.data(), response.size(), 0);
			} else if (headline.find("HTTP/1.1") != std::string::npos) {  // http 1.1
				{
					std::string response =
					    "HTTP/1.1 200 OK\r\nDate: Mon, 27 Jul 2009 12:28:53 GMT\r\nServer: Apache/2.2.14 "
					    "(Win32)\r\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\nContent-Length: "
					    "88\r\n";
					SEND(client, response.data(), response.size(), 0);
					response = "Content-Type: "
					    "text/html\r\nAlt-Svc: h2c=\":8081\"; ma=3600\r\nConnection: "
					    "keep-alive\r\nTransfer-Encoding: chunked\r\n\r\n";
					SEND(client, response.data(), response.size(), 0);
				}
				{
					std::string response = "d\r\nHello, world!\r\n";
					SEND(client, response.data(), response.size(), 0);
				}
				{
					std::string response = "0\r\n\r\n";
					SEND(client, response.data(), response.size(), 0);
				}
			} else if (headline.find("HTTP/1.0") != std::string::npos) {  // http 1.0
				std::string response =
				    "HTTP/1.1 200 OK\r\nDate: Mon, 27 Jul 2009 12:28:53 GMT\r\nServer: Apache/2.2.14 "
				    "(Win32)\r\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\nContent-Length: "
				    "88\r\nContent-Type: "
				    "text/html\r\nAlt-Svc: h2c=\":8081\"; ma=3600\r\nConnection: "
				    "keep-alive\r\nTransfer-Encoding: chunked\r\n\r\nHello, world!\r\n";
				SEND(client, response.data(), response.size(), 0);
#ifdef _WIN32
				closesocket(client);
#else
				close(client);
#endif
			}
		}
	}
}  // namespace protocol
