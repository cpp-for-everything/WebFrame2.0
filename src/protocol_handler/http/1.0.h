#include <boot_server/protocol_manager.h>
#include <protocol_handler/abstract/protocol.h>
#include <protocol_handler/http/common/status_code.h>
#include <stdexcept>

namespace protocol
{
	class HTTP1 : public abstract::Protocol
	{
	public:
		class HTTPProtocolParser;

		enum Flags : size_t
		{
			WHOLE_RESPONSE = 1 << 0
		};

		virtual std::shared_ptr<ProtocolParser> get_praser() override final;

		virtual bool check(const std::string& request) final
		{
			return request.find(" HTTP/1.0\r\n") != std::string::npos;
		}

		virtual void send_data(SOCKET client, size_t flags, std::shared_ptr<ProtocolParser> request_data) override final
		{
			if (flags != Flags::WHOLE_RESPONSE)
			{
				throw std::runtime_error("HTTP/1.0 sends only whole responses. Chunked responses are not allowed.");
			}
			// SEND(client, static_cast<const std::string*>(request_data->)->c_str(),
			//      static_cast<const std::string*>(response)->size(), 0);
			return;
		}
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
		HTTPProtocolParser()
		    : criteria_matched_to(0), current_element(MessageElements::pathname), key(), value(), ProtocolParser()
		{
		}

		/**
		 * @brief Continues the processing of the request using the incoming chunk
		 *
		 * @param chunk the incoming chunk
		 */
		virtual void process_chunk(std::string_view chunk)
		{
			size_t i = 0;
			ParsingState current_state = get_state();
			bool save_parsed_info = false, skip_current_symbol = false;

			if (current_state == ParsingState::NOT_STARTED)
			{
				update_state(ParsingState::PROCESSING);
			}

			for (; i < chunk.size(); i++)
			{
				if (current_state == ParsingState::PROCESSING)  // Not verified protocol yet
				{
					if (chunk[i] != criteria[criteria_matched_to])
					{
						criteria_matched_to = 0;
					}
					else
					{
						criteria_matched_to++;
						if (criteria_matched_to == criteria.size())
						{
							update_state(ParsingState::VERIFICATION_SUCCEEDED);
							current_state = ParsingState::VERIFICATION_SUCCEEDED;
							switch (current_element)
							{
								case MessageElements::pathname:
								{
									if (key.size() != 0)
									{
										key.resize(key.size() - criteria_matched_to);
									}
									break;
								}
								case MessageElements::query_string_key:
								{
									if (key.size() != 0)
									{
										key.resize(key.size() - criteria_matched_to);
									}
								}
								case MessageElements::query_string_value:
								{
									if (value.size() != 0)
									{
										value.resize(value.size() - criteria_matched_to);
									}
									break;
								}
								default:
								{
									throw "Parsing exception";
								}
							}
							save_parsed_info = true;
						}
					}
				}

				switch (current_element)
				{
					case MessageElements::pathname:
					{
						if (chunk[i] == '?')
						{
							save_parsed_info = true;
							skip_current_symbol = true;
						}
					}
					case MessageElements::query_string_key:
					{
						if (chunk[i] == '=')
						{
							save_parsed_info = true;
							skip_current_symbol = true;
						}
					}
					case MessageElements::query_string_value:
					{
						if (chunk[i] != '&' && value.back() == '&')
						{
							value.pop_back();
							save_parsed_info = true;
						}
					}
					case MessageElements::header_key:
					{
						if (chunk[i] == ':')
						{
							save_parsed_info = true;
							skip_current_symbol = true;
						}
						if (chunk[i] == '\n' &&
						    value == "\r\n\r")  // headers are followed by \r\n\r\n and then the body
						{
							skip_current_symbol = true;
							key.resize(0);
						}
					}
					case MessageElements::header_value:
					{
						if (chunk[i] == '\n' && value.back() == '\r')
						{
							value.pop_back();
							save_parsed_info = true;
							skip_current_symbol = true;
							current_element = MessageElements::body;
						}
					}
					default:
					{
						// body end is not indicated or separated by any specific sequance
					}
				}

				if (save_parsed_info)
				{
					save_parsed_info = false;
					switch (current_element)
					{
						case MessageElements::pathname:
						{
							request->pathname = key;
							current_element = MessageElements::query_string_key;
							key.resize(0);
							break;
						}
						case MessageElements::query_string_key:
						{
							current_element = MessageElements::query_string_value;
							break;
						}
						case MessageElements::query_string_value:
						{
							request->query_params.emplace(key, value);
							current_element = MessageElements::query_string_key;
							value.resize(0);
							key.resize(0);
							break;
						}
						case MessageElements::header_key:
						{
							current_element = MessageElements::header_value;
							break;
						}
						case MessageElements::header_value:
						{
							request->headers.emplace(key, value);
							current_element = MessageElements::header_key;
							value.resize(0);
							key.resize(0);
							break;
						}
						case MessageElements::body:
						{
							// the chunks are being appended below
						}
						default:
						{
							throw "Parsing exception";
						}
					}
				}

				if (skip_current_symbol)
				{
					skip_current_symbol = false;
					continue;
				}

				switch (current_element)
				{
					case MessageElements::pathname:
					case MessageElements::query_string_key:
					case MessageElements::header_key:
						key += chunk[i];
						break;
					case MessageElements::query_string_value:
					case MessageElements::header_value:
						value += chunk[i];
						break;
					case MessageElements::body:
						request->body.write(chunk.substr(i));  // write the rest of the chunk as body
						i = chunk.size() - 1;                  // skip the processing of rest of characters
					default:
						throw "Parsing exception";
				}
			}
		}
	};

	std::shared_ptr<abstract::Protocol::ProtocolParser> HTTP1::get_praser()
	{
		return std::make_shared<HTTPProtocolParser>();
	}
}  // namespace protocol