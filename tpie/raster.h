// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
#ifndef RASTER_H
#define RASTER_H
#include <gdal.h>
#include <gdal_priv.h>
#include "point_to_raster.h"

template <typename dest_t>
class RasterReader : public tp::node {
public:
	RasterReader(dest_t dest, GDALRasterBand * band):
		dest(std::move(dest)), band(band) {}

	virtual void prepare() override {
		int xsize=fetch<int>("xsize");
		set_minimum_memory(tpie::array<float>::memory_usage(xsize));
	}

	virtual void go() override {
		int xsize=fetch<int>("xsize");
		int ysize=fetch<int>("ysize");
		tpie::array<float> row(xsize);
		for (int y=0; y < ysize; ++y) {
			band->RasterIO(GF_Read,
						   0, y, // offset
						   xsize, 1, // size
						   row.get(), // output
						   xsize, 1, // output size
						   GDT_Float32, //Type
						   0, 0); // byt offset
			dest.push(row);
			tpie::increment_bytes_read(xsize * sizeof(float));
		}
	}
private:
	dest_t dest;
	GDALRasterBand * band;
};

class RasterWriter : public tp::node {
public:
	RasterWriter(GDALRasterBand * band): band(band), y(0) {}

	void propagate() override {
		outputxsize = fetch<int>("outputxsize");
		outputysize = fetch<int>("outputysize");
	}

	void push(const tpie::array<float> & row) {
		band->RasterIO(GF_Write,
					   0, y, //offset
					   outputxsize, 1, //size
					   const_cast<void *>( (const void *)row.get()),
					   outputxsize, 1, //input size
					   GDT_Float32, //Type
					   0, 0); //byte offset
		++y;
		tpie::increment_bytes_written(outputxsize * sizeof(float));
	}
private:
	GDALRasterBand * band;
	int outputxsize;
	int outputysize;
	int y;
};

typedef tp::pipe_begin<tp::factory<RasterReader, GDALRasterBand*> > read_raster;
typedef tp::pipe_end<tp::termfactory<RasterWriter, GDALRasterBand*> > write_raster_rows;

decltype(pointToRaster() | write_raster_rows(nullptr))
write_raster(GDALRasterBand * band) {
	return pointToRaster() | write_raster_rows(band);
}

#endif //RASTER_H
