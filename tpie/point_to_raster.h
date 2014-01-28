// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
#ifndef POINT_TO_RASTER_H
#define POINT_TO_RASTER_H

#include <tpie/pipelining.h>
#include <tpie/array.h>

namespace tp=tpie::pipelining;

template <typename dest_t>
class PointToRaster: public tp::node {
public:
	PointToRaster(dest_t && dest): dest(std::move(dest)) {}

	void prepare() override {
		xsize = fetch<int>("outputxsize");
		ysize = fetch<int>("outputysize");
		set_minimum_memory(tpie::array<float>::memory_usage(xsize));
	}

	void begin() override {
		nodata = fetch<float>("nodata");
		row.resize(xsize, nodata);
		y=0;
	}

	void push(const value_point & p) {
		while (p.point.y != y) {
			dest.push(row);
			std::fill(row.begin(), row.end(), nodata);
			++y;
		}
		row[p.point.x] = p.value;
	}

	void end() override {
		// Ensure we have pushed all rows before `ysize`
		push(value_point{point2{0,ysize},nodata});
		row.resize(0);
	}

private:
	dest_t dest;
	tpie::array<float> row;
	int y;
	int xsize;
	int ysize;
	float nodata;
};

typedef tp::pipe_middle<tp::factory<PointToRaster> > pointToRaster;

#endif //POINT_TO_RASTER_H
