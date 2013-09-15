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
 * Contributor(s): wander Lairson Costa
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file ghost/intern/GHOST_WindowWayland.cpp
 *  \ingroup GHOST
 */

#include "GHOST_WindowWayland.h"
#include <GL/glew.h>
#include <cassert>

WindowCallbackBase::WindowCallbackBase(GHOST_WindowWayland *window)
	: m_window(window)
	, m_callback(NULL)
{
}

WindowCallbackBase::~WindowCallbackBase()
{
	wl::destroy(m_callback);
}

GHOST_WindowWayland::GHOST_WindowWayland(GHOST_SystemWayland *system,
                                 const STR_String& title,
                                 GHOST_TInt32 left,
                                 GHOST_TInt32 top,
                                 GHOST_TUns32 width,
                                 GHOST_TUns32 height,
                                 GHOST_TWindowState state,
                                 const GHOST_TEmbedderWindowID parentWindow,
                                 GHOST_TDrawingContextType type,
                                 const bool stereoVisual,
                                 const bool exclusive,
                                 const GHOST_TUns16 numOfAASamples
                                 )
	: GHOST_Window(width, height, state, type, stereoVisual, exclusive, numOfAASamples)
	, m_system(system)
	, m_surface(NULL)
	, m_shell_surface(NULL)
	, m_window(NULL)
	, m_egl_surface(EGL_NO_SURFACE)
	, m_egl_context(EGL_NO_CONTEXT)
	, m_x(left)
	, m_y(top)
	, m_width(width)
	, m_height(height)
	, m_state(GHOST_kWindowStateNormal)
	, m_redraw(this)
	, m_sync(this)
{
	wl_display *display = m_system->getDisplay();
	wl_compositor *compositor = m_system->getCompositor();
	wl_shell *shell = m_system->getShell();
	EGLDisplay egl_display = m_system->getEglDisplay();

	assert(display);
	assert(compositor);
	assert(shell);

	m_surface = WL_CHK(wl_compositor_create_surface(compositor));

	m_shell_surface =
		WL_CHK(wl_shell_get_shell_surface(shell, m_surface));

	ADD_LISTENER(shell_surface);

	m_window = WL_CHK(wl_egl_window_create(m_surface, width, height));

	m_egl_surface =
		EGL_CHK(eglCreateWindowSurface(egl_display, m_system->getEglConf(),
			m_window, NULL));


	assert(m_egl_surface != EGL_NO_SURFACE);

	setTitle(title);

	/* now set up the rendering context. */
	if (installDrawingContext(type) == GHOST_kSuccess) {
		// m_valid_setup = true;
		GHOST_PRINT("Created window\n");
	}

	EGL_CHK(wl_shell_surface_set_toplevel(m_shell_surface));
	resize();
	m_sync.initialize();
}

GHOST_WindowWayland::~GHOST_WindowWayland()
{
	EGLDisplay display = m_system->getEglDisplay();

	if (m_egl_context != EGL_NO_CONTEXT) {
		context_make_current(EGL_NO_SURFACE);
		EGL_CHK(eglDestroyContext(display, m_egl_context));
	}

	if (m_egl_surface != EGL_NO_SURFACE)
		EGL_CHK(eglDestroySurface(display, m_egl_surface));

	if (m_window)
		wl_egl_window_destroy(m_window);

	wl::destroy(m_shell_surface);
	wl::destroy(m_surface);
}

GHOST_TSuccess
GHOST_WindowWayland::installDrawingContext(GHOST_TDrawingContextType type)
{
	// only support openGL for now.
	GHOST_TSuccess success;
	switch (type) {
		case GHOST_kDrawingContextTypeOpenGL: {
			EGLDisplay egl_display = m_system->getEglDisplay();

			if (m_egl_context != EGL_NO_CONTEXT) {
				context_make_current(EGL_NO_SURFACE);
				EGL_CHK(eglDestroyContext(egl_display, m_egl_context));
			}

			m_egl_context = EGL_CHK(eglCreateContext(
			        egl_display,
			        m_system->getEglConf(),
			        EGL_NO_CONTEXT,
			        NULL));

			if (EGL_NO_CONTEXT == m_egl_context)
				return GHOST_kFailure;

			context_make_current(m_egl_surface);

			success = GHOST_kSuccess;
			break;
		}

		case GHOST_kDrawingContextTypeNone:
			success = GHOST_kSuccess;
			break;

		default:
			success = GHOST_kFailure;
	}
	return success;
}


GHOST_TSuccess
GHOST_WindowWayland::invalidate(void)
{
	return GHOST_kSuccess;
}


GHOST_TSuccess
GHOST_WindowWayland::swapBuffers()
{
	if (getDrawingContextType() == GHOST_kDrawingContextTypeOpenGL) {
		EGL_CHK(eglSwapBuffers(
		            m_system->getEglDisplay(),
		            m_egl_surface));
		return GHOST_kSuccess;
	}
	else {
		return GHOST_kFailure;
	}
}


GHOST_TSuccess
GHOST_WindowWayland::activateDrawingContext()
{
	context_make_current(m_egl_surface);
	return GHOST_kSuccess;
}


GHOST_TSuccess
GHOST_WindowWayland::removeDrawingContext()
{
	context_make_current(EGL_NO_SURFACE);
	return GHOST_kSuccess;
}

GHOST_TSuccess
GHOST_WindowWayland::setState(GHOST_TWindowState state)
{
	if (m_state != state) {
		m_state = state;

		switch (state) {
			case GHOST_kWindowStateNormal:
				break;
			case GHOST_kWindowStateMaximized:
				break;
			case GHOST_kWindowStateMinimized:
				break;
			case GHOST_kWindowStateFullScreen:
				break;
			default:
				break;
		}
	}

	return GHOST_kSuccess;
}


