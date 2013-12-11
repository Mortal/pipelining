// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :

#ifndef COMMON_COMMON_H
#define COMMON_COMMON_H

#include <cstdint>
#include <tuple>
#include <iostream>
#include <sstream>
#include <vector>

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
	int outputxsize;
	int outputysize;

	matrix_transform transform;

	program_options(): outputxsize(-1), outputysize(-1) {}

	bool parse_args(int argc, char ** argv) {
		bool has_transform = false;
		for (int i = 1; i < argc; ++i) {
			std::string arg = argv[i];
			if (arg == "--input") {
				input_file = argv[++i];
			} else if (arg == "--output") {
				output_file = argv[++i];
			} else if (arg == "--outsize") {
				std::stringstream(argv[++i]) >> outputxsize >> outputysize;
			} else if (arg == "--translate") {
				has_transform = true;
				double dx, dy;
				std::stringstream(argv[++i]) >> dx >> dy;
				std::vector<double> M = {
					1, 0, -dx,
					0, 1, -dy,
					0, 0, 1
				};
				std::copy(M.begin(), M.end(), transform.coordinates);
			} else if (arg == "--rotate") {
				has_transform = true;
				double theta;
				std::stringstream(argv[++i]) >> theta;
				std::vector<double> M = {
					cos(-theta), -sin(-theta), 0,
					sin(-theta), cos(-theta), 0,
					0, 0, 1
				};
				std::copy(M.begin(), M.end(), transform.coordinates);
			} else if (arg == "--help") {
				std::cerr << argv[0] << " usage: --input <input> --output <output> --outsize \"width height\" --translate \"<dx> <dy>\"" << std::endl;
				return false;
			}
		}
		if (!has_transform || input_file.empty() || output_file.empty()) {
			std::cerr << "Missing arguments, try --help?" << std::endl;
			return false;
		}
		return true;
	}
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
