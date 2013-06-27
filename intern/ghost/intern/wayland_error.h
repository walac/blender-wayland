#ifndef WAYLAND_ERROR_H_
#define WAYLAND_ERROR_H_

#include <cstring>
#include <EGL/egl.h>

// EGL

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

// Wayland

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

#ifdef GHOST_DEBUG
#define EGL_CHK(expr) egl_error_check(expr, #expr, __FILE__, __LINE__)
#define WL_CHK(expr) wl_error_check(expr, #expr, __FILE__, __LINE__)
#else
#define EGL_CHK(expr) expr
#define WL_CHK(expr) expr
#endif /* GHOST_DEBUG */

#endif /* WAYLAND_ERROR_H_ */
