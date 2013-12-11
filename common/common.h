// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :

#ifndef COMMON_COMMON_H
#define COMMON_COMMON_H

#include <cstdint>
#include <string>

struct point2;

struct matrix_transform {
	double coordinates[9];

	point2 apply(const point2 &);
};

struct program_options {
	std::string input_file;
	std::string output_file;

	matrix_transform transform;
};

struct point2 {
	int x;
	int y;

	point2(int x, int y) : x(x), y(y) {}
};

#endif // COMMON_COMMON_H
