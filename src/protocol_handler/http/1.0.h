#include <boot_server/protocol_manager.h>
#include <protocol_handler/http/common/status_code.h>
#include <stdexcept>

namespace protocol
{
	class : public abstract::Protocol
	{
	public:
		enum Flags : size_t
		{
			WHOLE_RESPONSE = 1 << 0
		};

		virtual bool check(const std::string& request) final
		{
			return request.find(" HTTP/1.0\r\n") != std::string::npos;
		}

		virtual void sendData(SOCKET client, size_t flags, void* response) final
		{
			if (flags != Flags::WHOLE_RESPONSE)
			{
				throw std::runtime_error("HTTP/1.0 sends only whole responses. Chunked responses are not allowed.");
			}
			SEND(client, static_cast<const std::string*>(response)->c_str(),
			     static_cast<const std::string*>(response)->size(), 0);
			return;
		}
	} HTTP1;
}  // namespace protocol