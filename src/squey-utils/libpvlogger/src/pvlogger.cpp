//
// MIT License
//
// © ESI Group, 2015
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
//
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
//
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#include <pvlogger.h>

#include <algorithm>
#include <unordered_map>
#include <string>

/******************************************************************************
 *
 * pvlogger
 *
 *****************************************************************************/

bool pvlogger::level_is_greater_or_equal_to(log_level_t logger_log_level)
{
	return level() >= logger_log_level;
}

log_level_t pvlogger::level()
{
	return __impl::logger::level();
}

void pvlogger::set_level(log_level_t level)
{
	__impl::logger::set_level(level);
}

void pvlogger::push_level(log_level_t level)
{
	__impl::logger::push_level(level);
}

bool pvlogger::pop_level()
{
	return __impl::logger::pop_level();
}

void pvlogger::enable_log_level(bool enabled)
{
	__impl::logger::enable_log_level(enabled);
}

void pvlogger::enable_log_time(bool enabled)
{
	__impl::logger::enable_log_time(enabled);
}

void pvlogger::enable_colors(bool enabled)
{
	__impl::logger::enable_colors(enabled);
}

/******************************************************************************
 *
 * __impl::logger
 *
 *****************************************************************************/

pvlogger::__impl::logger::logger()
    : _log_level(PVLOGGER_INFO), _print_level(true), _print_time(false), _print_colors(true)
{
	std::unordered_map<std::string, log_level_t> level_resolver;

	_log_levels[PVLOGGER_FATAL] = std::make_pair("FATAL", pvlogger::font::FG_MAGENTA);
	_log_levels[PVLOGGER_ERROR] = std::make_pair("ERROR", pvlogger::font::FG_RED);
	_log_levels[PVLOGGER_WARN] = std::make_pair("WARN", pvlogger::font::FG_YELLOW);
	_log_levels[PVLOGGER_INFO] = std::make_pair("INFO", pvlogger::font::FG_GREEN);
	_log_levels[PVLOGGER_DEBUG] = std::make_pair("DEBUG", pvlogger::font::FG_CYAN);
	_log_levels[PVLOGGER_TRACE] = std::make_pair("TRACE", pvlogger::font::FG_BLUE);

	level_resolver[_log_levels[PVLOGGER_FATAL].first] = PVLOGGER_FATAL;
	level_resolver[_log_levels[PVLOGGER_ERROR].first] = PVLOGGER_ERROR;
	level_resolver[_log_levels[PVLOGGER_WARN].first] = PVLOGGER_WARN;
	level_resolver[_log_levels[PVLOGGER_INFO].first] = PVLOGGER_INFO;
	level_resolver[_log_levels[PVLOGGER_DEBUG].first] = PVLOGGER_DEBUG;
	level_resolver[_log_levels[PVLOGGER_TRACE].first] = PVLOGGER_TRACE;

	const char* env_var = std::getenv("PVLOGGER_LEVEL");
	std::string log_level = env_var ? env_var : std::string();

	if (!log_level.empty()) {
		std::transform(log_level.begin(), log_level.end(), log_level.begin(), ::toupper);

		auto iter = level_resolver.find(log_level);
		if (iter != level_resolver.end()) {
			_log_level = iter->second;
		}
	}
}

pvlogger::__impl::logger& pvlogger::__impl::logger::get()
{
	static logger instance;

	return instance;
}

log_level_t pvlogger::__impl::logger::level()
{
	return get()._log_level;
}

void pvlogger::__impl::logger::set_level(log_level_t level)
{
	get()._log_level = level;
}

void pvlogger::__impl::logger::push_level(log_level_t level)
{
	get()._log_levels_stack.push(get()._log_level);
	get()._log_level = level;
}

bool pvlogger::__impl::logger::pop_level()
{
	auto log_levels_stack = get()._log_levels_stack;

	if (log_levels_stack.empty() == false) {

		get()._log_level = log_levels_stack.top();
		log_levels_stack.pop();

		return true;
	}

	return false;
}

void pvlogger::__impl::logger::enable_log_level(bool enabled)
{
	get()._print_level = enabled;
}

void pvlogger::__impl::logger::enable_log_time(bool enabled)
{
	get()._print_time = enabled;
}

void pvlogger::__impl::logger::enable_colors(bool enabled)
{
	get()._print_colors = enabled;
}

void pvlogger::__impl::logger::lock()
{
	get()._mutex.lock();
}

void pvlogger::__impl::logger::unlock()
{
	get()._mutex.unlock();
}

