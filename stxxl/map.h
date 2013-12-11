// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :

#ifndef MAP_H
#define MAP_H

template <typename InputStream, typename F>
class point_mapper {
public:
	typedef map_point value_type;

	point_mapper(InputStream & inputStream, F f, int xsize, int ysize)
		: m_inputStream(inputStream)
		, f(std::move(f))
		, xsize(xsize)
		, ysize(ysize)
	{
		peek();
	}

	const value_type & operator*() const { return m_currentValue; }

	point_mapper & operator++() {
		++m_inputStream;
		peek();
		return *this;
	}

	bool empty() const { return m_inputStream.empty(); }

private:
	void peek() {
		while (!empty()) {
			point2 p = f(*m_inputStream);
			if (p.x >= 0 && p.x < xsize && p.y >= 0 && p.y < ysize) {
				m_currentValue = map_point{p, *m_inputStream};
				break;
			}
			++m_inputStream;
		}
	}

	InputStream & m_inputStream;
	value_type m_currentValue;
	F f;
	const int xsize;
	const int ysize;
};

#endif // MAP_H
