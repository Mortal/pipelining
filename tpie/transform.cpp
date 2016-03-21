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
		| fill_output_points(output_point_sorter.output())
		| tp::sort(value_point::yorder())
		| write_raster(&output);

	tpie::stream_size_type n = input.cell_count() + output.cell_count();
	tpie::progress_indicator_arrow a("", n);
	int ixsize, iysize; std::tie(ixsize, iysize) = input.dimensions();
	int oxsize, oysize; std::tie(oxsize, oysize) = output.dimensions();
	p.forward("xsize", ixsize);
	p.forward("ysize", iysize);
	p.forward("nodata", input.nodata_value());
	p.forward("outputxsize", oxsize);
	p.forward("outputysize", oysize);
	p.plot();
	p(n, a, TPIE_FSI);
	return 0;
}
