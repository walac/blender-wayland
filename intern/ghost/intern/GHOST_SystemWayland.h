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

/** \file ghost/intern/GHOST_SystemWayland.h
 *  \ingroup GHOST
 * Declaration of GHOST_SystemWayland class.
 */

#ifndef __GHOST_SYSTEMWAYLAND_H__
#define __GHOST_SYSTEMWAYLAND_H__

#include "GHOST_System.h"
#include "../GHOST_Types.h"
#include "GHOST_DisplayManagerWayland.h"
#include "GHOST_TimerManager.h"
#include "GHOST_WindowWayland.h"
#include "GHOST_Event.h"
#include "scoped_resource.h"
#include "wayland_util.h"
#include "wayland_listeners.h"

extern "C" {
#include <wayland-client.h>
#include <EGL/egl.h>
}

#include <cstring>
#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>

#define ADD_LISTENER(object) \
	wl::add_listener<wl::object##_listener>(this, m_##object.get())

class GHOST_WindowWayland;

class GHOST_SystemWayland
	: public GHOST_System
	, public wl::registry_listener
	, public wl::output_listener
{
public:

	GHOST_SystemWayland();
	~GHOST_SystemWayland();

	bool
	processEvents(bool waitForEvent);

	int
	toggleConsole(int action) { return 0; }

	GHOST_TSuccess
	getModifierKeys(GHOST_ModifierKeys& keys) const;

	GHOST_TSuccess
	getButtons(GHOST_Buttons& buttons) const;

	GHOST_TUns8 *
	getClipboard(bool selection) const;

	void
	putClipboard(GHOST_TInt8 *buffer, bool selection) const;

	GHOST_TUns8
	getNumDisplays() const;

	GHOST_TSuccess
	getCursorPosition(GHOST_TInt32& x,
	                  GHOST_TInt32& y) const;

	GHOST_TSuccess
	setCursorPosition(GHOST_TInt32 x,
	                  GHOST_TInt32 y);

	void
	getAllDisplayDimensions(GHOST_TUns32& width,
	                        GHOST_TUns32& height) const;

	void
	getMainDisplayDimensions(GHOST_TUns32& width,
	                         GHOST_TUns32& height) const;

	wl_display *getDisplay()
	{ return m_display.get(); }

	wl_compositor *getCompositor()
	{ return m_compositor.get(); }

	wl_shell *getShell()
	{ return m_shell.get(); }

	EGLDisplay getEglDisplay()
	{ return m_egl_display.get(); }

	EGLConfig getEglConf() const
	{ return m_conf; }

private:

	virtual void global(
			struct wl_registry *registry,
			uint32_t id,
			const char *interface,
			uint32_t version);

	virtual void mode(
		struct wl_output *output,
		uint32_t flags,
		int32_t width,
		int32_t height,
		int32_t refresh);

private:

	GHOST_TSuccess
	init();

	GHOST_IWindow *
	createWindow(const STR_String& title,
	             GHOST_TInt32 left,
	             GHOST_TInt32 top,
	             GHOST_TUns32 width,
	             GHOST_TUns32 height,
	             GHOST_TWindowState state,
	             GHOST_TDrawingContextType type,
	             const bool stereoVisual,
	             const bool exclusive = false,
	             const GHOST_TUns16 numOfAASamples = 0,
	             const GHOST_TEmbedderWindowID parentWindow = 0
	             );

	template<typename T>
	bool registry_bind(
		T &object,
		const char *name,
		const char *myname,
		uint32_t id,
		const wl_interface *interface);

	void dispatch_events(const boost::system::error_code &ec, bool &any_processed);
	void dispatch_timeout(const boost::system::error_code &ec);

private:
	wl::unique_ptr<wl_display> m_display;
	wl::unique_ptr<wl_registry> m_registry;
	wl::unique_ptr<wl_compositor> m_compositor;
	wl::unique_ptr<wl_shell> m_shell;
	wl::unique_ptr<wl_output> m_output;
	scoped_resource<EGLDisplay> m_egl_display;
	EGLConfig m_conf;
	boost::asio::io_service m_io_service;
	boost::asio::posix::stream_descriptor m_display_fd;
	boost::asio::deadline_timer m_dispatch_timer;
	uint32_t m_width;
	uint32_t m_height;
};

template<typename T> bool
GHOST_SystemWayland::registry_bind(
	T &object,
	const char *name,
	const char *myname,
	uint32_t id,
	const wl_interface *interface)
{
	typedef typename T::pointer pointer;

	if (!std::strcmp(name, myname)) {
		object.reset(
			static_cast<pointer> (WL_CHK(wl_registry_bind(
				m_registry.get(),
				id,
				interface,
				1))));
		return true;
	}

	return false;
}


#endif
