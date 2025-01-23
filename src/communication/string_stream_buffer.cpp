#include <iostream>
#include "./string_stream_buffer.h"

int main()
{
	try
	{
		TemporaryFileStringStream frw;

		frw.write("Hello, World!\t");
		frw.write("This is a temporary file test.");

		while (true)
		{
			auto chunk = frw.read_next_chunk(32);  // Read in chunks of 16 bytes
			if (chunk.empty()) break;
			std::cout << "Read: " << chunk << std::endl;
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}

	return 0;
}
