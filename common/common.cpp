// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
#include "common.h"

point2 matrix_transform::apply(const point2 & pt) {
	point2 res;
	res.x = coordinates[0] * pt.x
		+ coordinates[1] * pt.y
		+ coordinates[2];
	res.y = coordinates[3] * pt.x
		+ coordinates[4] * pt.y
		+ coordinates[5];
	return res;
}
