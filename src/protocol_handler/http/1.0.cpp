#include <protocol_handler/http/1.0.h>

namespace protocol
{
	bool HTTP1::check(const std::string& request) { return request.find(" HTTP/1.0\r\n") != std::string::npos; }

	void HTTP1::send_data(SOCKET client, size_t flags, std::shared_ptr<abstract::Protocol::ProtocolParser> request_data)
	{
		if (flags != Flags::WHOLE_RESPONSE)
		{
			throw std::runtime_error("HTTP/1.0 sends only whole responses. Chunked responses are not allowed.");
		}
		// SEND(client, static_cast<const std::string*>(request_data->)->c_str(),
		//      static_cast<const std::string*>(response)->size(), 0);
		return;
	}

	HTTP1::HTTPProtocolParser::HTTPProtocolParser()
	    : criteria_matched_to(0), current_element(MessageElements::pathname), key(), value(), ProtocolParser()
	{
	}

	/**
	 * @brief Continues the processing of the request using the incoming chunk
	 *
	 * @param chunk the incoming chunk
	 */
	void HTTP1::HTTPProtocolParser::process_chunk(std::string_view chunk)
	{
		size_t i = 0;
		ParsingState current_state = get_state();
		bool save_parsed_info = false, skip_current_symbol = false, move_to_body = false;

		if (current_state == ParsingState::NOT_STARTED)
		{
			update_state(ParsingState::PROCESSING);
			current_state = ParsingState::PROCESSING;
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
									key += chunk[i];
									key.resize(key.size() - criteria_matched_to);
								}
								break;
							}
							case MessageElements::query_string_key:
							{
								if (key.size() != 0)
								{
									key += chunk[i];
									key.resize(key.size() - criteria_matched_to);
								}
							}
							case MessageElements::query_string_value:
							{
								if (value.size() != 0)
								{
									value += chunk[i];
									value.resize(value.size() - criteria_matched_to);
								}
								break;
							}
							default:
							{
								throw "Parsing exception1";
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
					break;
				}
				case MessageElements::query_string_key:
				{
					if (chunk[i] == '=')
					{
						save_parsed_info = true;
						skip_current_symbol = true;
					}
					break;
				}
				case MessageElements::query_string_value:
				{
					if (chunk[i] != '&' && value.size() > 0 && value.back() == '&')
					{
						value.pop_back();
						save_parsed_info = true;
					}
					break;
				}
				case MessageElements::header_key:
				{
					if (chunk[i] == ':')
					{
						save_parsed_info = true;
						skip_current_symbol = true;
					}
					else if (chunk[i] == '\n' && key == "\r")  // headers are followed by \r\n\r\n and then the body
					{
						skip_current_symbol = true;
						key.clear();
						move_to_body = true;
					}
					break;
				}
				case MessageElements::header_value:
				{
					if (chunk[i] == '\n' && value.size() > 0 && value.back() == '\r')
					{
						value.pop_back();
						if (value[0] == ' ') value = value.substr(1);
						save_parsed_info = true;
						skip_current_symbol = true;
					}
					break;
				}
				default:
				{
					// body end is not indicated or separated by any specific sequance
				}
			}

			// printf(
			//     "\"%s\" save_parsed_info=%d skip_current_symbol=%d, current_element=%d, criteria_matched_to=%d "
			//     "move_to_body=%d "
			//     "key=%s value=%s\n",
			//     transform(std::string(1, chunk[i])).c_str(), save_parsed_info, skip_current_symbol,
			//     current_element, criteria_matched_to, move_to_body, transform(key).c_str(),
			//     transform(value).c_str());

			if (save_parsed_info)
			{
				save_parsed_info = false;
				switch (current_element)
				{
					case MessageElements::pathname:
					{
						request->pathname = key;
						if (criteria_matched_to != criteria.size())
						{
							if (chunk[i] == '?') current_element = MessageElements::query_string_key;
						}
						else
						{
							current_element = MessageElements::header_key;
						}
						key.clear();
						break;
					}
					case MessageElements::query_string_key:
					{
						if (chunk[i] == '=') current_element = MessageElements::query_string_value;
						break;
					}
					case MessageElements::query_string_value:
					{
						request->query_params.emplace(key, value);
						if (criteria_matched_to == criteria.size())
						{
							current_element = MessageElements::header_key;
						}
						else
						{
							current_element = MessageElements::query_string_key;
						}
						value.clear();
						key.clear();
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
						value.clear();
						key.clear();
						break;
					}
					case MessageElements::body:
					{
						// the chunks are being appended below
						break;
					}
					default:
					{
						throw "Parsing exception2";
					}
				}
			}

			if (skip_current_symbol || chunk[i] == '\n')
			{
				skip_current_symbol = false;
				continue;
			}
			if (move_to_body)
			{
				current_element = MessageElements::body;
				move_to_body = false;
				if (chunk[i] == '\r') continue;
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
					break;
				default:
					throw "Parsing exception3";
			}
		}
	}

	std::shared_ptr<abstract::Protocol::ProtocolParser> HTTP1::get_praser()
	{
		return std::make_shared<HTTPProtocolParser>();
	}
}  // namespace protocol