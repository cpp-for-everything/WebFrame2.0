#ifdef __linux__

#include <sys/epoll.h>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>

void setNonBlocking(int sock)
{
	int flags = fcntl(sock, F_GETFL, 0);
	fcntl(sock, F_SETFL, flags | O_NONBLOCK);
}

void runEpoll()
{
	const int MAX_EVENTS = 10;
	int epoll_fd = epoll_create1(0);
	if (epoll_fd < 0)
	{
		std::cerr << "Failed to create epoll instance.\n";
		return;
	}

	// Create two sockets
	int socket1 = socket(AF_INET, SOCK_STREAM, 0);
	int socket2 = socket(AF_INET, SOCK_STREAM, 0);

	if (socket1 < 0 || socket2 < 0)
	{
		std::cerr << "Socket creation failed.\n";
		return;
	}

	// Configure socket addresses
	sockaddr_in addr1{}, addr2{};
	addr1.sin_family = AF_INET;
	addr1.sin_addr.s_addr = INADDR_ANY;
	addr1.sin_port = htons(8080);

	addr2.sin_family = AF_INET;
	addr2.sin_addr.s_addr = INADDR_ANY;
	addr2.sin_port = htons(8081);

	// Bind and listen
	bind(socket1, (sockaddr*)&addr1, sizeof(addr1));
	bind(socket2, (sockaddr*)&addr2, sizeof(addr2));

	listen(socket1, 5);
	listen(socket2, 5);

	// Set non-blocking
	setNonBlocking(socket1);
	setNonBlocking(socket2);

	// Add sockets to epoll
	epoll_event event{}, events[MAX_EVENTS];
	event.events = EPOLLIN;

	event.data.fd = socket1;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket1, &event);

	event.data.fd = socket2;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket2, &event);

	std::cout << "Epoll server listening on ports 8080 and 8081...\n";

	while (true)
	{
		int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
		for (int i = 0; i < n; ++i)
		{
			if (events[i].events & EPOLLIN)
			{
				int client = accept(events[i].data.fd, nullptr, nullptr);
				if (client >= 0)
				{
					std::cout << "Connection on socket " << events[i].data.fd << ".\n";
					close(client);
				}
			}
		}
	}

	close(socket1);
	close(socket2);
	close(epoll_fd);
}

#endif
