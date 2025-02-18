#include <boot_server/server.h>

namespace boot
{
#ifdef __linux__
	void server::start()
	{
		// Set non-blocking
		nonblock_config(tcp_socket);
		nonblock_config(udp_socket);

		listen(tcp_socket, SOMAXCONN);
		listen(udp_socket, SOMAXCONN);
		const int MAX_EVENTS = 100000;
		// Add sockets to epoll
		epoll_event event{}, events[MAX_EVENTS];
		event.events = EPOLLIN;

		int epoll_fd = epoll_create1(0);
		if (epoll_fd < 0)
		{
			std::cerr << "Failed to create epoll instance.\n";
			return;
		}

		event.data.fd = tcp_socket;
		epoll_ctl(epoll_fd, EPOLL_CTL_ADD, event.data.fd, &event);

		event.data.fd = udp_socket;
		epoll_ctl(epoll_fd, EPOLL_CTL_ADD, event.data.fd, &event);

		sockaddr_in clientAddr;
		socklen_t addrLen = sizeof(clientAddr);

		while (true)
		{
			int n = epoll_wait(epoll_fd, events, sizeof(events) / sizeof(events[0]), -1);
			for (int i = 0; i < n; ++i)
			{
				if (events[i].events & EPOLLIN)
				{
					if (events[i].data.fd == tcp_socket)  // TCP conncetion established
					{
						SOCKET client = ACCEPT(events[i].data.fd, (sockaddr*)&clientAddr, &addrLen);
						event.data.fd = client;
						epoll_ctl(epoll_fd, EPOLL_CTL_ADD, event.data.fd, &event);
						submit_client_to_handler(client, SOCK_STREAM);
					}
					else if (events[i].data.fd == udp_socket)  // UDP message sent
					{
						submit_client_to_handler(events[i].data.fd, SOCK_DGRAM);
					}
					else  // TCP client event
					{
						// TODO: support non-UDP and non-TCP protocols
						// submit_client_to_handler(events[i].data.fd, SOCK_STREAM);
					}
				}
			}
		}

		CLOSE(epoll_fd);
		CLOSE(tcp_socket);
		CLOSE(udp_socket);
		tcp_socket = udp_socket = INVALID_SOCKET;
	}
#endif  // SERVER_START_LINUX_H
}  // namespace boot