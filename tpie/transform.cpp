// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :

#include "../common/common.h"
#include <tpie/pipelining.h>
#include "map.h"
#include "pointgenerator.h"
#include "raster.h"
#include "filler.h"
#include "point_to_raster.h"
#include <tpie/progress_indicator_arrow.h>

namespace tp = tpie::pipelining;

int main(int argc, char ** argv) {
	program_options options;
	if (!options.parse_args(argc, argv)) return EXIT_FAILURE;

	tpie::tpie_init();
	tpie::get_memory_manager().set_limit(options.memory*1024*1024);
	/// Initialize GDAL
	GDALAllRegister();

	std::unique_ptr<GDALDataset> in((GDALDataset*)GDALOpen(options.input_file.c_str(), GA_ReadOnly));
	int xsize = in->GetRasterXSize();
	int ysize = in->GetRasterYSize();

	if (options.outputxsize == -1) options.outputxsize=xsize;
	if (options.outputysize == -1) options.outputysize=ysize;

	GDALDriver * driver = GetGDALDriverManager()->GetDriverByName("ENVI");
	std::unique_ptr<GDALDataset> out(driver->Create(options.output_file.c_str(), 
													options.outputxsize, options.outputysize, 
													1, GDT_Float32 , nullptr));

	double geoCoords[6];  //Geographic metadata
	if (in->GetGeoTransform(geoCoords) == CE_None) out->SetGeoTransform(geoCoords);

	const char * sref = in->GetProjectionRef();
	if (sref != NULL) out->SetProjection(sref);

	GDALRasterBand * in_band = in->GetRasterBand(1);
	GDALRasterBand * out_band = out->GetRasterBand(1);

	float nodata;
	int has_nodata = false;
	nodata = in_band->GetNoDataValue(&has_nodata);
	if (!has_nodata) nodata=-9999;
	out_band->SetNoDataValue(nodata);
	/// Done initializing GDAL

	tp::passive_sorter<map_point, map_point::from_yorder> ps;

	tp::pipeline p_
		= generate_output_points()
		| compute_transformation(options.transform)
		| ps.input();

	tp::pipeline p
		= rasterReader(in_band)
		| filler(ps.output() | tp::pull_peek())
		| tp::sort(value_point::yorder())
		| pointToRaster()
		| rasterWriter(out_band);

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
