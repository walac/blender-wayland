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

/** \file ghost/intern/GHOST_SystemWayland.cpp
 *  \ingroup GHOST
 */

#include <cassert>
#include <memory>
#include <poll.h>

#include "GHOST_SystemWayland.h"
#include "GHOST_WindowWayland.h"

#include "GHOST_WindowManager.h"

#include "GHOST_EventCursor.h"
#include "GHOST_EventKey.h"
#include "GHOST_EventButton.h"
#include "GHOST_EventWheel.h"

GHOST_SystemWayland::GHOST_SystemWayland()
	: GHOST_System()
	, m_display(wl_display_connect(NULL))
	, m_registry(NULL)
	, m_compositor(NULL)
	, m_shell(NULL)
	, m_output(NULL)
	, m_seat(NULL)
	, m_keyboard(NULL)
	, m_pointer(NULL)
{
	static const EGLint config_attribs[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RED_SIZE, 1,
		EGL_GREEN_SIZE, 1,
		EGL_BLUE_SIZE, 1,
		EGL_ALPHA_SIZE, 1,
		EGL_DEPTH_SIZE, 1,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
		EGL_NONE
	};

	EGLint major, minor;

	m_registry = WL_CHK(wl_display_get_registry(m_display));

	ADD_LISTENER(registry);

	WL_CHK(wl_display_dispatch(m_display));

	m_egl_display = EGL_CHK(eglGetDisplay(m_display));

	EGL_CHK(eglInitialize(m_egl_display, &major, &minor));

	EGL_CHK(eglBindAPI(EGL_OPENGL_API));

	EGLint n;
	EGLBoolean ret = EGL_CHK(eglChooseConfig(m_egl_display, config_attribs, &m_conf, 1, &n));
	assert(EGL_TRUE == ret && 1 == n);
}

GHOST_SystemWayland::~GHOST_SystemWayland()
{
	if (EGL_NO_DISPLAY != m_egl_display)
		EGL_CHK(eglTerminate(m_egl_display));

	wl::destroy(m_keyboard);
	wl::destroy(m_pointer);
	wl::destroy(m_output);
	wl::destroy(m_shell);
	wl::destroy(m_compositor);
	wl::destroy(m_seat);
	wl::destroy(m_registry);

	WL_CHK(wl_display_flush(m_display));
	wl::destroy(m_display);
}

GHOST_IWindow *
GHOST_SystemWayland::createWindow(
        const STR_String& title,
        GHOST_TInt32 left,
        GHOST_TInt32 top,
        GHOST_TUns32 width,
        GHOST_TUns32 height,
        GHOST_TWindowState state,
        GHOST_TDrawingContextType type,
        const bool stereoVisual,
        const bool exclusive,
        const GHOST_TUns16 numOfAASamples,
        const GHOST_TEmbedderWindowID parentWindow
        )
{
	std::auto_ptr<GHOST_WindowWayland> window(new (std::nothrow) GHOST_WindowWayland(
	        this, title,
	        left, top, width, height,
	        state, parentWindow, type,
	        stereoVisual, exclusive,
	        numOfAASamples));

	if (!window.get())
		return NULL;

	if (GHOST_kWindowStateFullScreen == state) {
		// Full screen window
	}

	if (window->getValid()) {
		m_windowManager->addWindow(window.get());
		pushEvent(new GHOST_Event(getMilliSeconds(), GHOST_kEventWindowSize, window.get()));
	}
	else {
		return NULL;
	}

	return window.release();
}

GHOST_TSuccess
GHOST_SystemWayland::init()
{
	GHOST_TSuccess success = GHOST_System::init();

	if (success) {
		m_displayManager = new (std::nothrow) GHOST_DisplayManagerWayland(this);

		if (m_displayManager) {
			return GHOST_kSuccess;
		}
	}

	return GHOST_kFailure;
}

/**
 * Returns the dimensions of the main display on this system.
 * \return The dimension of the main display.
 */
void
GHOST_SystemWayland::getAllDisplayDimensions(GHOST_TUns32& width,
                                             GHOST_TUns32& height) const
{
	width = m_width;
	height = m_height;
}

void
GHOST_SystemWayland::getMainDisplayDimensions(GHOST_TUns32& width,
                                              GHOST_TUns32& height) const
{
	width = m_width;
	height = m_height;
}

GHOST_TUns8
GHOST_SystemWayland::getNumDisplays() const
{
	return 1;
}

