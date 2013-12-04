#ifndef COMMON_COMMON_H
#define COMMON_COMMON_H

#include <cstdint>
#include <string>

struct point2;

struct matrix_transform {
	uint32_t coordinates[9];

	point2 apply(const point2 &);
};

struct program_options {
	std::string input_file;
	std::string output_file;

	matrix_transform transform;
};

struct point2 {
	uint32_t x;
	uint32_t y;
};

#endif // COMMON_COMMON_H
