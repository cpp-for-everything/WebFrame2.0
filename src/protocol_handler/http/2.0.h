#include <boot_server/protocol_manager.h>
#include <protocol_handler/http/common/status_code.h>
#include <string_view>

namespace protocol
{
	class : public abstract::Protocol
	{
	public:
		enum Flags : size_t
		{
			HEADERS = 1 << 0,
			BODY = HEADERS << 1
		};
		virtual bool check(const std::string& request) final
		{
			static constexpr std::string_view matcher = "PRI * HTTP/2.0\r\n";
			for (size_t i = 0; i < matcher.size(); i++)
				if (request.size() <= i || matcher[i] != request[i]) return false;
			return true;
		}
	} HTTP2;
}  // namespace protocol