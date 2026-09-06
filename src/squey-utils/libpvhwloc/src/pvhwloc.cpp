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

#include <pvhwloc.h>

#include <hwloc.h>

pvhwloc::pvhwloc()
{
	int depth;
	hwloc_topology_t topology;

	// init hwloc topology
	hwloc_topology_init(&topology);
	hwloc_topology_load(topology);

	// physical cores
	_core_count = 0;
	depth = hwloc_get_type_depth(topology, HWLOC_OBJ_CORE);
	if (depth != HWLOC_TYPE_DEPTH_UNKNOWN) {
		_core_count = hwloc_get_nbobjs_by_depth(topology, depth);
	}

	// logical cores
	_thread_count = 0;
	depth = hwloc_get_type_depth(topology, HWLOC_OBJ_PU);
	if (depth != HWLOC_TYPE_DEPTH_UNKNOWN) {
		_thread_count = hwloc_get_nbobjs_by_depth(topology, depth);
	}

	// caches sizes
	hwloc_obj_t obj;
	for (obj = hwloc_get_obj_by_type(topology, HWLOC_OBJ_PU, 0); obj; obj = obj->parent) {
		if (hwloc_obj_type_is_cache(obj->type)) {
			_cache_level_sizes.push_back(obj->attr->cache.size);
		}
	}

	// destroy hwloc topology
	hwloc_topology_destroy(topology);
}
