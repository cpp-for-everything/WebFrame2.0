#include <boot_server/protocol_manager.h>
#include <protocol_handler/http/common/status_code.h>
#include <protocol_handler/abstract/protocol.h>
#include <nghttp2/nghttp2.h>  // Include the nghttp2 library

namespace protocol
{
	/**
	 * @brief Class for handling HTTP2 requests
	 *
	 */
	class HTTP2 : public abstract::Protocol
	{
	public:
		/**
		 * @brief HTTPProtocolParser class for HTTP2
		 *
		 */
		class HTTPProtocolParser;

	public:
		/**
		 * @brief Construct a new HTTP2 object
		 *
		 */
		HTTP2() : abstract::Protocol() {}

		/**
		 * @brief Destroy the HTTP2 object
		 *
		 */
		~HTTP2() = default;

		/**
		 * @brief Implementation of the pure virtual function from the base class
		 *
		 */
		std::shared_ptr<abstract::Protocol::ProtocolParser> get_praser() override;

		/**
		 * @brief Implementation of the pure virtual function from the base class
		 *
		 */
		void send_data(SOCKET client, size_t flags,
		               std::shared_ptr<abstract::Protocol::ProtocolParser> request_data) override
		{
			// Send the HTTP2 response
			// Use the request_data to construct the response
		}
	};  // class HTTP2
	class HTTP2::HTTPProtocolParser : public abstract::Protocol::ProtocolParser
	{
	private:
		// Define states for the state machine
		enum class MessageElements
		{
			// State for waiting the preable
			WAITING_FOR_PREABLE,
			// State for reading the frame header
			READING_FRAME_HEADER,
			// State for reading the frame payload
			READING_FRAME_PAYLOAD,
			// State for parsing frame header
			PARSING_FRAME_HEADER,
			// State for parsing frame payload
			PARSING_FRAME_PAYLOAD,
			// State for parsing the request
			PARSING_REQUEST,
			// State for parsing done
			PARSING_DONE,
			// State for verification succeeded
			VERIFICATION_SUCCEEDED,
			// State for verification failed
			VERIFICATION_FAILED
		};

		// Current state of the HTTPProtocolParser
		MessageElements current_state;

		// Buffer to store the preamble bytes
		std::string preamble_buffer;

		// Buffer to store the current frame header
		std::vector<unsigned char> frame_header_buffer;

		// Buffer to store the current frame payload
		std::vector<unsigned char> frame_payload_buffer;

		// Expected length of the current frame payload
		size_t expected_payload_length;

		// Type of the current frame
		uint8_t frame_type;

		// Flags of the current frame
		uint8_t frame_flags;

		// Stream identifier of the current frame
		uint32_t stream_identifier;

		// HPACK decompressor object
		nghttp2_hd_inflater* hpack_decompressor;

		// Helper function to extract header value from HPACK-decoded headers
		std::string extract_header_value(const std::vector<unsigned char>& frame_payload,
		                                 const std::string& header_name)
		{
			// 1. Decode the HPACK-compressed header block
			std::vector<nghttp2_nv> headers;
			size_t total_headers_size = 0;

			ssize_t rv = nghttp2_hd_inflate_hd3(hpack_decompressor, nullptr, &total_headers_size, frame_payload.data(),
			                                    frame_payload.size(), 1);
			if (rv < 0)
			{
				// Handle error
			}

			headers.resize(total_headers_size);
			rv = nghttp2_hd_inflate_hd3(hpack_decompressor, headers.data(), &total_headers_size, frame_payload.data(),
			                            frame_payload.size(), 1);
			if (rv < 0)
			{
				// Handle error
			}

			// 2. Find the header with the given name
			for (const auto& header : headers)
			{
				std::string name(reinterpret_cast<const char*>(header.name), header.namelen);
				if (name == header_name)
				{
					return std::string(reinterpret_cast<const char*>(header.value), header.valuelen);
				}
			}

			// 3. If the header is not found, return an empty string
			return "";
		}

		static void parse_query_params(const std::string& query_string,
		                               std::unordered_map<std::string, std::any>& query_params)
		{
			std::string key, value;
			std::string& current = key;

			for (auto& x : query_string)
			{
				switch (x)
				{
					case '&':
					{
						query_params[key] = value;
						current = key;
						key.clear();
						value.clear();
						break;
					}
					case '=':
					{
						current = value;
						break;
					}
					default:
					{
						current += x;
						break;
					}
				}
			}
			query_params[key] = value;
		}

	public:
		/**
		 * @brief Construct a new HTTPProtocolParser object
		 *
		 */
		HTTPProtocolParser() : abstract::Protocol::ProtocolParser(), current_state(MessageElements::WAITING_FOR_PREABLE)
		{
			// Initialize the HPACK decompressor
			int rv = nghttp2_hd_inflate_new(&hpack_decompressor);
			if (rv != 0)
			{
				// Handle error
			}
		}

		/**
		 * @brief Destroy the HTTPProtocolParser object
		 *
		 */
		~HTTPProtocolParser()
		{
			// Free the HPACK decompressor
			nghttp2_hd_inflate_del(hpack_decompressor);
		}

