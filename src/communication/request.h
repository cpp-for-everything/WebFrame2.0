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
		std::string_view pathname;
		std::unordered_map<std::string_view, std::any> query_params;
		std::unordered_map<std::string_view, std::any> headers;
		TemporaryFileStringStream body;

	public:
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

	class RequestInProcessing
	{
		Request r;
		Request::processing_stages status;
		std::condition_variable processing;
	};
}  // namespace protocol

#endif  // REQUEST_H