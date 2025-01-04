#include <boot_server/server.h>
#include <iostream>
#include <iomanip>
#include <vector>

namespace boot
{
	server::server(protocol::abstract::ProtocolManager& _protocolManager) : iserver(_protocolManager)
	{
#if defined(__linux__) || defined(__APPLE__)
		tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
		udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
#else
		tcp_socket = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
		udp_socket = WSASocketW(AF_INET, SOCK_DGRAM, IPPROTO_UDP, nullptr, 0, WSA_FLAG_OVERLAPPED);
#endif
		try
		{
			if (tcp_socket == INVALID_SOCKET)
			{
				throw exceptions::io_exception("Unable to create TCP socket");
			}
			if (udp_socket == INVALID_SOCKET)
			{
				throw exceptions::io_exception("Unable to create UDP socket");
			}
		}
		catch (std::exception& e)
		{
			if (tcp_socket != INVALID_SOCKET)
			{
				CLOSE(tcp_socket);
				tcp_socket = INVALID_SOCKET;
			}
			if (udp_socket != INVALID_SOCKET)
			{
				CLOSE(udp_socket);
				udp_socket = INVALID_SOCKET;
			}
			throw e;
		}
	}

	void server::bind_to(size_t port)
	{
		sockaddr_in addr{};
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		addr.sin_port = htons(port);

		if (bind(tcp_socket, (sockaddr*)&addr, sizeof(addr)) < 0)
		{
			throw exceptions::io_exception("Unable to bind TCP socket to the given port");
		}

		if (bind(udp_socket, (sockaddr*)&addr, sizeof(addr)) < 0)
		{
			throw exceptions::io_exception("Unable to bind UDP socket to the given port");
		}
	}

	void server::submit_client_to_handler(SOCKET client, short type)
	{
		if (client == INVALID_SOCKET)
		{
			return;
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
			if (status < 0)
			{
				return;
			}

			//opt_val_t type;
			//socklen_t optlen = sizeof(type);
			//if (getsockopt(client, SOL_SOCKET, SO_TYPE, &type, &optlen) == -1)
			//{
			//	throw exceptions::runtime_exception("getsockopt failed");
			//}
			nonblock_config(client);
		}
		if (type == SOCK_STREAM)  // TCP client
		{
			protocolManager.handle_tcp_client(client);
		}
		else if (type == SOCK_DGRAM)  // UDP message
		{
			protocolManager.handle_udp_client(client);
		}
	}

	server::~server() {}
}  // namespace boot

#include <boot_server/server_start/linux.h>
#include <boot_server/server_start/mac.h>
#include <boot_server/server_start/windows.h>
