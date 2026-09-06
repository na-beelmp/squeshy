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

#include <fstream>
#include <filesystem>

#include <pvlogger.h>

static constexpr const char separator = ' ';
static constexpr int current_version = 1;

/*****************************************************************************
 * pvcop::formatter_desc_list::formatter_desc_list
 *****************************************************************************/

pvcop::formatter_desc_list::formatter_desc_list()
= default;

/*****************************************************************************
 * pvcop::formatter_desc_list::formatter_desc_list
 *****************************************************************************/

pvcop::formatter_desc_list::formatter_desc_list(const std::string& filename)
{
	(void)load(filename);
}

/*****************************************************************************
 * pvcop::formatter_desc_list::load
 *****************************************************************************/

void pvcop::formatter_desc_list::load(const std::string& filename)
{
	std::ifstream file{std::filesystem::path(filename)};

	if (!file.is_open()) {
		throw std::ios::failure(std::string("can not open file '" + filename + "' for reading"));
	}

	file.exceptions(std::ifstream::badbit);

	size_t version;
	char c;
	std::string line;
	std::string formatter;
	std::string formatter_params;

	size_t line_count = 1;

	/* read the formatter format version (which is actually unused)
	 */
	file >> version;

	/* the newline must be explicitly read as next std::getline(...) call
	 * will stop on it...
	 */
	file.get(c);

	/* read the formatters list descriptions and create each one
	 */
	while (std::getline(file, line)) {
		size_t pos = line.find_first_of(separator);

		/* there is necessarily a separator in a formatter information
		 * line (even if there is no parameter for the formatter).
		 */
		if (pos == std::string::npos) {
			throw std::ios::failure(std::string("no formatter name at line " +
			                                    std::to_string(line_count) + " in file '" +
			                                    filename + "'"));
		}

		formatter = line.substr(0, pos);

		// skipping the separator
		formatter_params = line.substr(pos + 1);
		emplace_back(formatter, formatter_params);
		++line_count;
	}
}

/*****************************************************************************
 * pvcop::formatter_desc_list::save
 *****************************************************************************/

void pvcop::formatter_desc_list::save(const std::string& filename) const
{
	if (not operator bool()) {
		pvlogger::error() << "Trying to save an empty formatters description " << std::endl;
	}

	std::ofstream file{std::filesystem::path(filename), std::ios::binary};

	if (!file.is_open()) {
		throw std::ios::failure(std::string("can not open file '" + filename + "' for writing"));
	}

	file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	size_t count = size() - 1;

	/* write the formatter format version (which is actually unused)
	 */
	file << current_version << std::endl;

	/* write all but last formatter description with an trailing newline
	 */
	for (size_t i = 0; i < count; ++i) {
		const auto& value = operator[](i);

		file << value.name() << separator << value.parameters() << std::endl;
	}

	/* write the last formatter description without one
	 */
	const auto& value = operator[](count);
	file << value.name() << separator << value.parameters();

	file.close();
}

/*****************************************************************************
 * pvcop::formatter_desc_list::operator==
 *****************************************************************************/

bool pvcop::formatter_desc_list::operator==(const formatter_desc_list& rhs) const
{
	if (size() != rhs.size()) {
		return false;
	}

	return std::equal(begin(), end(), rhs.begin());
}

/*****************************************************************************
 * pvcop::formatter_desc_list::operator!=
 *****************************************************************************/

bool pvcop::formatter_desc_list::operator!=(const formatter_desc_list& rhs) const
{
	return not(*this == rhs);
}
