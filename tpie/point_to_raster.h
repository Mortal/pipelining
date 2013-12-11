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
	typedef value_point item_type; //TODO should be automated

	PointToRaster(dest_t && dest): dest(std::move(dest)) {
		set_name("PointToRaster"); //TODO should be automated
		add_push_destination(dest); //TODO should be automated
	}

	void prepare() override {
		xsize = fetch<int>("outputxsize");
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
		dest.push(row);
		row.resize(0);
	}

private:
	dest_t dest;
	tpie::array<float> row;
	int y;
	int xsize;
	float nodata;
};

typedef tp::pipe_middle<tp::factory<PointToRaster> > pointToRaster;

#endif //POINT_TO_RASTER_H
