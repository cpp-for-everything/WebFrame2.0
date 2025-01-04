#include <boot_server/server.h>

namespace boot
{
#ifdef _WIN32
	int conditionFunction(IN LPWSABUF lpCallerId, IN LPWSABUF lpCallerData, IN OUT LPQOS lpSQOS,
	                               IN OUT LPQOS lpGQOS, IN LPWSABUF lpCalleeId, IN LPWSABUF lpCalleeData,
	                               OUT GROUP FAR* g, IN DWORD_PTR dwCallbackData)
	{
		// Custom logic to allow or deny the connection
		std::cout << "Condition function called. Connection allowed.\n";
		return CF_ACCEPT;  // Accept the connection (use CF_REJECT to reject)
	}
	void server::start()
	{
		// Set non-blocking
		nonblock_config(tcp_socket);
		nonblock_config(udp_socket);

		if (listen(tcp_socket, SOMAXCONN) == SOCKET_ERROR)
		{
			std::cerr << "Failed to listen on TCP socket." << std::endl;
			closesocket(tcp_socket);
			closesocket(udp_socket);
			return;
		}

		HANDLE tcp_event = WSACreateEvent();
		HANDLE udp_event = WSACreateEvent();

		if (tcp_event == WSA_INVALID_EVENT || udp_event == WSA_INVALID_EVENT)
		{
			std::cerr << "Failed to create events." << std::endl;
			closesocket(tcp_socket);
			closesocket(udp_socket);
			return;
		}

		if (WSAEventSelect(tcp_socket, tcp_event, FD_ACCEPT | FD_READ | FD_CLOSE) == SOCKET_ERROR ||
		    WSAEventSelect(udp_socket, udp_event, FD_READ) == SOCKET_ERROR)
		{
			std::cerr << "WSAEventSelect failed: " << WSAGetLastError() << std::endl;
			WSACloseEvent(tcp_event);
			WSACloseEvent(udp_event);
			closesocket(tcp_socket);
			closesocket(udp_socket);
			return;
		}

		std::unordered_map<HANDLE, SOCKET> eventSocketMap = {{tcp_event, tcp_socket}, {udp_event, udp_socket}};
		std::vector<HANDLE> events = {tcp_event, udp_event};

		while (true)
		{
			DWORD waitResult = WSAWaitForMultipleEvents(events.size(), events.data(), FALSE, WSA_INFINITE, FALSE);

			if (waitResult == WSA_WAIT_FAILED)
			{
				//std::cerr << "WSAWaitForMultipleEvents failed: " << WSAGetLastError() << std::endl;
				break;
			}

			int eventIndex = waitResult - WSA_WAIT_EVENT_0;
			HANDLE signaledEvent = events[eventIndex];
			SOCKET signaledSocket = eventSocketMap[signaledEvent];

			WSANETWORKEVENTS networkEvents;
			if (WSAEnumNetworkEvents(signaledSocket, signaledEvent, &networkEvents) == SOCKET_ERROR)
			{
				//std::cerr << "WSAEnumNetworkEvents failed: " << WSAGetLastError() << std::endl;
				continue;
			}

			if (networkEvents.lNetworkEvents & FD_ACCEPT)  // TCP handshake is happening
			{
				sockaddr_in clientAddr;
				int addrLen = sizeof(clientAddr);
				SOCKET client = WSAAccept(tcp_socket, (sockaddr*)&clientAddr, &addrLen, conditionFunction, 0);
				if (client != INVALID_SOCKET)
				{
					HANDLE tcp_client_events = WSACreateEvent();
					if (tcp_client_events == WSA_INVALID_EVENT)
					{
						std::cerr << "Invalid event " << WSAGetLastError() << std::endl;
						CLOSE(client);
						continue;
					}
					if (WSAEventSelect(client, tcp_client_events, FD_READ | FD_CLOSE) == SOCKET_ERROR)
					{
						WSACloseEvent(tcp_client_events);
						CLOSE(client);
						continue;
					}
					events.push_back(tcp_client_events);
					eventSocketMap.emplace(tcp_client_events, client);
					submit_client_to_handler(client, SOCK_STREAM);
				}
				else
				{
					std::cerr << "Failed to accept client: " << WSAGetLastError() << std::endl;
				}
			}
			else if (networkEvents.lNetworkEvents & FD_READ)
			{
				if (signaledSocket == udp_socket)  // UDP client send data
				{
					submit_client_to_handler(udp_socket, SOCK_DGRAM);
				}
				else  // TCP client send data
				{
					submit_client_to_handler(signaledSocket, SOCK_STREAM);
				}
			}

			if (networkEvents.lNetworkEvents & FD_CLOSE)
			{
				WSACloseEvent(signaledEvent);
				events.erase(events.begin() + eventIndex);  // O(n) for n = number of accepted TCP clients
				eventSocketMap.erase(signaledEvent);
				std::cerr << "Socket closed: " << signaledSocket << std::endl;
			}

			WSAResetEvent(signaledEvent);
		}

		WSACloseEvent(tcp_event);
		WSACloseEvent(udp_event);
		closesocket(tcp_socket);
		closesocket(udp_socket);
	}
#endif
}  // namespace boot