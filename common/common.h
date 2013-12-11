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

	point2() = default;
	point2(const point2 &) = default;
	point2(point2 &&) = default;
	point2(int x, int y) : x(x), y(y) {}
};

struct value_point {
	point2 point;
	float value;

	value_point() = default;
	value_point(const value_point &) = default;
	value_point(value_point &&) = default;
	value_point(point2 point, float value): point(point), value(value) {}
};

struct map_point {
	point2 from;
	point2 to;

	map_point() = default;
	map_point(const map_point &) = default;
	map_point(map_point &&) = default;
	map_point(point2 from, point2 to): from(from), to(to) {}
};

#endif // COMMON_COMMON_H
