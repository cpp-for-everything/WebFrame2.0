#pragma once

#include "interface.h"

namespace logger
{
	class ErrorLogger : public LoggerInterface {
        virtual void on_flush() final { (*stream) << ("ERROR: " + buffer.str()); }
	public:
		ErrorLogger(std::ostream* os = nullptr) : LoggerInterface(os) {}
		ErrorLogger(ErrorLogger&& other) : LoggerInterface(other) {}
		ErrorLogger(const ErrorLogger& other) : LoggerInterface(other) {}
	};
} // namespace protocol
