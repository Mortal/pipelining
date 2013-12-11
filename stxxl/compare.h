#ifndef COMPARE_H
#define COMPARE_H

class map_point_value_less {
public:
	bool operator()(const map_point & a, const map_point & b) const {
		return std::tie(a.to.y, a.to.x) < std::tie(b.to.y, b.to.x);
	}

	map_point max_value() const {
		const int maxint = std::numeric_limits<int>::max();
		const point2 p2{maxint, maxint};
		return map_point{p2, p2};
	}

	map_point min_value() const {
		const int minint = std::numeric_limits<int>::min();
		const point2 p2{minint, minint};
		return map_point{p2, p2};
	}
};

#endif // COMPARE_H
