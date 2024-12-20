#include <boot_server/server.h>
#include <iostream>
#include <iomanip>
#include <vector>

namespace boot {
	server::server(protocol::abstract::ProtocolManager& _protocolManager) : iserver(_protocolManager) {
#if defined(__linux__) || defined(__APPLE__)
		tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
		udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
#else
		tcp_socket = WSASocketW(AF_INET, SOCK_STREAM, 0, nullptr, 0, WSA_FLAG_OVERLAPPED);
		udp_socket = WSASocketW(AF_INET, SOCK_DGRAM, 0, nullptr, 0, WSA_FLAG_OVERLAPPED);
#endif
		if (tcp_socket == INVALID_SOCKET) {
			throw exceptions::io_exception("Unable to create TCP socket");
		}
		if (udp_socket == INVALID_SOCKET) {
			throw exceptions::io_exception("Unable to create UDP socket");
		}
	}

	void server::bind_to(size_t port) {
		sockaddr_in addr{};
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		addr.sin_port = htons(port);

		if (bind(tcp_socket, (sockaddr*)&addr, sizeof(addr)) < 0) {
			throw exceptions::io_exception("Unable to bind TCP socket to the given port");
		}

		if (bind(udp_socket, (sockaddr*)&addr, sizeof(addr)) < 0) {
			throw exceptions::io_exception("Unable to bind UDP socket to the given port");
		}
	}

	void server::start() {
#ifdef __linux__
		// Set non-blocking
		nonblock_config(tcp_socket);
		nonblock_config(udp_socket);

		listen(tcp_socket, SOMAXCONN);
		listen(udp_socket, SOMAXCONN);
		const int MAX_EVENTS = 100000;
		// Add sockets to epoll
		epoll_event event{}, events[MAX_EVENTS];
		event.events = EPOLLIN;

		event.data.fd = tcp_socket;
		
		int epoll_fd = epoll_create1(0);
		if (epoll_fd < 0) {
			std::cerr << "Failed to create epoll instance.\n";
			return;
		}

		epoll_ctl(epoll_fd, EPOLL_CTL_ADD, tcp_socket, &event);

		event.data.fd = udp_socket;
		epoll_ctl(epoll_fd, EPOLL_CTL_ADD, udp_socket, &event);

		while (true) {
			int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
			for (int i = 0; i < n; ++i) {
				if (events[i].events & EPOLLIN) {
					int client = accept(events[i].data.fd, nullptr, nullptr);
					if (client >= 0) {
						int type;
						socklen_t optlen = sizeof(type);
						if (getsockopt(client, SOL_SOCKET, SO_TYPE, &type, &optlen) == -1) {
							throw exceptions::runtime_exception("getsockopt failed");
						}
						nonblock_config(client);
						protocolManager.handle_client(client, events[i].data.fd == tcp_socket ? SOCK_STREAM : SOCK_DGRAM);
					}
				}
			}
		}

		close(tcp_socket);
		close(udp_socket);
		close(epoll_fd);
		tcp_socket = udp_socket = INVALID_SOCKET;
#else
#ifdef __APPLE__
		int kq = kqueue();
		if (kq == -1) {
			throw exceptions::io_exception("Failed to create kqueue.");
			return;
		}

		// Set non-blocking
		nonblock_config(tcp_socket);
		nonblock_config(udp_socket);

		listen(tcp_socket, SOMAXCONN);
		listen(udp_socket, SOMAXCONN);

		struct kevent changes[2];
		EV_SET(&changes[0], tcp_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, nullptr);
		EV_SET(&changes[1], udp_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, nullptr);

		if (kevent(kq, changes, 2, nullptr, 0, nullptr) == -1) {
			throw exceptions::io_exception("Failed to register events with kqueue.");
			close(tcp_socket);
			close(udp_socket);
			close(kq);
			tcp_socket = udp_socket = INVALID_SOCKET;
			return;
		}

		// Event loop
		while (true) {
			struct kevent events[10];
			int n = kevent(kq, nullptr, 0, events, 10, nullptr);

			if (n < 0) {
				throw exceptions::runtime_exception("kevent error.\n");
				break;
			}

			for (int i = 0; i < n; ++i) {
				// std::cout << events[i].ident << " " << std::hex << events[i].flags << std::endl;
				if (events[i].flags & EVFILT_READ) {
					SOCKET client = ACCEPT(events[i].ident, NULL, NULL);
					if (client == INVALID_SOCKET) {
						continue;
					}
					// Check if the socket is valid
					{
						struct timeval selTimeout;
						selTimeout.tv_sec = 1;
						selTimeout.tv_usec = 0;
						fd_set readSet;
						FD_ZERO(&readSet);
						FD_SET(client + 1, &readSet);
						FD_SET(client, &readSet);

						int status = SELECT(client + 1, &readSet, nullptr, nullptr, &selTimeout);
						if (status < 0) {
							continue;
						}

						int type;
						socklen_t optlen = sizeof(type);
						if (getsockopt(client, SOL_SOCKET, SO_TYPE, &type, &optlen) == -1) {
							throw exceptions::runtime_exception("getsockopt failed");
						}
						nonblock_config(client);
						protocolManager.handle_client(client, type);
					}
				}
			}
		}

		close(tcp_socket);
		close(udp_socket);
		close(kq);
		tcp_socket = udp_socket = INVALID_SOCKET;
#else
		// Set non-blocking
		nonblock_config(tcp_socket);
		nonblock_config(udp_socket);

		listen(tcp_socket, SOMAXCONN);
		listen(udp_socket, SOMAXCONN);

		std::cout << "Waiting for clients" << std::endl;

		std::vector<WSAPOLLFD> pollFds = {
		    {tcp_socket, POLLRDNORM, 0},  // TCP listener
		    {udp_socket, POLLRDNORM, 0}   // UDP listener
		};

		sockaddr_in clientAddr;
		int addrLen = sizeof(clientAddr);

		while (true) {
			// Poll the sockets
			int ret = WSAPoll(pollFds.data(), pollFds.size(), -1);  // Infinite timeout
			if (ret == SOCKET_ERROR) {
				std::cerr << "WSAPoll failed\n";
				break;
			}

			for (auto& pfd : pollFds) {
				if (pfd.revents & POLLRDNORM) {
					if (pfd.fd == tcp_socket) {
						// TCP: Accept new connections
						SOCKET clientSocket = accept(tcp_socket, (sockaddr*)&clientAddr, &addrLen);
						if (clientSocket != INVALID_SOCKET) {
							std::cout << "New TCP connection from " << inet_ntoa(clientAddr.sin_addr) << ":"
							          << ntohs(clientAddr.sin_port) << "\n";
							this->protocolManager.handle_client(clientSocket, SOCK_STREAM);
						}
					} else if (pfd.fd == udp_socket) {
						// UDP: Accept new connections
						SOCKET clientSocket = accept(udp_socket, (sockaddr*)&clientAddr, &addrLen);
						if (clientSocket != INVALID_SOCKET) {
							nonblock_config(clientSocket);
							std::cout << "UDP message from " << inet_ntoa(clientAddr.sin_addr) << ":"
							          << ntohs(clientAddr.sin_port) << "\n";
							this->protocolManager.handle_client(clientSocket, SOCK_DGRAM);
						}
					}
				}
			}
		}
#endif
#endif
	}

	server::~server() {}
}  // namespace boot
