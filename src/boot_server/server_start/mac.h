#include <boot_server/server.h>

namespace boot
{
#ifdef __APPLE__
	void server::start()
	{
		nonblock_config(tcp_socket);
		nonblock_config(udp_socket);

		listen(tcp_socket, SOMAXCONN);

		int kq = kqueue();
		if (kq == -1)
		{
			std::cerr << "Failed to create kqueue." << std::endl;
			return;
		}

		using kevent_t = struct kevent;

		kevent_t event{};
		kevent_t events[100000];

		EV_SET(&event, tcp_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
		kevent(kq, &event, 1, NULL, 0, NULL);

		EV_SET(&event, udp_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
		kevent(kq, &event, 1, NULL, 0, NULL);

		sockaddr_in clientAddr;
		socklen_t addrLen = sizeof(clientAddr);

		while (true)
		{
			int n = kevent(kq, NULL, 0, events, 100000, NULL);
			for (int i = 0; i < n; ++i)
			{
				if (events[i].filter == EVFILT_READ)
				{
					if (events[i].ident == static_cast<uintptr_t>(tcp_socket))
					{  // TCP connection
						int client = ACCEPT(tcp_socket, (sockaddr*)&clientAddr, &addrLen);
						if (client >= 0)
						{
							nonblock_config(client);
							EV_SET(&event, client, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
							kevent(kq, &event, 1, NULL, 0, NULL);
							submit_client_to_handler(client, SOCK_STREAM);
						}
					}
					else if (events[i].ident == static_cast<uintptr_t>(udp_socket))
					{  // UDP message
						submit_client_to_handler(events[i].ident, SOCK_DGRAM);
					}
					else
					{  // TCP client event
						submit_client_to_handler(events[i].ident, SOCK_STREAM);
					}
				}
			}
		}

		CLOSE(kq);
		CLOSE(tcp_socket);
		CLOSE(udp_socket);
	}
#endif
}  // namespace boot