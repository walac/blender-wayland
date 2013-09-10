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

#include <cstring>
#include <EGL/egl.h>
#include <cassert>
#include <wayland-client.h>

#ifdef GHOST_DEBUG
#define EGL_CHK(expr) egl::error_check(expr, #expr, __FILE__, __LINE__)
#define WL_CHK(expr) wl::error_check(expr, #expr, __FILE__, __LINE__)
#else
#define EGL_CHK(expr) expr
#define WL_CHK(expr) expr
#endif /* GHOST_DEBUG */

namespace wl {

// Wayland error treatment

void
report_error(const char *expr, const char *srcfile, size_t linenumber);

int
error_check(int result, const char *expr, const char *srcfile, size_t linenumber);

template<typename T> T *
error_check(T *result, const char *expr, const char *srcfile, size_t linenumber)
{
	if (!result)
		report_error(expr, srcfile, linenumber);

	return result;
}

// Wayland utilities

template<typename B, typename D, typename Proxy> void
add_listener(D *pthis, Proxy *object)
{
	wl_proxy *proxy = reinterpret_cast<wl_proxy *> (object);
	B *b = dynamic_cast<B*> (pthis);
	assert(b);
	wl_proxy_add_listener(proxy,
		*reinterpret_cast<void(***)(void)> (b), b);
}

template<typename T, typename U> inline void
set_user_data(T *object, U *data)
{
	wl_proxy_set_user_data(
		reinterpret_cast<wl_proxy *> (object),
		static_cast<void *> (data));
}

template<typename T> inline void
destroy(T *object)
{
        if (object)
                wl_proxy_destroy(reinterpret_cast<wl_proxy*> (object));
}

inline void
destroy(wl_display *display)
{
        if (display)
                wl_display_disconnect(display);
}

}

namespace egl {

// EGL error treatment

void
report_error(const char *expr, const char *srcfile, size_t linenumber);

EGLBoolean
error_check(EGLBoolean result, const char *expr, const char *srcfile, size_t linenumber);

template<typename T> T *
error_check(T *result, const char *expr, const char *srcfile, size_t linenumber)
{
	if (!result)
		report_error(expr, srcfile, linenumber);

	return result;
}

}

#endif /* WAYLAND_UTIL_H_ */