		/**
		 * @brief Implementation of the pure virtual function from the base class
		 *
		 */
		void process_chunk(std::string_view chunk) override
		{
			// State machine to handle different parsing stages
			while (!chunk.empty())
			{
				switch (current_state)
				{
					case MessageElements::WAITING_FOR_PREABLE:
					{
						// Read the connection preface
						constexpr std::string_view preface = "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n";
						size_t preface_pos = chunk.find(preface);
						if (preface_pos == std::string_view::npos)
						{
							// If the preface is not found in the current chunk
							// Check if the beginning of the chunk matches the end of the preface
							size_t partial_match_length = 0;
							while (partial_match_length < chunk.size() &&
							       preamble_buffer.size() + partial_match_length < preface.size() &&
							       chunk[partial_match_length] ==
							           preface[preamble_buffer.size() + partial_match_length])
							{
								partial_match_length++;
							}

							if (partial_match_length == chunk.size())
							{
								// If the entire chunk is a partial match, append it to the buffer
								preamble_buffer += chunk;
								chunk.remove_prefix(chunk.size());
							}
							else if (partial_match_length > 0)
							{
								// If there's a partial match, append the matching part to the buffer
								preamble_buffer += chunk.substr(0, partial_match_length);
								chunk.remove_prefix(partial_match_length);

								// Check if the buffer now contains the complete preface
								if (preamble_buffer == preface)
								{
									preamble_buffer.clear();
									current_state = MessageElements::READING_FRAME_HEADER;
								}
							}
							else
							{
								// If there's no match, reset the buffer and mark verification failed
								preamble_buffer.clear();
								update_state(MessageElements::VERIFICATION_FAILED);
								return;
							}
						}
						else
						{
							// If the preface is found, remove it and proceed
							chunk.remove_prefix(preface_pos + preface.size());
							preamble_buffer.clear();
							current_state = MessageElements::READING_FRAME_HEADER;
						}
						break;
					}
					case MessageElements::READING_FRAME_HEADER:
					{
						// Read the 9-byte frame header
						size_t bytes_to_read =
						    std::min(static_cast<size_t>(9) - frame_header_buffer.size(), chunk.size());
						std::copy(chunk.begin(), chunk.begin() + bytes_to_read,
						          std::back_inserter(frame_header_buffer));
						chunk.remove_prefix(bytes_to_read);
						if (frame_header_buffer.size() == 9)
						{
							current_state = MessageElements::PARSING_FRAME_HEADER;
						}
						break;
					}
					case MessageElements::PARSING_FRAME_HEADER:
					{
						// Parse the frame header
						expected_payload_length = static_cast<size_t>(frame_header_buffer) << 16 |
						                          static_cast<size_t>(frame_header_buffer) << 8 |
						                          static_cast<size_t>(frame_header_buffer);
						frame_type = frame_header_buffer;
						frame_flags = frame_header_buffer;
						stream_identifier = static_cast<uint32_t>(frame_header_buffer) << 24 |
						                    static_cast<uint32_t>(frame_header_buffer) << 16 |
						                    static_cast<uint32_t>(frame_header_buffer) << 8 |
						                    static_cast<uint32_t>(frame_header_buffer);
						frame_header_buffer.clear();
						current_state = MessageElements::READING_FRAME_PAYLOAD;
						break;
					}
					case MessageElements::READING_FRAME_PAYLOAD:
					{
						if (frame_type == 0x00)  // DATA frame
						{
							// Directly write DATA frame payload to request->body
							size_t bytes_to_write = std::min(expected_payload_length, chunk.size());
							request->body.write(chunk.substr(0, bytes_to_write));
							chunk.remove_prefix(bytes_to_write);
							expected_payload_length -= bytes_to_write;

							if (expected_payload_length == 0)
							{
								current_state = MessageElements::READING_FRAME_HEADER;
							}
						}
						else
						{
							// For other frames, read the entire payload first
							size_t bytes_to_read =
							    std::min(expected_payload_length - frame_payload_buffer.size(), chunk.size());
							std::copy(chunk.begin(), chunk.begin() + bytes_to_read,
							          std::back_inserter(frame_payload_buffer));
							chunk.remove_prefix(bytes_to_read);
							if (frame_payload_buffer.size() == expected_payload_length)
							{
								current_state = MessageElements::PARSING_FRAME_PAYLOAD;
							}
						}
						break;
					}
					case MessageElements::PARSING_FRAME_PAYLOAD:
					{
						// Parse the frame payload based on the frame type
						switch (frame_type)
						{
							case 0x01:  // HEADERS frame
							{
								// Parse the HEADERS frame payload
								// Extract the ':path' pseudo-header using the helper function
								std::string path = extract_header_value(frame_payload_buffer, ":path");

								// Split the path into pathname and query parameters
								size_t query_start = path.find('?');
								if (query_start != std::string::npos)
								{
									request->pathname = path.substr(0, query_start);
									parse_query_params(path.substr(query_start + 1), request->query_params);
								}
								else
								{
									request->pathname = path;
								}

								// Extract other headers and populate the headers map
								//...

								break;
							}
							case 0x09:  // CONTINUATION frame
							{
								// Parse the CONTINUATION frame payload
								// Extract headers and add them to the headers map
								//...

								break;
							}
							// Handle other frame types as needed
							default:
							{
								// Ignore unknown or unsupported frame types for now
								break;
							}
						}

						frame_payload_buffer.clear();
						current_state = MessageElements::READING_FRAME_HEADER;
						break;
					}
				}  // switch (current_state)
			}  // while (!chunk.empty())
		}  // void process_chunk(std::string_view chunk) override
	};  // class HTTPProtocolParser

	std::shared_ptr<abstract::Protocol::ProtocolParser> HTTP2::get_praser()
	{
		return std::make_shared<HTTP2::HTTPProtocolParser>();
	}
}  // namespace protocol