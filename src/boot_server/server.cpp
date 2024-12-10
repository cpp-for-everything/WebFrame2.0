#include <boot_server/server.h>
#include <iostream>
#include <iomanip>

namespace boot {
	server::server() {
#if defined(__linux__) || defined(__APPLE__)
		tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
		udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
#else
		tcp_socket = WSASocket(AF_INET, SOCK_STREAM, 0, nullptr, 0, WSA_FLAG_OVERLAPPED);
		udp_socket = WSASocket(AF_INET, SOCK_DGRAM, 0, nullptr, 0, WSA_FLAG_OVERLAPPED);
#endif
		if (tcp_socket < 0) {
			throw exceptions::io_exception("Unable to create TCP socket");
		}
		if (udp_socket < 0) {
			throw exceptions::io_exception("Unable to create UDP socket");
		}
		protocolManager = nullptr;
	}

	void server::bind_to(size_t port) {
		sockaddr_in addr{};
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = INADDR_ANY;
		addr.sin_port = htons(port);

		if (bind(tcp_socket, (sockaddr*)&addr, sizeof(addr)) < 0) {
			throw exceptions::io_exception("Unable to bind TCP socket to the given port");
		}

		if (bind(udp_socket, (sockaddr*)&addr, sizeof(addr)) < 0) {
			throw exceptions::io_exception("Unable to bind UDP socket to the given port");
		}
	}

	void server::set_handler(protocol::interface::ProtocolManager* _protocolManager) {
		this->protocolManager = _protocolManager;
	}

	void server::start() {
		if (protocolManager == nullptr) {
			throw exceptions::runtime_exception("Protocol Manager is uninitialied. Server is getting shut down.");
		}
#ifdef __linux__
		// Set non-blocking
		setNonBlocking(tcp_socket);
		setNonBlocking(udp_socket);

		listen(tcp_socket, SOMAXCONN);
		listen(udp_socket, SOMAXCONN);
		// Add sockets to epoll
		epoll_event event{}, events[MAX_EVENTS];
		event.events = EPOLLIN;

		event.data.fd = tcp_socket;
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
						protocolManager->handle_client(client);
					}
				}
			}
		}

		close(tcp_socket);
		close(udp_socket);
		close(epoll_fd);
#elifdef __APPLE__
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
				std::cout << events[i].ident << " " << std::hex << events[i].flags << std::endl;
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
						protocolManager->handle_client(client, type);
					}
				}
			}
		}

		close(tcp_socket);
		close(udp_socket);
		close(kq);
#else
		// Create IOCP
		HANDLE iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
		if (!iocp) {
			throw exceptions::io_exception("Failed to create IOCP.");
			closesocket(tcp_socket);
			closesocket(udp_socket);
			WSACleanup();
			return;
		}

		// Associate sockets with IOCP
		CreateIoCompletionPort((HANDLE)tcp_socket, iocp, (ULONG_PTR)tcp_socket, 0);
		CreateIoCompletionPort((HANDLE)udp_socket, iocp, (ULONG_PTR)udp_socket, 0);

		// Worker thread
		while (true) {
			DWORD bytesTransferred;
			ULONG_PTR completionKey;
			OVERLAPPED* overlapped;

			if (GetQueuedCompletionStatus(iocp, &bytesTransferred, &completionKey, &overlapped, INFINITE)) {
				SOCKET client = (SOCKET)completionKey;
				if (client != INVALID_SOCKET) {
					int type;
					socklen_t optlen = sizeof(type);
					if (getsockopt(client, SOL_SOCKET, SO_TYPE, &type, &optlen) == -1) {
						throw exceptions::runtime_exception("getsockopt failed");
					}
					protocolManager->handle_client(client);
				}
			}
		}

		closesocket(tcp_socket);
		closesocket(udp_socket);
		CloseHandle(iocp);
		WSACleanup();
#endif
	}

	server::~server() {
#ifdef _WIN32
		closesocket(tcp_socket);
		closesocket(udp_socket);
#else
		close(tcp_socket);
		close(udp_socket);
#endif
	}
}  // namespace boot
