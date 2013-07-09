/*
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Contributor(s): Wander Lairson Costa
 *
 * ***** END GPL LICENSE BLOCK *****
 */

#ifndef SCOPED_RESOURCE_H_
#define SCOPED_RESOURCE_H_

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>
#include <boost/none.hpp>

// Inspired by http://www.andrewlsandoval.com/scope_exit/
template<typename T>
class scoped_resource : private boost::noncopyable {
public:
	typedef T value_type;
	typedef T element_type;
	typedef value_type &reference;
	typedef const value_type &const_reference;
	typedef boost::function<void(value_type)> deleter_type;
	typedef scoped_resource<value_type> this_type;

	scoped_resource() {}

	template<typename D>
	explicit scoped_resource(D d) : d_(d) {}

	template<typename D>
	scoped_resource(const_reference r, D d) : r_(r), d_(d) {}

	~scoped_resource()
	{ delete_resource(); }

	const_reference get() const
	{ return r_.get(); }

	reference get()
	{ return r_.get(); }

	// non-throw if resource has non-throw swap semantics
	void swap(this_type &other)
	{
		std::swap(r_, other.r_);
		std::swap(d_, other.d_);
	}

	void reset()
	{ delete_resource(); }

	void reset(const_reference r)
	{ this_type(r, d_).swap(*this); }

	template<typename D>
	void reset(const_reference r, D d)
	{ this_type(r, d).swap(*this); }

	typedef void (this_type::*unspecified_bool)();

	operator unspecified_bool() const
	{ return r_ ? &this_type::reset : 0; }

private:
	void delete_resource()
	{
		if (r_) {
			d_(r_.get());
			r_ = boost::none;
		}
	}

	boost::optional<value_type> r_;
	deleter_type d_;
};

namespace std {
	template<typename T>
	void swap(scoped_resource<T> &r1, scoped_resource<T> &r2)
	{ r1.swap(r2); }
}

#endif // SCOPED_RESOURCE_H_
