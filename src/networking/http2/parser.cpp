#include <iostream>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <any>
#include <sstream>
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* !HAVE_CONFIG_H */

#define NGHTTP2_NO_SSIZE_T
#include <nghttp2/nghttp2.h>

// Simulated class for handling HTTP/2 request body
class TemporaryFileStringStream
{
public:
	void write(std::string_view data) { buffer << data; }
	std::string str() const { return buffer.str(); }

private:
	std::stringstream buffer;
};

// HTTP/2 request structure
class Request
{
public:
	std::string pathname;
	std::unordered_map<std::string, std::any> query_params;
	std::unordered_map<std::string, std::any> headers;
	TemporaryFileStringStream body;
};

// Function to parse query parameters from a URL
std::unordered_map<std::string, std::any> parse_query_params(const std::string& path)
{
	std::unordered_map<std::string, std::any> params;
	size_t pos = path.find('?');
	if (pos != std::string::npos)
	{
		std::string query = path.substr(pos + 1);
		std::istringstream query_stream(query);
		std::string pair;
		while (std::getline(query_stream, pair, '&'))
		{
			size_t eq = pair.find('=');
			if (eq != std::string::npos)
			{
				params[pair.substr(0, eq)] = pair.substr(eq + 1);
			}
		}
	}
	return params;
}

// Function to decode HPACK headers using nghttp2
std::unordered_map<std::string, std::string> decode_hpack_headers(const std::vector<unsigned char>& raw_headers)
{
	nghttp2_hd_inflater* inflater;
	nghttp2_hd_inflate_new(&inflater);

	std::unordered_map<std::string, std::string> headers;
	const uint8_t* in = raw_headers.data();
	size_t inlen = raw_headers.size();
	nghttp2_nv nv;
	int inflate_flags;

	while (inlen > 0)
	{
		auto result = nghttp2_hd_inflate_hd3(inflater, &nv, &inflate_flags, in, inlen, 0);
		if (result < 0)
		{
			std::cerr << "HPACK decoding error: " << nghttp2_strerror(result) << std::endl;
			break;
		}
		headers[std::string((char*)nv.name, nv.namelen)] = std::string((char*)nv.value, nv.valuelen);
		in += result;
		inlen -= result;
		if (inflate_flags & NGHTTP2_HD_INFLATE_FINAL)
		{
			break;
		}
	}

	nghttp2_hd_inflate_del(inflater);
	return headers;
}

// Function to parse HTTP/2 frames dynamically
void parse_http2_frame(const std::vector<unsigned char>& frame_data, Request& req)
{
	if (frame_data.size() < 9)
	{
		std::cerr << "Invalid HTTP/2 frame: too short" << std::endl;
		return;
	}

	uint8_t frame_type = frame_data[3];
	switch (frame_type)
	{
		case 0x01:  // HEADERS frame
		{
			auto headers = decode_hpack_headers(frame_data);
			if (headers.find(":path") != headers.end())
			{
				req.pathname = headers[":path"];
				req.query_params = parse_query_params(req.pathname);
			}
			for (const auto& [key, value] : headers)
			{
				req.headers[key] = value;
			}
			break;
		}
		case 0x00:  // DATA frame
		{
			uint8_t flags = frame_data[4];
			size_t payload_length = (frame_data[0] << 16) | (frame_data[1] << 8) | frame_data[2];
			size_t offset = 9;

			if (flags & 0x08)
			{  // PADDED flag is set
				uint8_t pad_length = frame_data[offset];
				offset += 1;
				if (payload_length < pad_length + 1)
				{
					std::cerr << "Invalid DATA frame: padding exceeds payload size" << std::endl;
					return;
				}
				payload_length -= (pad_length + 1);
			}

			if (offset + payload_length > frame_data.size())
			{
				std::cerr << "Invalid DATA frame: insufficient payload" << std::endl;
				return;
			}

			std::string payload(frame_data.begin() + offset, frame_data.begin() + offset + payload_length);
			req.body.write(payload);
			break;
		}
		default:
		{
			std::cerr << "Unsupported HTTP/2 frame type: " << static_cast<int>(frame_type) << std::endl;
			break;
		}
	}
}

int main()
{
	// Example HTTP/2 HEADERS frame (replace with real encoded data)
	std::vector<unsigned char> raw_http2_headers = {0x00, 0x00, 0x08, 0x01, 0x05, 0x00, 0x00, 0x00, 0x01, 0x82,
	                                                0x86, 0x44, 0x0f, 0x77, 0x77, 0x77, 0x2e, 0x65, 0x78, 0x61,
	                                                0x6d, 0x70, 0x6c, 0x65, 0x2e, 0x63, 0x6f, 0x6d};

	// Example HTTP/2 DATA frame (replace with real frame data)
	std::vector<unsigned char> raw_http2_data_frame = {0x00, 0x00, 0x05, 0x00, 0x01, 0x00, 0x00,
	                                                   0x00, 0x01, 'H',  'e',  'l',  'l',  'o'};

	Request req;
	parse_http2_frame(raw_http2_headers, req);
	parse_http2_frame(raw_http2_data_frame, req);

	std::cout << "Parsed HTTP/2 Request:" << std::endl;
	std::cout << "  Pathname: " << req.pathname << std::endl;
	std::cout << "  Body: " << req.body.str() << std::endl;

	return 0;
}
