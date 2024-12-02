#include <server/interface.h>

namespace protocol {
	Server::Server(const std::string& _port, logger::InfoLogger _info, logger::WarningLogger _warning,
	               logger::ErrorLogger _error, handlers::HandlerInterface* _handler)
	    : port(_port), info(_info), warning(_warning), error(_error), handler(_handler) {}

	void Server::get_client() {
		while (true) {
			// Accept a request
			SOCKET client = ACCEPT(listener, NULL, NULL);
			if (client == INVALID_SOCKET) {
				continue;
			}
			this->info << "(thread " << std::this_thread::get_id() << ") Client invalid: " << (client == INVALID_SOCKET)
			           << std::endl;
			this->info << "(thread " << std::this_thread::get_id() << ") Client found: " << client << std::endl;

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
				this->info << "(thread " << std::this_thread::get_id() << ") SELECT status is " << status << std::endl;
				if (status < 0) {
					this->error << "(thread " << std::this_thread::get_id() << ") INVALID SOCKET: " << client
					            << " was skipped (" << status << ")" << std::endl;
					continue;
				}

				this->info << "(thread " << std::this_thread::get_id() << ") Client " << client << " vs invalid socket "
				           << INVALID_SOCKET << std::endl;
				this->info << "(thread " << std::this_thread::get_id() << ") Requestor " << client << " is still valid"
				           << std::endl;

				this->handler->handle(client);
			}
		}
		end_server();
	}

	namespace exceptions {
		unable_to_retrieve_address::unable_to_retrieve_address() : failure("Address failed to be retrieved.") {}
		unable_to_bind_socket::unable_to_bind_socket() : failure("Socket was unable to bind.") {}
		socket_not_created::socket_not_created() : failure("Socket failed to create.") {}
	}  // namespace exceptions
}  // namespace protocol
