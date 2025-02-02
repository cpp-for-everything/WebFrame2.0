#include <boot_server/protocol_manager.h>
#include <protocol_handler/abstract/protocol.h>
#include <protocol_handler/http/common/status_code.h>
#include <stdexcept>

namespace protocol
{
	/**
	 * @brief HTTP 1.0 protocol manager
	 *
	 */
	class HTTP1 : public abstract::Protocol
	{
	public:
		/**
		 * @brief HTTP 1.0 protocol parser type
		 *
		 */
		class HTTPProtocolParser;

		enum Flags : size_t
		{
			WHOLE_RESPONSE = 1 << 0
		};

		virtual std::shared_ptr<ProtocolParser> get_praser() override final;

		virtual bool check(const std::string& request) final;

		virtual void send_data(SOCKET client, size_t flags,
		                       std::shared_ptr<ProtocolParser> request_data) override final;
	};

	class HTTP1::HTTPProtocolParser : public abstract::Protocol::ProtocolParser
	{
	private:
		static constexpr std::string_view criteria = " HTTP/1.0\r\n";
		size_t criteria_matched_to;
		std::string key, value;  // for pathname and body only key is being used

		enum MessageElements
		{
			pathname,
			query_string_key,
			query_string_value,
			header_key,
			header_value,
			body
		} current_element;

	public:
		/**
		 * @brief Construct a new HTTP 1.0 protocol parser object
		 *
		 */
		HTTPProtocolParser();

		/**
		 * @brief Continues the processing of the request using the incoming chunk
		 *
		 * @param chunk the incoming chunk
		 */
		virtual void process_chunk(std::string_view chunk);
	};
}  // namespace protocol