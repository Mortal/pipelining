// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
#ifndef MAP_H
#define MAP_H
#include "../common/common.h"
#include <tpie/pipelining.h>
namespace tp = tpie::pipelining;

template <typename F>
class PointMap {
public:
	template <typename dest_t>
	class type : public tp::node {
	public:
		typedef point2 item_type;
		
		type(dest_t && dest, F f)
			: f(std::move(f))
			, dest(std::move(dest)) {
			add_push_destination(dest); //TODO should be automated
			set_name("Map");
		}

		void begin() override {
			xsize = fetch<int>("xsize");
			ysize = fetch<int>("ysize");
		}
		
		void push(const point2 & to) {
			point2 from=f(to);
			if (from.x >= 0 && from.x < xsize && from.y >= 0 && from.y < ysize)
				dest.push(map_point{from, to});
		}
		
	private:
		int xsize,ysize;
		F f;
		dest_t dest;
	};
};

template <typename F>
tp::pipe_middle<tp::tempfactory<PointMap<F>, F> > pointMap(const F & f) {
	return tp::tempfactory<PointMap<F>, F>(f);
}

#endif // MAP_H
