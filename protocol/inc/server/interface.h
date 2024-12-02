#pragma once

#include <platform.h>
#include <generator.h>
#include <loggers.h>
#include <handlers/http/interface/client_handler.h>
#include <ios>

namespace protocol {
	class Server {
	protected:
		Server(const std::string& _port, logger::InfoLogger _info, logger::WarningLogger _warning,
		       logger::ErrorLogger _error, handlers::HandlerInterface* _handler);

	private:
		void get_client();

	protected:
		virtual void start_server() = 0;
		virtual void end_server() = 0;

		SOCKET listener;
		std::string port;
		logger::InfoLogger info;
		logger::WarningLogger warning;
		logger::ErrorLogger error;
		handlers::HandlerInterface* handler;
	};

	namespace exceptions {
		class unable_to_retrieve_address : public std::ios_base::failure {
		public:
			explicit unable_to_retrieve_address();
		};

		class unable_to_bind_socket : public std::ios_base::failure {
		public:
			explicit unable_to_bind_socket();
		};

		class socket_not_created : public std::ios_base::failure {
		public:
			explicit socket_not_created();
		};
	}  // namespace exceptions
}  // namespace protocol
