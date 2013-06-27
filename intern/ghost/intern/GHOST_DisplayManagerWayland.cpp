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
 * Mode switching
 * Copyright (C) 1997-2001 Id Software, Inc.
 * Copyright (c) 1993-2011 Tim Riker
 * Copyright (C) 2012 Alex Fraser
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file ghost/intern/GHOST_DisplayManagerWayland.cpp
 *  \ingroup GHOST
 */

#include "GHOST_SystemWayland.h"
#include "GHOST_DisplayManagerWayland.h"

#include "GHOST_WindowManager.h"

GHOST_DisplayManagerWayland::GHOST_DisplayManagerWayland()
	: GHOST_DisplayManager()
{
}

GHOST_TSuccess
GHOST_DisplayManagerWayland::getNumDisplays(GHOST_TUns8& numDisplays) const
{
	numDisplays = 1;
	return GHOST_kSuccess;
}


GHOST_TSuccess GHOST_DisplayManagerWayland::getNumDisplaySettings(GHOST_TUns8 display,
                                                              GHOST_TInt32& numSettings) const
{
	GHOST_ASSERT(display < 1, "Only single display systems are currently supported.\n");

	numSettings = 1;

	return GHOST_kSuccess;
}

GHOST_TSuccess
GHOST_DisplayManagerWayland::getDisplaySetting(GHOST_TUns8 display,
                                           GHOST_TInt32 index,
                                           GHOST_DisplaySetting& setting) const
{
	GHOST_ASSERT(display < 1, "Only single display systems are currently supported.\n");

	(void) display;
	(void) index;
	(void) setting;

	return GHOST_kSuccess;
}

GHOST_TSuccess
GHOST_DisplayManagerWayland::getCurrentDisplaySetting(GHOST_TUns8 display,
                                                  GHOST_DisplaySetting& setting) const
{
	(void) display;
	(void) setting;

	return GHOST_kSuccess;
}

GHOST_TSuccess
GHOST_DisplayManagerWayland:: setCurrentDisplaySetting(GHOST_TUns8 display,
                                                   const GHOST_DisplaySetting& setting)
{
	(void) display;
	(void) setting;
	return GHOST_kSuccess;
}
