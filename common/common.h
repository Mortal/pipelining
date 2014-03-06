// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :

#ifndef COMMON_COMMON_H
#define COMMON_COMMON_H

#include <cstdint>
#include <tuple>
#include <iostream>
#include <sstream>
#include <vector>
#include <cmath>

struct point2;

struct matrix_transform {
	static const size_t DIM = 3;
	double coordinates[DIM*DIM];

	point2 operator()(const point2 &);

	typedef point2 result_type;
	typedef point2 argument_type;

	void set(std::vector<double> l) {
		if (l.size() != DIM*DIM)
			throw std::runtime_error("Bad list passed to operator=");
		std::copy(l.begin(), l.end(), coordinates);
	}

	// Multiply by l from the right
	void right_multiply(std::vector<double> l);
};

struct program_options {
	std::string input_file;
	std::string output_file;
	int outputxsize;
	int outputysize;

	size_t memory;

	matrix_transform transform;

	program_options()
		: outputxsize(-1)
		, outputysize(-1)
		, memory(1024)
	{
	}

	bool parse_args(int argc, char ** argv) {
		// If the user specifies transforms A, B, C
		// to be applied in that order to the input,
		// this is equivalent to applying the matrix product CBA to the input.
		// In terms of the output, we must apply
		// (CBA)^-1 = A^-1 B^-1 C^-1  to each output point
		// to get the corresponding input point.
		// Therefore, process the arguments from left to right,
		// for each transformation multiplying the inverse transformation
		// on the right.
		transform.set({
			1, 0, 0,
			0, 1, 0,
			0, 0, 1
		});
		for (int i = 1; i < argc; ++i) {
			std::string arg = argv[i];
			if (arg == "--input") {
				input_file = argv[++i];
			} else if (arg == "--output") {
				output_file = argv[++i];
			} else if (arg == "--outsize") {
				std::stringstream(argv[++i]) >> outputxsize >> outputysize;
			} else if (arg == "--memory") {
				std::stringstream(argv[++i]) >> memory;
			} else if (arg == "--translate") {
				double dx, dy;
				std::stringstream(argv[++i]) >> dx >> dy;
				transform.right_multiply({
					1, 0, -dx,
					0, 1, -dy,
					0, 0, 1
				});
			} else if (arg == "--rotate") {
				double theta;
				std::stringstream(argv[++i]) >> theta;
				transform.right_multiply({
					cos(-theta), sin(-theta), 0,
					-sin(-theta), cos(-theta), 0,
					0, 0, 1
				});
			} else if (arg == "--scale") {
				double factor;
				std::stringstream(argv[++i]) >> factor;
				transform.right_multiply({
					1.0/factor, 0, 0,
					0, 1.0/factor, 0,
					0, 0, 1
				});
			} else if (arg == "--help") {
				std::cerr << argv[0] << " usage: --input <input> --output <output> --memory <MB> --outsize \"width height\" --translate \"<dx> <dy>\"" << std::endl;
				return false;
			}
		}
		if (input_file.empty() || output_file.empty()) {
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
