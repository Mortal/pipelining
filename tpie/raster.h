// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
#ifndef RASTER_H
#define RASTER_H
#include <gdal.h>

template <typename dest_t>
class RasterReader : public tp::node {
public:
	RasterReader(dest_t && dest, GDALRasterBand * band):
		dest(std::move(dest)), band(band) {
		add_push_destination(dest);
		set_name("RasterReader");
	}

	// TODO memory usage

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
	RasterWriter(GDALRasterBand * band): band(band) {
	}

	//TODO fetch xsize and ysize

	void push(const tpie::array<float> & row) {
		band->RasterIO(GF_WRITE,
					   0, y, //offset
					   xsize, 1, //size
					   const_cast<void *>( (void *)row.get()),
					   xsize, 1, //input size
					   GDT_Float32, //Type
					   0, 0); //byte offset
		++y;
		tpie::increment_bytes_written(xsize * sizeof(T));
	}
private:
	GDALRasterBand * band;
	int xsize;
	int ysize;
};

typedef tp::pipe_begin<tp::factory<RasterReader, GDALRasterBand*> > rasterReader;
typedef tp::pipe_end<tp::term_factory<RasterWriter, GDALRasterBand*> > rasterWriter;

#endif //RASTER_H
