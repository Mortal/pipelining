// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :

#ifndef COMMON_COMMON_H
#define COMMON_COMMON_H

#include <cstdint>
#include <string>
#include <tuple>

struct point2;

struct matrix_transform {
	double coordinates[9];

	point2 operator()(const point2 &);

	typedef point2 result_type;
	typedef point2 argument_type;
};

struct program_options {
	std::string input_file;
	std::string output_file;

	matrix_transform transform;
};

struct point2 {
	int x;
	int y;

	struct yorder {
		bool operator()(const point2 & lhs, const point2 & rhs) const {
			return std::tie(lhs.y, lhs.x) < std::tie(rhs.y, rhs.x);
		}
	};
};

struct value_point {
	point2 point;
	float value;

	struct yorder {
		bool operator()(const value_point & lhs, const value_point & rhs) const {
			return point2::yorder()(lhs.point, rhs.point);
		}
	};
};

struct map_point {
	point2 from;
	point2 to;

	struct from_yorder {
		bool operator()(const map_point & lhs, const map_point & rhs) const {
			return point2::yorder()(lhs.from, rhs.from);
		}
	};
};

#endif // COMMON_COMMON_H
