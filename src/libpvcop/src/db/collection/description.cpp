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

#include <db/collection/description.h>

#include <type_traits>
#include <fstream>
#include <filesystem>

static constexpr const char* filename = "description";
static constexpr int current_version = 2;

/*****************************************************************************
 * pvcop:db::description::load
 *****************************************************************************/

void pvcop::db::description::load(const std::string& rootdir,
                                  const process_header_f& process_header,
                                  const process_column_f& process_column)
{
	size_t version;
	size_t index;
	size_t row_count;
	size_t column_count;
	std::string type;
	std::string column_path;
	std::string dict_path;

	std::string input_filename = rootdir + filename;
	std::ifstream ifs{std::filesystem::path(input_filename)};

	if (!ifs.is_open()) {
		throw std::ios::failure(
		    std::string("can not open file '" + input_filename + "' for reading"));
	}

	ifs.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	/* read the description format version (which is actually unused)
	 */
	ifs >> version;

	/* read the collection geometry
	 */
	ifs >> row_count;
	ifs >> column_count;

	/* process the collection geometry
	 */
	process_header(row_count, column_count);

	/* read the columns informations and process them
	 */
	for (size_t i = 0; i < column_count; ++i) {
		ifs >> index;
		ifs >> type;

		if (version == 1 and type != "string") {
			type = "number_" + type;
		}

		column_path = rootdir + std::to_string(index) + data_file_extension;
		dict_path = rootdir + std::to_string(index) + dict_file_extension;

		process_column(column_path, dict_path, i, type);
	}
}

/*****************************************************************************
 * pvcop:db::description::append_column
 *****************************************************************************/

void pvcop::db::description::append_column(const std::string& rootdir, const size_t column_count, const get_column_info_f& get)
{
	std::string output_filename = rootdir + filename;
	std::ofstream ofs{std::filesystem::path(output_filename), std::ios::binary};

	if (!ofs.is_open()) {
		throw std::ios::failure(
		    std::string("can not open file '" + output_filename + "' for writing"));
	}

	size_t i = column_count-1;
	type_t type = get(i);
	ofs << i << " " << type << std::endl;
}

/*****************************************************************************
 * pvcop:db::description::remove_column
 *****************************************************************************/

void pvcop::db::description::remove_column(const std::string& rootdir, size_t column_index)
{
	std::string io_filename = rootdir + filename;
	std::ifstream ifs(io_filename);

	if (!ifs.is_open()) {
		throw std::ios::failure(
		    std::string("can not open file '" + io_filename + "' for reading"));
	}

	std::vector<std::string> content;
	for (std::string line; std::getline(ifs, line); /**/) {
		content.push_back(line);
	}
	ifs.close();

	content.erase(content.begin() + 3 /* header */ + column_index);
	content[2] = std::to_string(std::stoll(content[2]) -1); // decrement column count
	
	std::ofstream ofs{std::filesystem::path(io_filename), std::ios::binary};
	for (const auto &line : content) ofs << line << std::endl;
}

/*****************************************************************************
 * pvcop:db::description::save
 *****************************************************************************/

void pvcop::db::description::save(const std::string& rootdir,
                                  const size_t row_count,
                                  const size_t column_count,
                                  const get_column_info_f& get)
{
	std::string output_filename = rootdir + filename;
	std::ofstream ofs{std::filesystem::path(output_filename), std::ios::binary};

	if (!ofs.is_open()) {
		throw std::ios::failure(
		    std::string("can not open file '" + output_filename + "' for writing"));
	}

	ofs.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	/* write the actual format version (which is actually unused)
	 */
	ofs << current_version << std::endl;

	/* write the collection geometry
	 */
	ofs << row_count << std::endl;
	ofs << column_count << std::endl;

	/* write the columns informations
	 */
	for (size_t i = 0; i < column_count; ++i) {
		type_t type = get(i);
		ofs << i << " " << type << std::endl;
	}
}
