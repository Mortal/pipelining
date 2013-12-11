// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :

#include "../common/common.h"
#include <tpie/pipelining.h>
#include "factory_helpers.h"
#include "map.h"
#include "pointgenerator.h"
#include "raster.h"
#include "filler.h"
#include "point_to_raster.h"

namespace tp = tpie::pipelining;

int main() {
	GDALRasterBand * in;
	GDALRasterBand * out;

	tp::passive_sorter<map_point, map_point::from_yorder> ps;

	tp::pipeline p_ = pointGenerator() 
		| pointMap([](point2 x){return x;})
		| ps.input();
	
	tp::pipeline p=
		rasterReader(in)
		| filler(ps.output() | tp::pull_peek()) 
		| tp::sort(value_point::yorder())
		| pointToRaster()
		| rasterWriter(out);
	
	p();

	return 0; 

}
