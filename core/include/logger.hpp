#pragma once

#include <fmt/core.h>
#include <fmt/color.h>
#include <source_location>

namespace logger
{
	struct fmt_location
	{
		const char* fmt;
		std::source_location location;

		fmt_location(const char* fmt, const std::source_location& location = std::source_location::current())
			: fmt(fmt)
			, location(location)
		{
		}
	};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
	template <typename... Args>
	void log(fmt_location format, fmt::color fgcol, Args&&... args)
	{
		int size_s = std::snprintf(nullptr, 0, format.fmt, std::forward<Args>(args)...);
		char* buf = new char[size_s + 1];
		std::snprintf(buf, size_s + 1, format.fmt, std::forward<Args>(args)...);
		fmt::print(fg(fgcol), "{}: {}\n", format.location.function_name(), buf);

		delete[] buf;
	}
#pragma GCC diagnostic pop
	template <typename... Args>
	void info(fmt_location format, Args&&... args)
	{
		fmt::print(fg(fmt::color::blue), "[INFO ] ");
		log(format, fmt::color::blue, std::forward<Args>(args)...);
	}

	template <typename... Args>
	void warn(fmt_location format, Args&&... args)
	{
		fmt::print(fg(fmt::color::yellow), "[WARN ] ");
		log(format, fmt::color::yellow, std::forward<Args>(args)...);
	}

	template <typename... Args>
	void error(fmt_location format, Args&&... args)
	{
		fmt::print(fg(fmt::color::red), "[ERROR] ");
		log(format, fmt::color::red, std::forward<Args>(args)...);
	}

	template <typename... Args>
	void debug(fmt_location format, Args&&... args)
	{
		fmt::print(fg(fmt::color::green), "[DEBUG] ");
		log(format, fmt::color::green, std::forward<Args>(args)...);
	}
}; // namespace logger
