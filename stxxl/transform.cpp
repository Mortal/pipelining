// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :

#include "../common/common.h"
#include "map.h"
#include "pointgenerator.h"
#include <stxxl/stream>
#include <stxxl/vector>
#include <stxxl/sorter>
#include "compare.h"
#include "filler.h"
#include "point_to_raster.h"
#include "raster.h"

namespace sxs = stxxl::stream;

int main(int argc, char ** argv) {
	program_options options;
	if (!options.parse_args(argc, argv)) return 1;

	static const size_t memory = options.memory * 1024 * 1024;

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

	// std::vector memory usage is at most twice the size of the elements.
	const size_t row_memory = sizeof(float) * xsize * 2;

	// Only one sorter in phase 1.
	const size_t phase1_sort_memory = memory;

	// Memory per sorter for the two sorters in phase 2.
	const size_t phase2_sort_memory = (memory - row_memory) / 2;

	// Row from previous phase is not deallocated automatically,
	// so we have two rows and one sorter in memory in phase 3.
	const size_t phase3_sort_memory = memory - 2*row_memory;

	point_generator pg(options.outputxsize, options.outputysize);

	typedef point_mapper<point_generator, matrix_transform> mapped_pg_type;
	mapped_pg_type mapped_pg(pg, options.transform, xsize, ysize);

	typedef sxs::runs_creator<mapped_pg_type, map_point_value_less> rc_type;
	rc_type rc(mapped_pg, map_point_value_less(), phase1_sort_memory);

	typedef sxs::runs_merger<rc_type::sorted_runs_type, map_point_value_less> rm_type;
	rm_type rm(rc.result(), map_point_value_less(), phase2_sort_memory);
	std::cout << "Initialized runs merger 1" << std::endl;

	typedef RasterReader raster_reader_type;
	raster_reader_type raster_reader(in_band, xsize, ysize);

	typedef Filler<rm_type, raster_reader_type> filler_type;
	filler_type filler(rm, raster_reader, nodata);

	typedef sxs::runs_creator<filler_type, value_point_less> rc_filler_type;
	rc_filler_type rc_filler(filler, value_point_less(), phase2_sort_memory);

	typedef sxs::runs_merger<rc_filler_type::sorted_runs_type, value_point_less> rm_filler_type;
	rm_filler_type rm_filler(rc_filler.result(), value_point_less(), phase3_sort_memory);
	std::cout << "Initialized runs merger 2" << std::endl;

	typedef PointToRaster<rm_filler_type> point_to_raster_type;
	point_to_raster_type point_to_raster(rm_filler, options.outputxsize, options.outputysize, nodata);
	std::cout << "Initialized point_to_raster" << std::endl;

	write_raster(point_to_raster, out_band, options.outputxsize);
	std::cout << "I am now done" << std::endl;

	return 0;
}
