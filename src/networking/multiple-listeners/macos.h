#ifdef __APPLE__

#include <sys/event.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

void setNonBlocking(int sock)
{
	int flags = fcntl(sock, F_GETFL, 0);
	fcntl(sock, F_SETFL, flags | O_NONBLOCK);
}

void runKqueue()
{
	int kq = kqueue();
	if (kq == -1)
	{
		std::cerr << "Failed to create kqueue.\n";
		return;
	}

	// Create sockets
	int socket1 = socket(AF_INET, SOCK_STREAM, 0);
	int socket2 = socket(AF_INET, SOCK_STREAM, 0);

	if (socket1 < 0 || socket2 < 0)
	{
		std::cerr << "Socket creation failed.\n";
		return;
	}

	// Set non-blocking
	setNonBlocking(socket1);
	setNonBlocking(socket2);

	// Configure addresses
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

	// Add sockets to kqueue
	struct kevent changes[2];
	EV_SET(&changes[0], socket1, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, nullptr);
	EV_SET(&changes[1], socket2, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, nullptr);

	if (kevent(kq, changes, 2, nullptr, 0, nullptr) == -1)
	{
		std::cerr << "Failed to register events with kqueue.\n";
		close(socket1);
		close(socket2);
		close(kq);
		return;
	}

	std::cout << "Kqueue server listening on ports 8080 and 8081...\n";

	// Event loop
	while (true)
	{
		struct kevent events[10];
		int n = kevent(kq, nullptr, 0, events, 10, nullptr);

		if (n < 0)
		{
			std::cerr << "kevent error.\n";
			break;
		}

		for (int i = 0; i < n; ++i)
		{
			if (events[i].flags & EVFILT_READ)
			{
				int client = accept(events[i].ident, nullptr, nullptr);
				if (client >= 0)
				{
					std::cout << "Connection on socket " << events[i].ident << ".\n";
					close(client);
				}
			}
		}
	}

	close(socket1);
	close(socket2);
	close(kq);
}

#endif
