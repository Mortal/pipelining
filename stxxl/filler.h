// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :

#ifndef FILLER_H
#define FILLER_H

template <typename OutputPointStream, typename InputRowStream>
class Filler {
public:
	typedef value_point value_type;

	Filler(OutputPointStream & pointSource, InputRowStream & src, float nodata)
		: pointSource(pointSource)
		, src(src)
		, y(0)
		, nodata(nodata)
	{
	}

	bool empty() const {
		return pointSource.empty();
	}

	Filler & operator++() {
		if (empty()) {std::cout << "filler: now done (1)" << std::endl; }
		if (empty()) return *this;
		map_point p;
		size_t skipped = 0;
		do {
			++skipped;
			++pointSource;
			if (empty()) break;
			p = *pointSource;
			while (p.from.y > y) {
				++y;
				++src;
			}
		} while ((*src)[p.from.x] == nodata);
		if (empty()) {std::cout << "filler: now done (2)" << std::endl; }
		return *this;
	}

	value_type operator*() {
		map_point p = *pointSource;
		const std::vector<float> & row = *src;
		return value_point{p.to, row[p.from.x]};
	}

private:
	OutputPointStream & pointSource;
	InputRowStream & src;
	int y;
	const float nodata;
};

#endif // FILLER_H
