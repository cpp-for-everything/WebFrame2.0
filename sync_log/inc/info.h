#pragma once

#include "interface.h"

namespace logger {
	class InfoLogger : public LoggerInterface {
		virtual void on_flush() final { (*stream) << ("INFO: " + buffer.str()); }

	public:
		InfoLogger(std::ostream* os = nullptr) : LoggerInterface(os) {}
		InfoLogger(InfoLogger&& other) : LoggerInterface(other) {}
		InfoLogger(const InfoLogger& other) : LoggerInterface(other) {}
	};
}  // namespace logger
