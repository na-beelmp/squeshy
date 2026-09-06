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

#include <pvcop/formatter_desc_list.h>

#include <pvlogger.h>
#include <filesystem>

#include <stdexcept>
#include <fstream>

static constexpr const size_t fill_count = 66236;
static const std::string test_filename(std::filesystem::temp_directory_path().string() +  "/test_formatter_desc_list.formatter");
static const pvcop::formatter_desc fmt_desc("a", "b");

/**
 * This program tests the class pvcop::formatter_desc_list
 */
int main()
{
	size_t i;

	/* tests on empty object
	 */
	pvcop::formatter_desc_list fmt_desc_list;

	if (fmt_desc_list == true) {
		pvlogger::error() << "formatter_desc_list::operator bool() should return false"
		                  << std::endl;
		return 1;
	}

	if (fmt_desc_list.size() != 0) {
		pvlogger::error() << "formatter_desc_list should be empty" << std::endl;
		return 1;
	}

	if (fmt_desc_list.begin() != fmt_desc_list.end()) {
		pvlogger::error() << "formatter_desc_list::{begin,end}() should be equal" << std::endl;
		return 1;
	}

	std::string error_msg = "formatter_desc_list::at(0) should throw std::out_of_range";

	try {
		fmt_desc_list.at(0);
	} catch (std::out_of_range& e) {
		error_msg = "";
	} catch (std::exception& e) {
	}

	if (error_msg.size() != 0) {
		pvlogger::error() << error_msg << std::endl;
		return 1;
	}

	/* tests on non-empty object
	 */
	fmt_desc_list.push_back(fmt_desc);

	if (fmt_desc_list == false) {
		pvlogger::error() << "formatter_desc_list::operator bool() should return true" << std::endl;
		return 1;
	}

	if (fmt_desc_list.size() == 0) {
		pvlogger::error() << "formatter_desc_list should not be empty" << std::endl;
		return 1;
	}

	if (fmt_desc_list.begin() == fmt_desc_list.end()) {
		pvlogger::error() << "formatter_desc_list::{begin,end}() should be different" << std::endl;
		return 1;
	}

	error_msg = "";

	try {
		fmt_desc_list.at(0);
	} catch (std::exception& e) {
		error_msg = "formatter_desc_list::at(0) not should throw any exception";
	}

	if (error_msg.size() != 0) {
		pvlogger::error() << error_msg << std::endl;
		return 1;
	}

	error_msg = "formatter_desc_list::at(1) should throw std::out_of_range";

	try {
		fmt_desc_list.at(1);
	} catch (std::out_of_range& e) {
		error_msg = "";
	} catch (std::exception& e) {
	}

	if (error_msg.size() != 0) {
		pvlogger::error() << error_msg << std::endl;
		return 1;
	}

	fmt_desc_list.clear();

	if (fmt_desc_list.size() != 0) {
		pvlogger::error() << "formatter_desc_list::clear() has not work" << std::endl;
		return 1;
	}

	/* tests of ::push_back(...)
	 */
	for (size_t i = 0; i < fill_count; ++i) {
		fmt_desc_list.push_back(fmt_desc);
	}

	if (fmt_desc_list.size() != fill_count) {
		pvlogger::error() << "formatter_desc_list has " << fmt_desc_list.size()
		                  << "elements but it should be " << fill_count << std::endl;
		return 1;
	}

	/* and accessors
	 */
	for (size_t i = 0; i < fmt_desc_list.size(); ++i) {
		const auto& entry = fmt_desc_list[i];
		if (entry != fmt_desc) {
			pvlogger::error() << "formatter_desc_list::operator[](" << i << ") has returned ("
			                  << entry.name() << "," << entry.parameters() << ") instead of"
			                  << fmt_desc.name() << "," << fmt_desc.parameters() << std::endl;
			return 1;
		}
	}

	try {
		for (i = 0; i < fmt_desc_list.size(); ++i) {
			const auto& entry = fmt_desc_list.at(i);
			if (entry != fmt_desc) {
				pvlogger::error() << "formatter_desc_list::at(" << i << ") has returned ("
				                  << entry.name() << "," << entry.parameters() << ") instead of"
				                  << fmt_desc.name() << "," << fmt_desc.parameters() << std::endl;
				return 1;
			}
		}
	} catch (std::exception& e) {
		pvlogger::error() << "formatter_desc_list::at(" << i
		                  << ") has throw an exception: " << e.what() << std::endl;
		return 1;
	}

	/* and iterators
	 */
	i = 0;
	for (const auto& entry : fmt_desc_list) {
		if (entry != fmt_desc) {
			pvlogger::error() << "formatter_desc_list::const_iterator has returned ("
			                  << entry.name() << "," << entry.parameters() << ") instead of ("
			                  << fmt_desc.name() << "," << fmt_desc.parameters()
			                  << ") for the index " << i << std::endl;
			return 1;
		}
		++i;
	}

	fmt_desc_list.clear();

	/* tests of ::emplace_back(...)
	 */
	for (size_t i = 0; i < fill_count; ++i) {
		std::string s = std::to_string(i);
		fmt_desc_list.emplace_back(s, s);
	}

	if (fmt_desc_list.size() != fill_count) {
		pvlogger::error() << "formatter_desc_list has " << fmt_desc_list.size()
		                  << "elements but it should be " << fill_count << std::endl;
		return 1;
	}

	/* tests of saving
	 */
	fmt_desc_list.save(test_filename);

	std::ifstream file(std::filesystem::path{test_filename});
	if (not file.is_open()) {
		pvlogger::error() << "can not open '" << test_filename << "'" << std::endl;
		return 1;
	}

	std::string line;

	/* the file format version
	 */
	std::string s = std::to_string(1);
	size_t line_pos = 0;

	std::getline(file, line);
	if (line != s) {
		pvlogger::error() << "wrong line at " << line_pos << ": '" << line << "' instead of '" << s
		                  << "'" << std::endl;
	}

	++line_pos;
	i = 0;
	while (std::getline(file, line)) {
		s = std::to_string(i) + " " + std::to_string(i);

		if (line != s) {
			pvlogger::error() << "wrong line at " << line_pos << ": '" << line << "' instead of '"
			                  << s << "'" << std::endl;
			return 1;
		}
		++i;
		++line_pos;
	}
	file.close();

	/* tests of loading
	 */
	pvcop::formatter_desc_list fmt_desc_list2;

	fmt_desc_list2.load(test_filename);

	if (fmt_desc_list2.size() != fill_count) {
		pvlogger::error() << "formatter_desc_list2 has " << fmt_desc_list2.size()
		                  << "elements but it should be " << fill_count << std::endl;
		return 1;
	}

	i = 0;
	for (const auto& entry : fmt_desc_list) {
		std::string s = std::to_string(i);
		const pvcop::formatter_desc ref_value(s, s);

		if (entry != ref_value) {
			pvlogger::error() << "formatter_desc_list2[" << i << "] is" << entry.name() << ","
			                  << entry.parameters() << ") instead of (" << ref_value.name() << ","
			                  << ref_value.parameters() << ") for the index " << i << std::endl;
			return 1;
		}
		++i;
	}

	/* tests for equality/inequality
	 */
	if (fmt_desc_list2 != fmt_desc_list) {
		pvlogger::error() << "the 2 formatter_desc_list should be equal" << std::endl;
		return 1;
	}

	fmt_desc_list2.emplace_back("a", "b");

	if (fmt_desc_list2 == fmt_desc_list) {
		pvlogger::error() << "the 2 formatter_desc_list should be different" << std::endl;
		return 1;
	}

	return 0;
}
