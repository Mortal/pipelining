// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :

#ifndef FACTORY_HELPERS_H
#define FACTORY_HELPERS_H

#include <tpie/pipelining/factory_base.h>
#include <tuple>

namespace tpie {
namespace pipelining {

///////////////////////////////////////////////////////////////////////////////
/// \class factory
/// Node factory for variadic argument generators.
///////////////////////////////////////////////////////////////////////////////
template <template <typename dest_t> class R, typename... T>
class factory : public factory_base {
public:
	factory(T... v) : v(v...) {}

	template<typename dest_t>
	struct constructed {
		typedef R<dest_t> type;
	};

	template <typename dest_t>
	R<dest_t> construct(dest_t && dest) const {
		return invoker<sizeof...(T)>::go(std::move(dest), *this);
	}

private:
	std::tuple<T...> v;

	template <size_t N, size_t... S>
	class invoker {
	public:
		template <typename dest_t>
		static R<dest_t> go(dest_t && dest, const factory & parent) {
			return invoker<N-1, N-1, S...>::go(std::move(dest), parent);
		}
	};

	template <size_t... S>
	class invoker<0, S...> {
	public:
		template <typename dest_t>
		static R<dest_t> go(dest_t && dest, const factory & parent) {
			R<dest_t> r(std::move(dest), std::get<S>(parent.v)...);
			parent.init_node(r);
			parent.add_default_edge(r, dest);
			return r;
		}
	};

	template <size_t N, size_t... S>
	friend class invoker;
};

///////////////////////////////////////////////////////////////////////////////
/// \class tempfactory
/// Node factory for variadic argument templated generators.
///////////////////////////////////////////////////////////////////////////////
template <typename Holder, typename... T>
class tempfactory : public factory_base {
public:
	tempfactory(T... v) : v(v...) {}

	template<typename dest_t>
	struct constructed {
		typedef typename Holder::template type<dest_t> type;
	};

	template <typename dest_t>
	typename Holder::template type<dest_t> construct(dest_t && dest) const {
		return invoker<sizeof...(T)>::go(std::move(dest), *this);
	}

private:
	std::tuple<T...> v;

	template <size_t N, size_t... S>
	class invoker {
	public:
		template <typename dest_t>
		static typename Holder::template type<dest_t> go(dest_t && dest, const tempfactory & parent) {
			return invoker<N-1, N-1, S...>::go(std::move(dest), parent);
		}
	};

	template <size_t... S>
	class invoker<0, S...> {
	public:
		template <typename dest_t>
		static typename Holder::template type<dest_t> go(dest_t && dest, const tempfactory & parent) {
			typename Holder::template type<dest_t> r(std::move(dest), std::get<S>(parent.v)...);
			parent.init_node(r);
			parent.add_default_edge(r, dest);
			return r;
		}
	};

	template <size_t N, size_t... S>
	friend class invoker;
};

///////////////////////////////////////////////////////////////////////////////
/// \class termfactory
/// Node factory for variadic argument terminators.
///////////////////////////////////////////////////////////////////////////////
template <typename R, typename... T>
class termfactory : public factory_base {
public:
	typedef R constructed_type;

	termfactory(T... v) : v(v...) {}

	R construct() const {
		return invoker<sizeof...(T)>::go(*this);
	}

private:
	std::tuple<T...> v;

	template <size_t N, size_t... S>
	class invoker {
	public:
		static R go(const termfactory & parent) {
			return invoker<N-1, N-1, S...>::go(parent);
		}
	};

	template <size_t... S>
	class invoker<0, S...> {
	public:
		static R go(const termfactory & parent) {
			R r(std::get<S>(parent.v)...);
			parent.init_node(r);
			return r;
		}
	};

	template <size_t N, size_t... S>
	friend class invoker;
};

}
}

#endif // __TPIE_PIPELINING_FACTORY_HELPERS_H__
