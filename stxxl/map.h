// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :

#ifndef MAP_H
#define MAP_H

template <typename InputStream, typename F>
class point_mapper {
public:
	typedef map_point value_type;

private:
	InputStream & m_inputStream;
	value_type m_currentValue;
	F f;

public:
	point_mapper(InputStream & inputStream, F f)
		: m_inputStream(inputStream)
		, f(std::move(f))
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
		if (!empty()) m_currentValue = map_point{*m_inputStream, f(*m_inputStream)};
	}
};

#endif // MAP_H
