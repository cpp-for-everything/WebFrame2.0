#ifdef _WIN32

#include <winsock2.h>
#include <windows.h>
#include <mswsock.h>

void runIOCP()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		std::cerr << "WSAStartup failed\n";
		return;
	}

	// Create sockets
	SOCKET socket1 = WSASocket(AF_INET, SOCK_STREAM, 0, nullptr, 0, WSA_FLAG_OVERLAPPED);
	SOCKET socket2 = WSASocket(AF_INET, SOCK_STREAM, 0, nullptr, 0, WSA_FLAG_OVERLAPPED);

	if (socket1 == INVALID_SOCKET || socket2 == INVALID_SOCKET)
	{
		std::cerr << "Socket creation failed.\n";
		WSACleanup();
		return;
	}

	// Bind and listen
	sockaddr_in addr1{}, addr2{};
	addr1.sin_family = AF_INET;
	addr1.sin_addr.s_addr = INADDR_ANY;
	addr1.sin_port = htons(8080);

	addr2.sin_family = AF_INET;
	addr2.sin_addr.s_addr = INADDR_ANY;
	addr2.sin_port = htons(8081);

	bind(socket1, (sockaddr*)&addr1, sizeof(addr1));
	bind(socket2, (sockaddr*)&addr2, sizeof(addr2));

	listen(socket1, SOMAXCONN);
	listen(socket2, SOMAXCONN);

	// Create IOCP
	HANDLE iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
	if (!iocp)
	{
		std::cerr << "Failed to create IOCP.\n";
		closesocket(socket1);
		closesocket(socket2);
		WSACleanup();
		return;
	}

	// Associate sockets with IOCP
	CreateIoCompletionPort((HANDLE)socket1, iocp, (ULONG_PTR)socket1, 0);
	CreateIoCompletionPort((HANDLE)socket2, iocp, (ULONG_PTR)socket2, 0);

	std::cout << "IOCP server listening on ports 8080 and 8081...\n";

	// Worker thread
	while (true)
	{
		DWORD bytesTransferred;
		ULONG_PTR completionKey;
		OVERLAPPED* overlapped;

		if (GetQueuedCompletionStatus(iocp, &bytesTransferred, &completionKey, &overlapped, INFINITE))
		{
			SOCKET client = (SOCKET)completionKey;
			if (client != INVALID_SOCKET)
			{
				std::cout << "Connection on socket " << client << ".\n";
				closesocket(client);
			}
		}
	}

	closesocket(socket1);
	closesocket(socket2);
	CloseHandle(iocp);
	WSACleanup();
}

#endif
