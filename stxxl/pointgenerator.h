// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
#ifndef POINTGENERATOR_H
#define POINTGENERATOR_H

#include "progress.h"

class point_generator {
public:
	typedef point2 value_type;

	point_generator(int xsize, int ysize)
		: m_xsize(xsize), m_ysize(ysize), m_x(0), m_y(0) {}

	bool empty() const { return m_ysize == m_y; }

	value_type operator*() const { return point2{m_x, m_y}; }

	point_generator & operator++() {
		if (empty()) return *this;
		if (m_x == m_xsize - 1) {
			m_x = 0;
			++m_y;
			set_progress("Generate points", m_y, m_ysize);
		} else {
			++m_x;
		}
		if (empty()) end_progress();
		return *this;
	}

private:
	const int m_xsize;
	const int m_ysize;
	int m_x;
	int m_y;
};

#endif // POINTGENERATOR_H
