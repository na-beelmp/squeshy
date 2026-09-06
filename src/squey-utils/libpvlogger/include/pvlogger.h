/* * MIT License
 *
 * © ESI Group, 2015
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 *
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 *
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef __PVLOGGER_H__
#define __PVLOGGER_H__

#include <ctime>
#include <iostream>
#include <mutex>
#include <sstream>
#include <stack>
#include <string>
#include <array>
#include <utility>

#include <sys/time.h>

enum log_level_t {
	PVLOGGER_FATAL,
	PVLOGGER_ERROR,
	PVLOGGER_WARN,
	PVLOGGER_INFO,
	PVLOGGER_DEBUG,
	PVLOGGER_TRACE,

	PVLOGGER_LEVELS_COUNT
};

namespace pvlogger
{

namespace __impl
{
class logger_stream;
struct null_stream;
}
/**
 * Check if the library log level is greater or equal to the logger stream log level.
 */
bool level_is_greater_or_equal_to(log_level_t logger_log_level);

/**
 * Returns the library log level.
 */
log_level_t level();

/**
 * Set log level.
 */
void set_level(log_level_t level);

/**
 * Push specified log level on the stack.
 */
void push_level(log_level_t level);

/**
 * Pop last pushed log level from the stack.
 */
bool pop_level();

void enable_log_level(bool enabled);
void enable_log_time(bool enabled);
void enable_colors(bool enabled);

namespace font
{

enum color_t {
	FG_RED = 31,
	FG_GREEN = 32,
	FG_YELLOW = 33,
	FG_BLUE = 34,
	FG_MAGENTA = 35,
	FG_CYAN = 36,
	FG_DEFAULT = 39,
};

const char* bold();
const char* bold_off();
std::string bold(const std::string& text);

const char* italic();
const char* italic_off();
std::string italic(const std::string& text);

const char* underline();
const char* underline_off();
std::string underline(const std::string& text);

const char* default_color();
const char* reset();
}

namespace __impl
{

/**
 * Singleton class handling the logger configuration.
 */
class logger
{
	friend class logger_stream;

  public:
	/**
	 * Singleton access
	 */
	static logger& get();

  public:
	static log_level_t level();
	static void set_level(log_level_t level);

	static void push_level(log_level_t level);
	static bool pop_level();

	static void enable_log_level(bool enabled);
	static void enable_log_time(bool enabled);
	static void enable_colors(bool enabled);

  private:
	static bool print_level();
	static const std::string& level_string(log_level_t level);

	static bool print_time();

	static bool print_colors();
	static pvlogger::font::color_t level_color(log_level_t level);

	static size_t header_size();

  private:
	/**
	 * Lock the internal std::mutex.
	 */
	static void lock();

	/**
	 * Unlock the internal std::mutex.
	 */
	static void unlock();

  private:
	/**
	 * Default constructor converting to enum the library log level stored
	 * in the environment variable PVLOGGER_LEVEL as a string.
	 */
	logger();

  private:
	logger(const logger&) = delete;
	logger& operator=(const logger&) = delete;

  private:
	using log_levels_t =
	    std::array<std::pair<std::string, pvlogger::font::color_t>, PVLOGGER_LEVELS_COUNT>;

  private:
	log_level_t _log_level;
	log_levels_t _log_levels;

	bool _print_level;
	bool _print_time;
	bool _print_colors;

	std::stack<log_level_t> _log_levels_stack;

	std::mutex _mutex;
};

/**
 * Thread-safe logging class supporting several log levels and compliant with ostream syntax.
 */
class logger_stream
{
  public:
	/**
	 * Initialization constructor
	 */
	logger_stream(log_level_t log_level);

	/**
	 * Copy constructor
	 */
	logger_stream(const logger_stream& rhs);

	/**
	 * Destructor
	 */
	~logger_stream();

  public:
	/**
	 * Stream operator handling the differents types
	 */
	template <typename T>
	inline logger_stream& operator<<(T&& item)
	{
		if (pvlogger::level_is_greater_or_equal_to(_logger_stream_log_level)) {

			write_header_to_stream();

			if (_endl) {
				_string_stream << std::string(logger::header_size(), ' ');
				_endl = false;
			}
			_string_stream << item;
		}

		return *this;
	}

	/**
	 * Stream operator handling the differents manipulators (like std::endl)
	 */
	logger_stream& operator<<(std::ostream& (*F)(std::ostream&));

  private:
	void write_header_to_stream();

	std::string now();

  private:
	bool _init = true;
	bool _endl = false;
	log_level_t _logger_stream_log_level;
	std::stringstream _string_stream;
};

/**
 * Null stream class used by debug() and trace() in release mode
 * in order to prevent any code to be generated.
 */
struct null_stream {

	template <typename T>
	inline null_stream& operator<<(T const&)
	{
		return *this;
	}

	inline null_stream& operator<<(std::ostream& (*)(std::ostream&)) { return *this; }
};

} // __impl

// Disable this warning as we want these functions static as they may differ on
// compiler flags for the current Translation Unit.
// Could be fix poviding a pvlogger and a pvlogger_dbg and link with the correct
// based on the DNDEBUG flags.
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic push

static __impl::logger_stream fatal()
{
	return __impl::logger_stream(PVLOGGER_FATAL);
}
static __impl::logger_stream error()
{
	return __impl::logger_stream(PVLOGGER_ERROR);
}
static __impl::logger_stream warn()
{
	return __impl::logger_stream(PVLOGGER_WARN);
}
static __impl::logger_stream info()
{
	return __impl::logger_stream(PVLOGGER_INFO);
}
#ifndef NDEBUG
static __impl::logger_stream debug()
{
	return __impl::logger_stream(PVLOGGER_DEBUG);
}
static __impl::logger_stream trace()
{
	return __impl::logger_stream(PVLOGGER_TRACE);
}
#else
static __impl::null_stream debug()
{
	return __impl::null_stream();
}
static __impl::null_stream trace()
{
	return __impl::null_stream();
}
#endif

#pragma GCC diagnostic pop

} // pvlogger

std::ostream& operator<<(std::ostream& os, pvlogger::font::color_t code);

#endif // __PVLOGGER_H__
