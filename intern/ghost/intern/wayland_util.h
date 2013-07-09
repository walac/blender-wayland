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

#ifndef WAYLAND_UTIL_H_
#define WAYLAND_UTIL_H_

#include <boost/interprocess/smart_ptr/unique_ptr.hpp>
#include <cstring>
#include <functional>
#include <boost/function.hpp>
#include <EGL/egl.h>

// Wayland error treatment

void
wl_report_error(const char *expr, const char *srcfile, size_t linenumber);

int
wl_error_check(int result, const char *expr, const char *srcfile, size_t linenumber);

template<typename T> T *
wl_error_check(T *result, const char *expr, const char *srcfile, size_t linenumber)
{
	if (!result)
		wl_report_error(expr, srcfile, linenumber);

	return result;
}

// EGL error treatment

void
egl_report_error(const char *expr, const char *srcfile, size_t linenumber);

EGLBoolean
egl_error_check(EGLBoolean result, const char *expr, const char *srcfile, size_t linenumber);

template<typename T> T *
egl_error_check(T *result, const char *expr, const char *srcfile, size_t linenumber)
{
	if (!result)
		egl_report_error(expr, srcfile, linenumber);

	return result;
}

#ifdef GHOST_DEBUG
#define EGL_CHK(expr) egl_error_check(expr, #expr, __FILE__, __LINE__)
#define WL_CHK(expr) wl_error_check(expr, #expr, __FILE__, __LINE__)
#else
#define EGL_CHK(expr) expr
#define WL_CHK(expr) expr
#endif /* GHOST_DEBUG */

template<typename T>
struct wayland_ptr {
	typedef boost::interprocess::unique_ptr<T, void(*)(T*)> type;
};

template<typename T>
struct egl_object_deleter
	: public std::unary_function<void, T>
{
	template<typename D>
	egl_object_deleter(EGLDisplay disp, D d)
		: disp(disp), deleter(d)
	{ }

	void operator()(T o)
	{ EGL_CHK(deleter(disp, o)); }

	EGLDisplay disp;
	boost::function<EGLBoolean(EGLDisplay, T)> deleter;
};

#endif /* WAYLAND_UTIL_H_ */
