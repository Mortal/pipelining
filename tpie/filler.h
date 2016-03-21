// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
#ifndef FILLER_H
#define FILLER_H

#include <tpie/pipelining.h>
#include <tpie/array.h>

namespace tp=tpie::pipelining;

/**
 * Fills values from raster rows into map_points to generate value_points
 */
template <typename psf_t>
struct Filler {
	template <typename dest_t>
	class type: public tp::node {
	public:
		type(dest_t dest, psf_t point_source_factory)
			: dest(std::move(dest)),
			  point_source(point_source_factory.construct()) {
			add_push_destination(dest);
			add_pull_source(point_source);
		}

		void begin() override {
			y=0;
			nodata=fetch<float>("nodata");
			skip();
		}
		
		void push(const tpie::array<float> & row) {
			// For every row ind all the output points that get input points from this row
			while (canPeek && peekItem.from.y == y) {
				map_point p = peekItem;
				skip();
				if (row[p.from.x] == nodata) continue;
				dest.push(value_point{p.to, row[p.from.x]});
			}
			++y;
		}

		void skip() {
			canPeek = point_source.can_pull();
			if (canPeek) peekItem = point_source.pull();
		}

	private:
		int y;
		float nodata;
		bool canPeek;
		map_point peekItem;
		dest_t dest;
		typename psf_t::constructed_type point_source;
	};
};

template <typename psf_t>
tp::pipe_middle<tp::tempfactory<Filler<psf_t>, psf_t> > fill_output_points(psf_t psf) {
	return {std::move(psf)};
}

#endif //FILLER_H
