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

extern "C" {
#include "wayland-client.h"
#include "EGL/egl.h"
}

#include <boost/interprocess/smart_ptr/unique_ptr.hpp>
#include <boost/type_traits.hpp>

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

	boost::interprocess::unique_ptr<wl_display, void(*)(wl_display*)> m_wl_display;
	EGLDisplay m_egl_display;
	EGLContext m_egl_context;
	EGLConfig m_conf;
};

#endif