GHOST_TWindowState
GHOST_WindowWayland::getState() const
{
	return m_state;
}


void
GHOST_WindowWayland::setTitle(const STR_String& title)
{
	m_title = title.ReadPtr();
	wl_shell_surface_set_title(m_shell_surface, m_title.c_str());
}


void
GHOST_WindowWayland::getTitle(STR_String& title) const
{
	title = m_title.c_str();
}


void
GHOST_WindowWayland::getWindowBounds(GHOST_Rect& bounds) const
{
	getClientBounds(bounds);
}


void
GHOST_WindowWayland::getClientBounds(GHOST_Rect& bounds) const
{
	bounds.m_l = m_x;
	bounds.m_r = m_x + m_width;
	bounds.m_t = m_y;
	bounds.m_b = m_y + m_height;
}

GHOST_TSuccess
GHOST_WindowWayland::setClientWidth(GHOST_TUns32 width)
{
	m_width = width;
	resize();
	return GHOST_kSuccess;
}

GHOST_TSuccess
GHOST_WindowWayland::setClientHeight(GHOST_TUns32 height)
{
	m_height = height;
	resize();
	return GHOST_kSuccess;
}

GHOST_TSuccess
GHOST_WindowWayland::setClientSize(GHOST_TUns32 width,
		GHOST_TUns32 height)
{
	m_width = width;
	m_height = height;
	resize();
	return GHOST_kSuccess;
}

void
GHOST_WindowWayland::screenToClient(GHOST_TInt32 inX, GHOST_TInt32 inY, GHOST_TInt32& outX, GHOST_TInt32& outY) const
{
	outX = inX - m_x;
	outY = inY - m_x;
}
void
GHOST_WindowWayland::clientToScreen(GHOST_TInt32 inX, GHOST_TInt32 inY, GHOST_TInt32& outX, GHOST_TInt32& outY) const
{
	outX = inX + m_x;
	outY = inY + m_x;
}

GHOST_TSuccess
GHOST_WindowWayland::setWindowCursorGrab(GHOST_TGrabCursorMode mode)
{
	(void) mode;
	return GHOST_kSuccess;
}


GHOST_TSuccess
GHOST_WindowWayland::setWindowCursorShape(GHOST_TStandardCursor shape)
{
	(void) shape;
	return GHOST_kSuccess;
}


GHOST_TSuccess
GHOST_WindowWayland::setWindowCustomCursorShape(GHOST_TUns8 bitmap[16][2],
                                            GHOST_TUns8 mask[16][2],
                                            int hotX,
                                            int hotY)
{
	return setWindowCustomCursorShape((GHOST_TUns8 *)bitmap,
	                                  (GHOST_TUns8 *)mask,
	                                  16, 16,
	                                  hotX, hotY,
	                                  0, 1);
}


GHOST_TSuccess
GHOST_WindowWayland::setWindowCustomCursorShape(GHOST_TUns8 *bitmap,
                                            GHOST_TUns8 *mask,
                                            int sizex, int sizey,
                                            int hotX, int hotY,
                                            int fg_color, int bg_color)
{
	(void) bitmap;
	(void) mask;
	(void) hotX;
	(void) hotY;
	(void) fg_color;
	(void) bg_color;
	return GHOST_kSuccess;
}


GHOST_TSuccess
GHOST_WindowWayland::setWindowCursorVisibility(bool visible)
{
	(void) visible;
	return GHOST_kSuccess;
}

void
GHOST_WindowWayland::context_make_current(EGLSurface surf)
{
	assert(m_egl_context != EGL_NO_CONTEXT);

	EGL_CHK(eglMakeCurrent(
	        m_system->getEglDisplay(),
	        surf,
	        surf,
	        m_egl_context));
}

void
GHOST_WindowWayland::resize(void)
{
	wl_egl_window_resize(
	        m_window,
	        m_width,
	        m_height,
	        m_x,
	        m_y);
}

void
GHOST_WindowWayland::ping(struct wl_shell_surface *shell_surface, uint32_t serial)
{
	wl_shell_surface_pong(shell_surface, serial);
}

void
GHOST_WindowWayland::configure(
        struct wl_shell_surface *shell_surface,
        uint32_t edges,
        int32_t width,
        int32_t height)
{
	if (m_window)
		wl_egl_window_resize(m_window, width, height, 0, 0);

	m_width = width;
	m_height = height;
}

// redraw
void
GHOST_WindowWayland::WindowDrawHandler::done(
	struct wl_callback *callback,
	uint32_t)
{
	assert(callback == m_callback);
	GHOST_SystemWayland *system = m_window->m_system;

	system->pushEvent(
	        new GHOST_Event(
	            system->getMilliSeconds(),
	            GHOST_kEventWindowUpdate,
	            m_window));

	wl::destroy(m_callback);
	initialize();
}

void
GHOST_WindowWayland::WindowDrawHandler::init()
{
	m_callback = WL_CHK(wl_surface_frame(m_window->m_surface));
	ADD_LISTENER(callback);
}

void
GHOST_WindowWayland::DisplaySyncHandler::done(
        struct wl_callback *callback,
        uint32_t)
{
	assert(callback == m_callback);
	GHOST_SystemWayland *system = m_window->m_system;

	system->pushEvent(
	        new GHOST_Event(
	            system->getMilliSeconds(),
	            GHOST_kEventWindowUpdate,
	            m_window));

	wl::destroy(m_callback);
	m_callback = NULL;
	m_window->m_redraw.initialize();
}

void
GHOST_WindowWayland::DisplaySyncHandler::init()
{
	m_callback = WL_CHK(wl_display_sync(m_window->m_system->getDisplay()));
	ADD_LISTENER(callback);
}

