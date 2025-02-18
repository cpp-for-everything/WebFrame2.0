#include <unordered_map>
#include <string_view>
#include <any>
#include <condition_variable>
#include "string_stream_buffer.h"
#ifndef REQUEST_H
#define REQUEST_H

namespace protocol
{
	class Request
	{
	public:
		std::string pathname;
		std::unordered_map<std::string, std::any> query_params;
		std::unordered_map<std::string, std::any> headers;
		RawTempFileStream body;

		enum class processing_stages : unsigned char
		{
			not_started = 0,
			pathname,
			query_string,
			headers,
			body,
			done
		};
	};
}  // namespace protocol

#endif  // REQUEST_H