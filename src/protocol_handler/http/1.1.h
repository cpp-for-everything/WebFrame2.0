#include <boot_server/protocol_manager.h>
#include <protocol_handler/http/common/status_code.h>
#include <stdexcept>
#include <sstream>
#include <format>

namespace protocol
{
	class : public abstract::Protocol
	{
	public:
		enum Flags : size_t
		{
			STATUS_CODE = 1 << 0,
			HEADERS = STATUS_CODE << 1,
			BODY = HEADERS << 1,
		};
		virtual bool check(const std::string& request) final
		{
			return request.find(" HTTP/1.1\r\n") != std::string::npos;
		}
		virtual void sendData(SOCKET client, size_t flags, void* response) final
		{
			if (flags == Flags::STATUS_CODE)
			{
				const size_t status_code = reinterpret_cast<size_t>(response);
				std::string status_line =
				    "HTTP/1.1 " + std::to_string(status_code) + " " + http::status_message[status_code] + "\r\n";
				SEND(client, status_line.c_str(), status_line.size(), 0);
			}
			else if (flags == Flags::HEADERS)
			{
				if (response != nullptr)
				{
					const std::unordered_map<std::string, std::string>* headers =
					    static_cast<const std::unordered_map<std::string, std::string>*>(response);
					std::stringstream ss;
					for (const auto& header : *headers)
					{
						ss << header.first << ":" << header.second << "\r\n";
						SEND(client, ss.str().c_str(), ss.str().size(), 0);
						ss.str("");
					}
					SEND(client, "\r\n", 2, 0);
				}
				else
				{
					SEND(client, "\r\n\r\n", 2, 0);
				}
			}
			else if (flags == Flags::BODY)
			{
				if (response != nullptr)
				{
					const std::string* body = static_cast<const std::string*>(response);
					for (size_t i = 0; i < body->size(); i += 1024)
					{
						if (i + 1024 <= body->size())
						{
							SEND(client, "400\r\n", 5, 0);
							SEND(client, body->c_str() + i, 1024, 0);
							SEND(client, "\r\n", 2, 0);
						}
						else
						{
							std::string chunk_size = std::format("{:x}", (body->size() - i)) + "\r\n";
							SEND(client, chunk_size.c_str(), chunk_size.size(), 0);
							SEND(client, body->c_str() + i, body->size() - i, 0);
							SEND(client, "\r\n", 2, 0);
						}
					}
				}
				SEND(client, "0\r\n\r\n", 5, 0);
			}
			else
			{
				throw std::runtime_error("Invalid flags for the type of data sent via HTTP/1.1.");
			}
			return;
		}
	} HTTP11;
}  // namespace protocol