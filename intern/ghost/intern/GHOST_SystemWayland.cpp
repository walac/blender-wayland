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
#include <boost/bind.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>

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
	, m_egl_display(eglTerminate)
	, m_display_fd(m_io_service)
	, m_dispatch_timer(m_io_service)
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

	m_registry.reset(WL_CHK(wl_display_get_registry(m_display.get())));

	ADD_LISTENER(registry);

	WL_CHK(wl_display_dispatch(m_display.get()));

	m_egl_display.reset(EGL_CHK(eglGetDisplay(m_display.get())));

	EGL_CHK(eglInitialize(m_egl_display.get(), &major, &minor));

	EGL_CHK(eglBindAPI(EGL_OPENGL_API));

	EGLint n;
	EGL_CHK(eglChooseConfig(m_egl_display.get(), config_attribs, &m_conf, 1, &n));

	m_display_fd.assign(wl_display_get_fd(m_display.get()));
}

GHOST_SystemWayland::~GHOST_SystemWayland()
{
	m_display_fd.release();
	WL_CHK(wl_display_flush(m_display.get()));
}

GHOST_IWindow *
GHOST_SystemWayland::createWindow(const STR_String& title,
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
	std::auto_ptr<GHOST_WindowWayland> window(new (std::nothrow) GHOST_WindowWayland(this, title,
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
	} else {
		return NULL;
	}

	return window.release();
}

GHOST_TSuccess
GHOST_SystemWayland::init() {
	GHOST_TSuccess success = GHOST_System::init();

	if (success) {
		m_displayManager = new (std::nothrow) GHOST_DisplayManagerWayland(this);

		if (m_displayManager) {
			return GHOST_kSuccess;
		}
	}

	return GHOST_kFailure;
}

void
GHOST_SystemWayland::dispatch_events(const boost::system::error_code &ec, bool &any_processed)
{
	m_dispatch_timer.cancel();

	if (ec) {
#ifdef GHOST_DEBUG
		fprintf(stderr, "GHOST_SystemWayland::dispatch_events: %s\n",
			ec.message().c_str());
#endif // GHOST_DEBUG
		return;
	}

	if (WL_CHK(wl_display_dispatch(m_display.get())) > 0)
		any_processed = true;
}

void
GHOST_SystemWayland::dispatch_timeout(const boost::system::error_code &ec)
{
	m_display_fd.cancel();

#ifdef GHOST_DEBUG
	if (ec)
		fprintf(stderr, "GHOST_SystemWayland::dispatch_timeout: %s\n",
			ec.message().c_str());
#else
	(void) ec;
#endif // GHOST_DEBUG
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
	// Get all the current events -- translate them into
	// ghost events and call base class pushEvent() method.

	using boost::posix_time::milliseconds;
	using namespace boost::asio;

	bool anyProcessed = false;
	GHOST_TimerManager *timerMgr = getTimerManager();

	do {
		if (WL_CHK(wl_display_dispatch_pending(m_display.get())) <= 0 && waitForEvent) {
			const GHOST_TUns64 next = timerMgr->nextFireTime();
			const GHOST_TUns64 cur_milliseconds = getMilliSeconds();

			if (GHOST_kFireTimeNever == next) {
				if (WL_CHK(wl_display_dispatch(m_display.get())) > 0)
					anyProcessed = true;
			} else if (cur_milliseconds < next) {
				const GHOST_TUns64 wait_time = next - cur_milliseconds;

				// with a null_buffer our callback will be called when
				// there bytes available but no reading will happen
				m_display_fd.async_read_some(
					null_buffers(),
					boost::bind(
						&GHOST_SystemWayland::dispatch_events,
						this,
						placeholders::error(),
						boost::ref(anyProcessed)));

				// timeout
				m_dispatch_timer.expires_from_now(milliseconds(wait_time));
				m_dispatch_timer.async_wait(
					boost::bind(
						&GHOST_SystemWayland::dispatch_timeout,
						this,
						placeholders::error()));

				m_io_service.run_one();
			}
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

