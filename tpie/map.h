// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
#ifndef MAP_H
#define MAP_H

template <typename F>
class Map {
public:
template <typename dest_t>
class type {
public:
	typedef typename F::argument_type item_type;

	type(dest_t && dest, F f)
		: f(std::move(f))
		, dest(std::move(dest))
	{
		add_push_destination(dest);
		set_name("Map");
	}

	void push(const item_type & x) {
		dest.push(f(x));
	}

private:
	F f;
	dest_t dest;
};
};

template <typename F>
tp::pipe_middle<tp::temp_factory<Map<F>, F> >
map(const F & f) {
	return tp::temp_factory<Map<F>, F>(f);
}

#endif // MAP_H
