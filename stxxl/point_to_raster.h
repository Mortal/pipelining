// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :

#ifndef POINT_TO_RASTER_H
#define POINT_TO_RASTER_H

template <typename InputStream>
class PointToRaster {
public:
	typedef std::vector<float> value_type;

	PointToRaster(InputStream & inputStream, int xsize, int ysize, float nodata)
		: inputStream(inputStream)
		, xsize(xsize)
		, ysize(ysize)
		, nodata(nodata)
		, y(0)
	{
		row.resize(xsize);
		peek();
	}

	bool empty() const {
		return y >= ysize;
	}

	PointToRaster & operator++() {
		++y;
		if (!empty()) peek();
		return *this;
	}

	const value_type & operator*() const {
		return row;
	}

private:
	void peek() {
		std::fill(row.begin(), row.end(), nodata);
		size_t points = 0;
		while (!inputStream.empty()) {
			value_point p = *inputStream;
			++inputStream;
			if (p.point.y != y) break;
			row[p.point.x] = p.value;
			++points;
		}
	}

	InputStream & inputStream;
	value_type row;
	const int xsize;
	const int ysize;
	const float nodata;
	int y;
};

#endif // POINT_TO_RASTER_H
