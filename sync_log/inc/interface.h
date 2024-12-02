#pragma once

#include <sstream>
#include <iostream>
#include <mutex>

namespace logger
{
	class LoggerInterface {
    protected:
        virtual void on_flush() { (*stream) << buffer.str(); }
	public:
		LoggerInterface(std::ostream* os = nullptr) : stream(os) {}
		void set(std::ostream* os = nullptr) { stream = os; }
		LoggerInterface(LoggerInterface&& other) : stream(other.stream) {}
		LoggerInterface(const LoggerInterface& other) : stream(other.stream) {}

		// Overloaded insertion operator
		template <typename T>
		LoggerInterface& operator<<(const T& value) {
			std::lock_guard<std::mutex> lock(mtx);
			if (stream == nullptr) return *this;
			buffer << value;  // Write to the internal buffer
			return *this;
		}

		// Special handling for std::endl
		LoggerInterface& operator<<(std::ostream& (*manip)(std::ostream&)) {
			std::lock_guard<std::mutex> lock(mtx);
			if (stream == nullptr) return *this;
			if (manip == static_cast<std::ostream& (*)(std::ostream&)>(std::endl)) {
				on_flush();  // Flush the internal buffer to the stream
				buffer.str("");             // Clear the buffer
				buffer.clear();             // Reset any error flags
				(*stream) << manip;         // Apply the manipulator (flush)
			} else if (manip != static_cast<std::ostream& (*)(std::ostream&)>(std::flush)) {
				buffer << manip;  // Apply the manipulator
			}
			return *this;
		}

	protected:
		std::ostream* stream;       // Reference to the output stream
		std::ostringstream buffer;  // Thread-local buffer
		std::mutex mtx;             // Mutex for synchronization
	};
} // namespace protocol
