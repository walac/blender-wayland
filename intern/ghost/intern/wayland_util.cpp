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

#include "wayland_util.h"
#include <libgen.h>
#include <string>
#include <vector>
#include <cstdio>

namespace {
	std::string path_basename(const char *path)
	{
		const size_t len = strlen(path) + 1;
		std::vector<char> v(path, path + len);
		return basename(&v[0]);
	}

	std::string function_name(const char *expr)
	{
		const char *p = strchr(expr, '(');
		return std::string(expr, p + (p ? 1 : strlen(expr)));
	}

	const char *egl_str_error(EGLint error)
	{
		switch (error) {
#define EGL_ERROR_2_STR(e) case e: return #e
			EGL_ERROR_2_STR(EGL_SUCCESS);
			EGL_ERROR_2_STR(EGL_NOT_INITIALIZED);
			EGL_ERROR_2_STR(EGL_BAD_ACCESS);
			EGL_ERROR_2_STR(EGL_BAD_ALLOC);
			EGL_ERROR_2_STR(EGL_BAD_ATTRIBUTE);
			EGL_ERROR_2_STR(EGL_BAD_CONTEXT);
			EGL_ERROR_2_STR(EGL_BAD_CONFIG);
			EGL_ERROR_2_STR(EGL_BAD_CURRENT_SURFACE);
			EGL_ERROR_2_STR(EGL_BAD_DISPLAY);
			EGL_ERROR_2_STR(EGL_BAD_SURFACE);
			EGL_ERROR_2_STR(EGL_BAD_MATCH);
			EGL_ERROR_2_STR(EGL_BAD_PARAMETER);
			EGL_ERROR_2_STR(EGL_BAD_NATIVE_PIXMAP);
			EGL_ERROR_2_STR(EGL_BAD_NATIVE_WINDOW);
			EGL_ERROR_2_STR(EGL_CONTEXT_LOST);
#undef EGL_ERROR_2_STR

			default:
			return "EGL_UNKNOWN_ERROR";
		}
	}

}

void
egl_report_error(const char *expr, const char *srcfile, size_t linenumber)
{
	std::fprintf(
			stderr,
			"%s (%s:%zu): %s\n",
			function_name(expr).c_str(),
			path_basename(srcfile).c_str(),
			linenumber,
			egl_str_error(eglGetError()));
}

EGLBoolean
egl_error_check(EGLBoolean result, const char *expr, const char *srcfile, size_t linenumber)
{
	if (EGL_FALSE == result)
		egl_report_error(expr, srcfile, linenumber);

	return result;
}

void
wl_report_error(const char *expr, const char *srcfile, size_t linenumber)
{
	std::fprintf(
			stderr,
			"%s (%s:%zu): FAILED\n",
			function_name(expr).c_str(),
			path_basename(srcfile).c_str(),
			linenumber);
}

int
wl_error_check(int result, const char *expr, const char *srcfile, size_t linenumber)
{
	if (result < 0)
		wl_report_error(expr, srcfile, linenumber);

	return result;
}

