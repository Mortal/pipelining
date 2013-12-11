// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
#ifndef POINTGENERATOR_H
#define POINTGENERATOR_H

template <typename dest_t>
class PointGenerator : public tp::node {
public:
	PointGenerator(dest_t && dest)
		: dest(std::move(dest))
	{
		add_push_destination(dest);
		set_name("Point generator");
	}

	virtual void propagate() override {
		int xsize = fetch<int>("outputxsize");
		int ysize = fetch<int>("outputysize");
		set_steps(xsize * ysize);
		forward<size_t>("items", xsize * ysize);
	}

	virtual void go() override {
		int xsize = fetch<int>("outputxsize");
		int ysize = fetch<int>("outputysize");
		for (int y = 0; y < ysize; ++y) {
			for (int x = 0; x < xsize; ++x) {
				dest.push(point2(x, y));
				step();
			}
		}
	}
};

#endif // POINTGENERATOR_H
