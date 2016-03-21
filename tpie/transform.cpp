// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :

#include "../common/common.h"
#include <tpie/pipelining.h>
#include "map.h"
#include "pointgenerator.h"
#include "raster.h"
#include "filler.h"
#include <tpie/progress_indicator_arrow.h>

namespace tp = tpie::pipelining;

int main(int argc, char ** argv) {
	program_options options;
	if (!options.parse_args(argc, argv)) return EXIT_FAILURE;

	tpie::tpie_init();
	tpie::get_memory_manager().set_limit(options.memory*1024*1024);
	/// Initialize GDAL
	GDALAllRegister();

	raster_input input = raster_input::open(options.input_file);
	raster_output output = raster_output::create(
		options.output_file, input, options.outputxsize, options.outputysize);
	/// Done initializing GDAL

	tp::passive_sorter<map_point, map_point::from_yorder> output_point_sorter;

	tp::pipeline p_
		= generate_output_points()
		| compute_transformation(options.transform)
		| output_point_sorter.input();

	tp::pipeline p
		= read_raster(&input)
		| fill_output_points(output_point_sorter.output() | tp::pull_peek())
		| tp::sort(value_point::yorder())
		| write_raster(&output);

	tpie::stream_size_type n=xsize*(tpie::stream_size_type)ysize;
	tpie::progress_indicator_arrow a("", n);
	p.forward("xsize", xsize);
	p.forward("ysize", ysize);
	p.forward("nodata", nodata);
	p.forward("outputxsize", options.outputxsize);
	p.forward("outputysize", options.outputysize);
	p.plot();
	p(n, a, TPIE_FSI);
	return 0;
}