GHOST_TSuccess
GHOST_SystemWayland::getModifierKeys(GHOST_ModifierKeys& keys) const
{
	(void) keys;
	return GHOST_kSuccess;
}

GHOST_TSuccess
GHOST_SystemWayland::getCursorPosition(GHOST_TInt32& x,
                                       GHOST_TInt32& y) const
{
	x = 0;
	y = 0;
	return GHOST_kSuccess;
}

GHOST_TSuccess
GHOST_SystemWayland::setCursorPosition(GHOST_TInt32 x,
                                       GHOST_TInt32 y)
{
	(void) x;
	(void) y;
	return GHOST_kSuccess;
}

bool
GHOST_SystemWayland::processEvents(bool waitForEvent)
{
	bool anyProcessed = false;
	GHOST_TimerManager *timerMgr = getTimerManager();
	struct pollfd fds;

	fds.fd = wl_display_get_fd(m_display);
	fds.events = POLLIN;

	do {
		if (WL_CHK(wl_display_dispatch_pending(m_display)) <= 0) {
			WL_CHK(wl_display_flush(m_display));

			const GHOST_TUns64 next = timerMgr->nextFireTime();
			const GHOST_TUns64 cur_milliseconds = getMilliSeconds();

			if (GHOST_kFireTimeNever == next && waitForEvent) {
				if (WL_CHK(wl_display_dispatch(m_display)) > 0)
					anyProcessed = true;
			}
			else if (cur_milliseconds <= next) {
				const GHOST_TUns64 wait_time = waitForEvent ? next - cur_milliseconds : 0;

				fds.revents = 0;
				int ret = poll(&fds, 1, wait_time);

				if (ret < 0) {
					perror("poll");
					break;
				}
				else if (ret) {
					switch (fds.revents) {
						case POLLIN:
							if (WL_CHK(wl_display_dispatch(m_display)) > 0)
								anyProcessed = true;
							break;

						case POLLOUT:
							WL_CHK(wl_display_flush(m_display));
							break;

						default:
							fprintf(stderr, "epoll error: %hd\n", fds.revents);
					}
				}
			}
		}
		else {
			anyProcessed = true;
		}

		if (timerMgr->fireTimers(getMilliSeconds()))
			anyProcessed = true;
	} while (waitForEvent && !anyProcessed);

	return anyProcessed;
}

GHOST_TSuccess GHOST_SystemWayland::getButtons(GHOST_Buttons& buttons) const
{
	(void) buttons;
	return GHOST_kSuccess;
}

GHOST_TUns8 *
GHOST_SystemWayland::getClipboard(bool selection) const
{
	(void) selection;
	return NULL;
}

void
GHOST_SystemWayland::putClipboard(GHOST_TInt8 *buffer, bool selection) const
{
	(void) buffer;
	(void) selection;
}

void
GHOST_SystemWayland::global(
        struct wl_registry *registry,
        uint32_t id,
        const char *interface,
        uint32_t version)
{
	using std::strcmp;

	(void) version;

#define REGISTRY_BIND(object) \
	registry_bind( \
		m_##object, \
		interface, \
		"wl_" #object, \
		id, \
		&wl_##object##_interface)

	REGISTRY_BIND(compositor);
	REGISTRY_BIND(shell);

	if (REGISTRY_BIND(output))
		ADD_LISTENER(output);

	if (REGISTRY_BIND(seat))
		ADD_LISTENER(seat);

#undef REGISTRY_BIND
}

void
GHOST_SystemWayland::mode(
        struct wl_output *output,
        uint32_t flags,
        int32_t width,
        int32_t height,
        int32_t refresh)
{
	(void) output;
	(void) refresh;

	if (flags & WL_OUTPUT_MODE_CURRENT) {
		m_width = width;
		m_height = height;
	}
}

void
GHOST_SystemWayland::capabilities(
	struct wl_seat *seat,
	uint32_t capabilities)
{
#define REGISTRY_INPUT(device, cap) \
	do { \
		const int has_##device = capabilities & cap; \
		if (has_##device && !m_##device) { \
			m_##device = WL_CHK(wl_seat_get_##device(seat)); \
			ADD_LISTENER(device); \
		} \
		else if (!has_##device && m_##device) { \
			wl::destroy(m_##device); \
			m_##device = NULL; \
		} \
	} while (0)

	REGISTRY_INPUT(keyboard, WL_SEAT_CAPABILITY_KEYBOARD);
	REGISTRY_INPUT(pointer, WL_SEAT_CAPABILITY_POINTER);

#undef REGISTRY_INPUT
}
