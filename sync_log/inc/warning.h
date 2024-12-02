#pragma once

#include "interface.h"

namespace logger
{
	class WarningLogger : public LoggerInterface {
        virtual void on_flush() final { (*stream) << ("WARNING: " + buffer.str()); }
	public:
		WarningLogger(std::ostream* os = nullptr) : LoggerInterface(os) {}
		WarningLogger(WarningLogger&& other) : LoggerInterface(other) {}
		WarningLogger(const WarningLogger& other) : LoggerInterface(other) {}
	};
} // namespace protocol
