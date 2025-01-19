#include <unordered_map>
#include <string_view>
#include <any>
#ifndef RESPONSE_H
#define RESPONSE_H

namespace protocol
{
	class Response
	{
		std::unordered_map<std::string_view, std::any> headers;
	};
}  // namespace protocol

#endif  // RESPONSE_H