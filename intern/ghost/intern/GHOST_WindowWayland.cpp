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
#include <assert.h>

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
{
	setTitle(title);
}

GHOST_WindowWayland::~GHOST_WindowWayland()
{
}


GHOST_TSuccess
GHOST_WindowWayland::installDrawingContext(GHOST_TDrawingContextType type)
{
	// only support openGL for now.
	GHOST_TSuccess success;
	switch (type) {
		case GHOST_kDrawingContextTypeOpenGL:
			success = GHOST_kSuccess;
			break;

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
	return GHOST_kSuccess;
}


GHOST_TSuccess
GHOST_WindowWayland::activateDrawingContext()
{
	return GHOST_kSuccess;
}


GHOST_TSuccess
GHOST_WindowWayland::removeDrawingContext()
{
	return GHOST_kSuccess;
}


GHOST_TSuccess
GHOST_WindowWayland::setState(GHOST_TWindowState state)
{
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

	return GHOST_kSuccess;
}


GHOST_TWindowState
GHOST_WindowWayland::getState() const
{
	return GHOST_kWindowStateNormal;
}


void
GHOST_WindowWayland::setTitle(const STR_String& title)
{
	(void) title;
}


void
GHOST_WindowWayland::getTitle(STR_String& title) const
{
	title = "";
}


void
GHOST_WindowWayland::getWindowBounds(GHOST_Rect& bounds) const
{
	getClientBounds(bounds);
}


void
GHOST_WindowWayland::getClientBounds(GHOST_Rect& bounds) const
{
	bounds.m_l = 0;
	bounds.m_r = 500;
	bounds.m_t = 0;
	bounds.m_b = 300;
}

GHOST_TSuccess
GHOST_WindowWayland::setClientWidth(GHOST_TUns32 width)
{
	(void) width;
	return GHOST_kSuccess;
}

GHOST_TSuccess
GHOST_WindowWayland::setClientHeight(GHOST_TUns32 height)
{
	(void) height;
	return GHOST_kSuccess;
}

GHOST_TSuccess
GHOST_WindowWayland::setClientSize(GHOST_TUns32 width,
		GHOST_TUns32 height)
{
	(void) width;
	(void) height;
	return GHOST_kSuccess;
}

void
GHOST_WindowWayland::screenToClient(GHOST_TInt32 inX, GHOST_TInt32 inY, GHOST_TInt32& outX, GHOST_TInt32& outY) const
{
	(void) inX;
	(void) inY;
	(void) outX;
	(void) outY;
}
void
GHOST_WindowWayland::clientToScreen(GHOST_TInt32 inX, GHOST_TInt32 inY, GHOST_TInt32& outX, GHOST_TInt32& outY) const
{
	(void) inX;
	(void) inY;
	(void) outX;
	(void) outY;
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
