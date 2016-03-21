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
#include <memory>
#include <gdal.h>
#include <gdal_priv.h>

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

struct raster_input {
	std::unique_ptr<GDALDataset> input_dataset;
	GDALRasterBand * input_band;

	static raster_input open(std::string filename) {
		std::unique_ptr<GDALDataset> input_dataset {(GDALDataset *) GDALOpen(filename.c_str(), GA_ReadOnly)};
		return {std::move(input_dataset), nullptr};
	}

	std::pair<int, int> dimensions() {
		return {input_dataset->GetRasterXSize(),
				input_dataset->GetRasterYSize()};
	}

	void init_band() {
		if (input_band == nullptr)
			input_band = input_dataset->GetRasterBand(1);
	}

	GDALRasterBand * get_input_band() {
		init_band();
		return input_band;
	}

	float nodata_value() {
		float nodata;
		int has_nodata = false;
		nodata = get_input_band()->GetNoDataValue(&has_nodata);
		if (!has_nodata) nodata = -9999;
		return nodata;
	}

	uint64_t cell_count() {
		uint64_t xsize, ysize;
		std::tie(xsize, ysize) = dimensions();
		return xsize * ysize;
	}
};

struct raster_output {
	std::unique_ptr<GDALDataset> output_dataset;
	GDALRasterBand * output_band;

	static raster_output create(std::string filename, raster_input & input,
								int outputxsize, int outputysize) {
		GDALDriver * driver = GetGDALDriverManager()->GetDriverByName("ENVI");
		int xsize, ysize;
		std::tie(xsize, ysize) = input.dimensions();
		if (outputxsize != -1) xsize = outputxsize;
		if (outputysize != -1) ysize = outputysize;
		std::unique_ptr<GDALDataset> out {
			driver->Create(filename.c_str(),
						   xsize, ysize,
						   1, GDT_Float32, nullptr)};

		double geoCoords[6];  //Geographic metadata
		if (input.input_dataset->GetGeoTransform(geoCoords) == CE_None)
			out->SetGeoTransform(geoCoords);

		const char * sref = input.input_dataset->GetProjectionRef();
		if (sref != NULL) out->SetProjection(sref);

		GDALRasterBand * out_band = out->GetRasterBand(1);
		out_band->SetNoDataValue(input.nodata_value());

		raster_output r = {std::move(out), out_band};
		return std::move(r);
	}

	std::pair<int, int> dimensions() {
		return {output_dataset->GetRasterXSize(),
				output_dataset->GetRasterYSize()};
	}

	void init_band() {
		if (output_band == nullptr)
			output_band = output_dataset->GetRasterBand(1);
	}

	GDALRasterBand * get_output_band() {
		init_band();
		return output_band;
	}

	float nodata_value() {
		float nodata;
		int has_nodata = false;
		nodata = get_output_band()->GetNoDataValue(&has_nodata);
		if (!has_nodata) nodata = -9999;
		return nodata;
	}

	uint64_t cell_count() {
		uint64_t xsize, ysize;
		std::tie(xsize, ysize) = dimensions();
		return xsize * ysize;
	}
};

#endif // COMMON_COMMON_H
