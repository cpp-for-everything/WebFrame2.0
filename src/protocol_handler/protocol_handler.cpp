#include <protocol_handler/protocol_handler.h>
#include <string>
#include <iostream>
#include <chrono>

namespace protocol
{
	void ProtocolManager::handle_client(SOCKET client, short type)
	{
		auto last_activity = std::chrono::system_clock::now();
		if (type == SOCK_DGRAM)
		{  // UDP aka HTTP 3
		}
		else
		{  // TCP aka HTTP 0.9, 1.0, 1.1, or 2
			while (true)
			{
				std::string request, headline;
				constexpr std::size_t capacity = 1024;
				size_t ind;
				char data[capacity];
				int total_recv = 0;
				int n = 0;
				do
				{
					n = RECV(client, data, capacity, 0);
					std::cout << "{{{{{{{{\n" << data << "\n}}}}}}}}" << std::endl;
					if (n <= 0)
					{
						break;
					}

					total_recv += n;
					request += std::string(data, data + n);

					if ((ind = request.find('\r')) != std::string::npos)
					{
						headline = request.substr(0, ind);
						// break;
					}
				} while (n > 0);
				// std::cout << headline << "\n----------\n" << request << std::endl;
				if (headline == "PRI * HTTP/2.0")
				{  // http2
					std::string response =
					    "HEADERS\r\n:status: 200\r\ncontent-type: text/plain\r\ncontent-length: "
					    "13\r\nEND_HEADERS\r\nDATA\r\nHello, World!\r\nEND_STREAM";
					SEND(client, response.data(), response.size(), 0);
				}
				else if (headline.find("HTTP/1.1") != std::string::npos)
				{  // http 1.1
					std::cout << "----------------------------------\n" << request << std::endl;
					if (request.find("Connection: close\r\n") != std::string::npos)
					{
						{
							std::string response =
							    "HTTP/1.1 200 OK\r\nDate: Mon, 27 Jul 2009 12:28:53 GMT\r\nServer: Apache/2.2.14 "
							    "(Win32)\r\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\nContent-Length: "
							    "88\r\nContent-Type: text/plain\r\nAlt-Svc: h2c=\":8081\"; ma=3600\r\nConnection: "
							    "close\r\nTransfer-Encoding: chunked\r\n\r\n";
							{
								std::time_t end_time =
								    std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
								std::cout << "Trying to send headers for closing connection to client at "
								          << std::ctime(&end_time) << std::endl;
							}
							SEND(client, response.data(), response.size(), 0);
							{
								std::time_t end_time =
								    std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
								std::cout << "Sent headers for closing connection to client at "
								          << std::ctime(&end_time) << std::endl;
							}
						}
#ifdef _WIN32
						closesocket(client);
#else
						close(client);
#endif
						break;
					}
					{
						std::string response =
						    "HTTP/1.1 200 OK\r\nDate: Mon, 27 Jul 2009 12:28:53 GMT\r\nServer: Apache/2.2.14 "
						    "(Win32)\r\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\nContent-Length: "
						    "88\r\n";
						{
							std::time_t end_time =
							    std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
							std::cout << "Trying to send first chunk of headers to client at " << std::ctime(&end_time)
							          << std::endl;
						}
						SEND(client, response.data(), response.size(), 0);
						{
							std::time_t end_time =
							    std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
							std::cout << "Sent first chunk of headers to client at " << std::ctime(&end_time)
							          << std::endl;
						}
					}
					{
						std::string response =
						    "Content-Type: "
						    "text/plain\r\nAlt-Svc: h2c=\":8081\"; ma=3600\r\nConnection: "
						    "keep-alive\r\nTransfer-Encoding: chunked\r\n\r\n";
						{
							std::time_t end_time =
							    std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
							std::cout << "Trying to send second chunk of headers to client at " << std::ctime(&end_time)
							          << std::endl;
						}
						SEND(client, response.data(), response.size(), 0);
						{
							std::time_t end_time =
							    std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
							std::cout << "Sent second chunk of headers to client at " << std::ctime(&end_time)
							          << std::endl;
						}
					}
					{
						std::string response = "5\r\nHello\r\n";
						{
							std::time_t end_time =
							    std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
							std::cout << "Trying to send first chunk of body to client at " << std::ctime(&end_time)
							          << std::endl;
						}
						SEND(client, response.data(), response.size(), 0);
						{
							std::time_t end_time =
							    std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
							std::cout << "Sent first chunk of body to client at " << std::ctime(&end_time) << std::endl;
						}
					}
					{
						std::string response = "8\r\n, world!\r\n";
						{
							std::time_t end_time =
							    std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
							std::cout << "Trying to send second chunk of body to client at " << std::ctime(&end_time)
							          << std::endl;
						}
						SEND(client, response.data(), response.size(), 0);
						{
							std::time_t end_time =
							    std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
							std::cout << "Sent second chunk of body to client at " << std::ctime(&end_time)
							          << std::endl;
						}
					}
					{
						std::string response = "0\r\n\r\n";
						{
							std::time_t end_time =
							    std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
							std::cout << "Trying to send EoF chunk to client at " << std::ctime(&end_time) << std::endl;
						}
						SEND(client, response.data(), response.size(), 0);
						{
							std::time_t end_time =
							    std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
							std::cout << "Sent EoF chunk to client at " << std::ctime(&end_time) << std::endl;
						}
					}
					last_activity = std::chrono::system_clock::now();
				}
				else if (headline.find("HTTP/1.0") != std::string::npos)
				{  // http 1.0
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
					break;
				}
				else if (std::chrono::duration<double>(std::chrono::system_clock::now() - last_activity).count() >= 10)
				{
					{
						std::string response =
						    "HTTP/1.1 200 OK\r\nDate: Mon, 27 Jul 2009 12:28:53 GMT\r\nServer: Apache/2.2.14 "
						    "(Win32)\r\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\nContent-Length: "
						    "88\r\nContent-Type: text/plain\r\nAlt-Svc: h2c=\":8081\"; ma=3600\r\nConnection: "
						    "close\r\nTransfer-Encoding: chunked\r\n\r\n";
						{
							std::time_t end_time =
							    std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
							std::cout << "Trying to send headers for closing connection to client at "
							          << std::ctime(&end_time) << std::endl;
						}
						SEND(client, response.data(), response.size(), 0);
						{
							std::time_t end_time =
							    std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
							std::cout << "Sent headers for closing connection to client at " << std::ctime(&end_time)
							          << std::endl;
						}
					}
#ifdef _WIN32
					closesocket(client);
#else
					close(client);
#endif
					break;
				}
			}
		}
	}
}  // namespace protocol
