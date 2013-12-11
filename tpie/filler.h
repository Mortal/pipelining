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
		typedef tpie::array<float> item_type; //TODO should be automated

		type(dest_t && dest, psf_t point_source_factory)
			: dest(std::move(dest)),
			  point_source(point_source_factory.construct()) {
			set_name("Filler");
			add_push_destination(dest); //TODO should be automated
			add_pull_source(point_source);
		}

		void begin() override {
			y=0;
			nodata=fetch<float>("nodata");
		}
		
		void push(const tpie::array<float> & row) {
			// For every row ind all the output points that get input points from this row
			while (point_source.can_pull() && point_source.peek().from.y == y) {
				map_point p = point_source.pull();
				if (row[p.from.x] == nodata) continue;
				dest.push(value_point{p.to, row[p.from.x]});
			}
			++y;
		}
	private:
		int y;
		float nodata;
		dest_t dest;
		typename psf_t::constructed_type point_source;
	};
};

template <typename psf_t>
tp::pipe_middle<tp::tempfactory<Filler<psf_t>, psf_t> > filler(psf_t psf) {
	return tp::tempfactory<Filler<psf_t>, psf_t>(psf);
}

#endif //FILLER_H
