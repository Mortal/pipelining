// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :

#ifndef RASTER_H
#define RASTER_H

#include <gdal.h>
#include <gdal_priv.h>

class RasterReader {
public:
	typedef std::vector<float> value_type;

	RasterReader(GDALRasterBand * band, int xsize, int ysize)
		: band(band)
		, xsize(xsize)
		, ysize(ysize)
		, y(0)
	{
		row.resize(xsize);
		peek();
	}

	bool empty() const { return y >= ysize; }

	RasterReader & operator++() {
		if (y == 0) std::cout << "RasterReader: Read first row" << std::endl;
		++y;
		if (!empty()) peek();
		else std::cout << "RasterReader: Finish reading" << std::endl;
		return *this;
	}

	const value_type & operator*() const { return row; }

private:
	void peek() {
		band->RasterIO(GF_Read,
					   0, y, // offset
					   xsize, 1, // size
					   &row[0], // output
					   xsize, 1, // output size
					   GDT_Float32, //Type
					   0, 0); // byt offset
	}

	GDALRasterBand * band;
	const int xsize;
	const int ysize;
	int y;
	std::vector<float> row;
};

template <typename InputStream>
void write_raster(InputStream & inputStream, GDALRasterBand * band, int xsize) {
	std::cout << "write_raster: Begin output" << std::endl;
	for (int y = 0; !inputStream.empty(); ++y, ++inputStream) {
		band->RasterIO(GF_Write,
					   0, y, //offset
					   xsize, 1, //size
					   const_cast<void *>( (const void *)&(*inputStream)[0]),
					   xsize, 1, //input size
					   GDT_Float32, //Type
					   0, 0); //byte offset
	}
}

#endif // RASTER_H
