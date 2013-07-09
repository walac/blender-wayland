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

extern "C" {
#include "wayland-client.h"
#include "EGL/egl.h"
}

#include <cstring>

class GHOST_WindowWayland;

class GHOST_SystemWayland : public GHOST_System {
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

	static void display_handle_global(
			void *data,
			struct wl_registry *registry,
			uint32_t id,
			const char *interface,
			uint32_t version);

	void display_handle(
			struct wl_registry *registry,
			uint32_t id,
			const char *interface,
			uint32_t version);

	template<typename T>
	void registry_bind(
		T &object,
		const char *name,
		const char *myname,
		uint32_t id,
		const wl_interface *interface);

	wayland_ptr<wl_display>::type m_display;
	wayland_ptr<wl_registry>::type m_registry;
	wayland_ptr<wl_compositor>::type m_compositor;
	wayland_ptr<wl_shell>::type m_shell;
	scoped_resource<EGLDisplay> m_egl_display;
	scoped_resource<EGLContext> m_egl_context;
	EGLConfig m_conf;
};

template<typename T> void
GHOST_SystemWayland::registry_bind(
	T &object,
	const char *name,
	const char *myname,
	uint32_t id,
	const wl_interface *interface)
{
	typedef typename T::pointer pointer;

	if (!std::strcmp(name, myname))
		object.reset(
			static_cast<pointer> (WL_CHK(wl_registry_bind(
				m_registry.get(),
				id,
				interface,
				1))));
}


#endif
