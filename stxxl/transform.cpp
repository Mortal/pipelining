// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :

#include "../common/common.h"
#include "map.h"
#include "pointgenerator.h"
#include <stxxl/stream>
#include <stxxl/vector>
#include <stxxl/sorter>
#include "compare.h"

namespace sxs = stxxl::stream;

int main(int argc, char ** argv) {
	program_options po;
	if (!po.parse_args(argc, argv)) return 1;

	static const int runs_creator_memory = 10 * 1024 * 1024;
	static const int runs_merger_memory = runs_creator_memory;

	point_generator pg(po.outputxsize, po.outputysize);

	typedef point_mapper<point_generator, matrix_transform> mapped_pg_type;

	mapped_pg_type mapped_pg(pg, po.transform);

	typedef sxs::runs_creator<mapped_pg_type, map_point_value_less> rc_type;

	rc_type rc(mapped_pg, map_point_value_less(), runs_creator_memory);

	typedef sxs::runs_merger<rc_type::sorted_runs_type, map_point_value_less> rm_type;

	rm_type rm(rc.result(), map_point_value_less(), runs_merger_memory);

	while (!rm.empty()) {
		map_point pt = *rm;
		std::cout << pt.from.x << ' ' << pt.from.y << ' ' << pt.to.x << ' ' << pt.to.y << std::endl;
		++rm;
	}
	return 0;
}
