#include <server/http/1.0.hpp>
#include <thread>

namespace protocol {
	namespace http {
		HTTP1_0::HTTP1_0(const std::string& _port, logger::InfoLogger _info, logger::WarningLogger _warning,
		                 logger::ErrorLogger _error, handlers::HandlerInterface* _handler)
		    : Server(_port, _info, _warning, _error, _handler) {}
		void HTTP1_0::start_server() {
			int status;
			struct addrinfo hints, *res;
			SOCKET listener;

			// Before using hint you have to make sure that the data
			// structure is empty
			memset(&hints, 0, sizeof hints);
			// Set the attribute for hint
			hints.ai_family = AF_INET;        // We don't care V4 AF_INET or 6 AF_INET6
			hints.ai_socktype = SOCK_STREAM;  // TCP Socket SOCK_DGRAM
			hints.ai_flags = AI_PASSIVE;
			hints.ai_protocol = 0; /* Any protocol */
			hints.ai_canonname = NULL;
			hints.ai_addr = NULL;
			hints.ai_next = NULL;

			// Fill the res data structure and make sure that the
			// results make sense.
			status = getaddrinfo(NULL, port.c_str(), &hints, &res);
			if (status != 0) {
				this->error << "(main) getaddrinfo error: " << gai_strerror(status) << "\n";
				// application::port_status.change_status(port.c_str(),
				//                                        webframe::utils::server_status::port::Status::STARTED);
				// application::port_status.change_status(port.c_str(),
				//                                        webframe::utils::server_status::port::Status::STOPPED);
				throw exceptions::unable_to_retrieve_address();
			}

			// Create Socket and check if error occured afterwards
			listener = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
			if (listener == INVALID_SOCKET) {
				this->error << "(main) socket error: " << gai_strerror(status) << "\n";
				// application::port_status.change_status(port.c_str(),
				//                                        webframe::utils::server_status::port::Status::STARTED);
				// application::port_status.change_status(port.c_str(),
				//                                        webframe::utils::server_status::port::Status::STOPPED);
				throw exceptions::socket_not_created();
			}

			// Bind the socket to the address of my local machine and
			// port number
			status = bind(listener, res->ai_addr, sizeof(*res->ai_addr) /*res->ai_addrlen*/);
			if (status < 0) {
				this->error << "(main) bind error: " << status << " " << gai_strerror(status) << "\n";
				// application::port_status.change_status(port.c_str(),
				//                                        webframe::utils::server_status::port::Status::STARTED);
				// application::port_status.change_status(port.c_str(),
				//                                        webframe::utils::server_status::port::Status::STOPPED);
				throw exceptions::unable_to_bind_socket();
			}

			status = listen(listener, 128);
			if (status < 0) {
				this->error << "(main) listen error: " << gai_strerror(status) << "\n";
				// application::port_status.change_status(port.c_str(),
				//                                        webframe::utils::server_status::port::Status::STARTED);
				// application::port_status.change_status(port.c_str(),
				//                                        webframe::utils::server_status::port::Status::STOPPED);
				throw exceptions::unable_to_bind_socket();
			}

			// status = nonblock_config(listener);
			status = nonblock_config(listener);
			if (status < 0) {
				this->error << "(main) nonblocking config error: " << gai_strerror(status) << "\n";
				// application::port_status.change_status(port.c_str(),
				//                                        webframe::utils::server_status::port::Status::STARTED);
				// application::port_status.change_status(port.c_str(),
				//                                        webframe::utils::server_status::port::Status::STOPPED);
				throw exceptions::socket_not_created();
			}

			// Free the res linked list after we are done with it
			freeaddrinfo(res);
			// application::port_status.change_status(port.c_str(),
			// webframe::utils::server_status::port::Status::STARTED);
			this->listener = listener;
		}
		void HTTP1_0::end_server() {}
	}  // namespace http

}  // namespace protocol
