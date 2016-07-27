#include <iostream>
#include <vector>
#include <algorithm>

namespace tpie {
	template <typename T>
	class file_stream {
	public:
		file_stream() {i = 0;}
		void write(T x) {
			if (i == contents.size()) contents.push_back(std::move(x));
			else contents[i] = std::move(x);
			++i;
		}
		void open() {}
		void close() {}
		void seek(size_t p) {i = p;}
		bool can_read() { return i < contents.size(); }
		T read() { return contents[i++]; }
		T peek() { return contents[i]; }

		std::vector<T> contents;
		size_t i;
	};

	template <typename T>
	class array {
	public:
		array(size_t c) { contents.resize(c); }
		T & operator[](size_t i) {return contents[i];}

		std::vector<T> contents;
	};

	template <typename T>
	void sort(file_stream<T> & x) {
		std::sort(x.contents.begin(), x.contents.end());
	}
}

struct raster_input {
	raster_input() { i = 0; }
	void read_next_row(tpie::array<float> * row) {
		for (float & x : row->contents) {
			x = 0.01 + i++ / 50.0;
		}
	}
	size_t i;
};

struct raster_output {
	void write_next_row(tpie::array<float> * row) {
		for (float x : row->contents) {
			std::cout << x << ' ';
		}
		std::cout << '\n';
	}
};

struct vec2 { int x, y; };
vec2 f(vec2 a) { return {a.y, a.x}; }  // Here, f is matrix transposition
bool operator<(vec2 a, vec2 b) { return (a.y != b.y) ? (a.y < b.y) : (a.x < b.x); }
struct map_point { vec2 from, to; };
bool operator<(map_point a, map_point b) { return a.from < b.from; }
struct value_point { vec2 point; float value; };
bool operator<(value_point a, value_point b) { return a.point < b.point; }
int main() {raster_input input; raster_output output; int outputxsize=10, outputysize=10, xsize=10, ysize=10; float nodata=-1;

// Sort the input points into the order in which they appear in the output.
tpie::file_stream<map_point> stream1; stream1.open();
for (int y = 0; y < outputysize; ++y)
    for (int x = 0; x < outputxsize; ++x) {
        map_point p = { f({x, y}), {x, y} };
        if (0 <= p.from.x && p.from.x < xsize && 0 <= p.from.y && p.from.y < ysize)
            stream1.write(p);
    }
// Sort the input points in row-major order, so we can scan them simultaneously with A.
tpie::sort(stream1);
// Scan input raster and input point stream, filling the input points with values.
stream1.seek(0);  // Seek to beginning of stream
tpie::file_stream<value_point> stream2; stream2.open();
tpie::array<float> row1(xsize);
for (int y = 0; y < ysize; ++y) {
    input.read_next_row(&row1);
    while (stream1.can_read() && stream1.peek().from.y == y) {
        map_point p = stream1.read();
        stream2.write(value_point{ p.to, row1[p.from.x] });
    }
}
stream1.close();
// Sort the filled input points into output order.
tpie::sort(stream2);
// Write the output points to a raster.
stream2.seek(0);  // Seek to beginning of stream
tpie::array<float> row2(outputxsize);
for (int y = 0; y < outputysize; ++y) {
    for (int x = 0; x < outputxsize; ++x) row2[x] = nodata;
    while(stream2.can_read() && stream2.peek().point.y == y) {
        value_point p = stream2.read();
        row2[p.point.x] = p.value;
    }
    output.write_next_row(&row2);
}
stream2.close();

	return 0;
}