bool pvlogger::__impl::logger::print_level()
{
	return get()._print_level;
}

bool pvlogger::__impl::logger::print_time()
{
	return get()._print_time;
}

bool pvlogger::__impl::logger::print_colors()
{
	return get()._print_colors;
}

const std::string& pvlogger::__impl::logger::level_string(log_level_t level)
{
	return get()._log_levels[level].first;
}

pvlogger::font::color_t pvlogger::__impl::logger::level_color(log_level_t level)
{
	return get()._log_levels[level].second;
}

size_t pvlogger::__impl::logger::header_size()
{
	size_t size = (print_level() || print_time()) ? 5 : 0;
	size += print_level() ? 5 : 0;
	size += print_time() ? 12 : 0;
	size += (print_level() && print_time()) ? 1 : 0;

	return size;
}

/******************************************************************************
 *
 * __impl::logger_stream
 *
 *****************************************************************************/

pvlogger::__impl::logger_stream::logger_stream(log_level_t log_level)
    : _logger_stream_log_level(log_level), _string_stream()
{
}

pvlogger::__impl::logger_stream::logger_stream(const logger_stream& rhs)
    : _logger_stream_log_level(rhs._logger_stream_log_level)
    , _string_stream(rhs._string_stream.str())
{
}

// Every stream written to the logger is temporary stored in an internal std::stringstream
// and is only flushed to std::clog uppon deletion in a thread-safe manner.
// As such, all calls to the logging methods are therefore thread-safe.
pvlogger::__impl::logger_stream::~logger_stream()
{
	if (pvlogger::level_is_greater_or_equal_to(_logger_stream_log_level)) {
		__impl::logger::lock();

		std::clog << _string_stream.rdbuf() << std::flush;

		__impl::logger::unlock();
	}
}

void pvlogger::__impl::logger_stream::write_header_to_stream()
{
	if (_init) {

		if (__impl::logger::print_level() || __impl::logger::print_time()) {

			if (__impl::logger::print_colors()) {
				_string_stream << __impl::logger::level_color(_logger_stream_log_level);
			}

			_string_stream << "[ ";

			if (__impl::logger::print_level()) {
				const std::string& level_string =
				    __impl::logger::level_string(_logger_stream_log_level);
				_string_stream << level_string << ((level_string.length() == 4) ? "  " : " ");
			}

			if (__impl::logger::print_time()) {
				_string_stream << now() << " ";
			}

			_string_stream << "] " << pvlogger::font::default_color();
		}

		_init = false;
	}
}

pvlogger::__impl::logger_stream& pvlogger::__impl::logger_stream::
operator<<(std::ostream& (*f)(std::ostream&))
{
	if (pvlogger::level_is_greater_or_equal_to(_logger_stream_log_level)) {
		f(_string_stream);

		const std::string& str = _string_stream.str();
		if (!str.empty()) {
			const char& c = str.back();
			if (c == '\n') {
				_endl = true;
			}
		}
	}

	return *this;
}

std::string pvlogger::__impl::logger_stream::now()
{
	std::stringstream ss;

	time_t rawtime;
	struct tm* timeinfo;
	char buffer[12];

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	timeval current_time;
	gettimeofday(&current_time, nullptr);
	size_t milli = current_time.tv_usec / 1000;

	strftime(buffer, 12, "%H:%M:%S", timeinfo);

	ss << buffer << ":" << std::fixed << milli;

	return ss.str();
}

/******************************************************************************
 *
 * pvlogger::font
 *
 *****************************************************************************/

const char* pvlogger::font::bold()
{
	return "\033[;1m";
}

const char* pvlogger::font::bold_off()
{
	return "\033[;22m";
}

std::string pvlogger::font::bold(const std::string& text)
{
	return bold() + text + bold_off();
}

const char* pvlogger::font::italic()
{
	return "\033[;3m";
}

const char* pvlogger::font::italic_off()
{
	return "\033[;23m";
}

std::string pvlogger::font::italic(const std::string& text)
{
	return italic() + text + italic_off();
}

const char* pvlogger::font::underline()
{
	return "\033[;4m";
}

const char* pvlogger::font::underline_off()
{
	return "\033[;24m";
}

std::string pvlogger::font::underline(const std::string& text)
{
	return underline() + text + underline_off();
}

const char* pvlogger::font::default_color()
{
	return "\033[;39m";
}

const char* pvlogger::font::reset()
{
	return "\033[;0m";
}

std::ostream& operator<<(std::ostream& os, pvlogger::font::color_t code)
{
	return os << "\033[1;" << static_cast<size_t>(code) << "m";
}
