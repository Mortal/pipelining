// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 et :

#include "../common/common.h"
#include <gdal.h>
#include <gdal_priv.h>
#include <tpie/file_stream.h>
#include <tpie/progress_indicator_arrow.h>
#include <tpie/sort.h>

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

	tpie::stream_size_type in_n = xsize * static_cast<tpie::stream_size_type>(ysize);
	tpie::stream_size_type out_n = options.outputxsize * static_cast<tpie::stream_size_type>(options.outputysize);
	tpie::stream_size_type n = in_n + out_n;
	tpie::progress_indicator_arrow pi("Transform", n);
	tpie::fractional_progress fp(&pi); 
	fp.id() << __FILE__ << __FUNCTION__;

	tpie::fractional_subindicator p_gen(fp, "gen", TPIE_FSI, out_n, "Generating points");
	tpie::fractional_subindicator p_sort1(fp, "sort1", TPIE_FSI, out_n, "Sorting points");
	tpie::fractional_subindicator p_fill(fp, "fill", TPIE_FSI, n, "Filling points from raster");
	tpie::fractional_subindicator p_sort2(fp, "sort2", TPIE_FSI, out_n, "Sorting points");
	tpie::fractional_subindicator p_write(fp, "write", TPIE_FSI, out_n, "Writing points to raster");
	fp.init();

// Generate points in the output raster
tpie::file_stream<map_point> stream1; stream1.open();
for (int y = 0; y < outputysize; ++y)
    for (int x = 0; x < outputxsize; ++x) {
        map_point p = { f({x, y}), {x, y} };
        if (0 <= p.from.x && p.from.x < xsize && 0 <= p.from.y && p.from.y < ysize)
            stream1.write(p);
    }
tpie::sort(stream1);
stream1.seek(0);  // Seek to beginning of stream
tpie::file_stream<value_point> stream2; stream2.open();
tpie::array<float> row1(xsize);
for (int y = 0; y < ysize; ++y) {
    input.read_next_row(&row1);
    while (stream1.can_read() && stream1.peek().from.y == y) {
        map_point p = stream1.read();
        stream2.write(value_point{ p.to, row1[p.from.x] });
    }
}
stream1.close();
tpie::sort(stream2);
stream2.seek(0);  // Seek to beginning of stream
tpie::array<float> row2(outputxsize); 
for (int y = 0; y < outputysize; ++y) {
    for (int x = 0; x < outputxsize; ++x) row2[x] = nodata;
    while(stream2.can_read() && stream2.peek().point.y == y) {
        value_point p = stream2.read();
        row2[p.point.x] = p.value; 
    }
    output.write_next_row(&row2);
}
stream2.close();

	return 0; 
}
