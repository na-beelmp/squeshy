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

#include <pvcop/types/factory.h>

#include <pvcop/types/formatter/formatter_interface.h>
#include <pvcop/types/impl/formatter_factory.h>

bool pvcop::types::factory::is_registered(const std::string& name)
{
	return not __impl::formatter_factory::type(name).empty();
}

/**
 * Creates a new formatter instance using its name and its parameters
 *
 * @param name the formatter name
 * @param parameters the formatter parameters string
 *
 * @return a pointer on the new parameter
 */
pvcop::types::formatter_interface* pvcop::types::factory::create(const std::string& name,
                                                                 const std::string& parameters)
{
	return __impl::formatter_factory::create(name, parameters);
}
