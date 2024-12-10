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
				write(client, response.data(), response.size());
			} else {  // http 1.0 or http 1.1
				std::string response =
				    "HTTP/1.1 200 OK\r\nDate: Mon, 27 Jul 2009 12:28:53 GMT\r\nServer: Apache/2.2.14 "
				    "(Win32)\r\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\nContent-Length: 88\r\nContent-Type: "
				    "text/html\r\nConnection: Closed\r\n\r\n<html>\r\n<body>\r\n<h1>Hello, "
				    "World!</h1>\r\n</body>\r\n</html>";
				write(client, response.data(), response.size());
#ifdef _WIN32
				closesocket(client);
#else
				close(client);
#endif
			}
		}
	}
}  // namespace protocol
