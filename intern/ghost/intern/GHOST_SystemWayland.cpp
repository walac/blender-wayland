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
#include <sys/mman.h>
#include <unistd.h>

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
	, m_xkb_context(xkb_context_new(xkb_context_flags(0)))
	, m_xkb_keymap(NULL)
	, m_xkb_state(NULL)
	, m_active_window(NULL)
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

	assert(m_xkb_context);

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
	if (m_xkb_state)
		xkb_state_unref(m_xkb_state);

	if (m_xkb_keymap)
		xkb_map_unref(m_xkb_keymap);

	if (m_xkb_context)
		xkb_context_unref(m_xkb_context);

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
	keys.set(GHOST_kModifierKeyRightControl, m_mod_state & m_mod_masks.rcontrol);
	keys.set(GHOST_kModifierKeyLeftControl, m_mod_state & m_mod_masks.lcontrol);
	keys.set(GHOST_kModifierKeyRightAlt, m_mod_state & m_mod_masks.ralt);
	keys.set(GHOST_kModifierKeyLeftAlt, m_mod_state & m_mod_masks.lalt);

	/*
	 * libxkbcommon does not support left and right shift distinction yet
	 *
	 * ref: http://lists.freedesktop.org/archives/wayland-devel/2013-September/011239.html
	 */
	keys.set(GHOST_kModifierKeyLeftShift, m_mod_state & m_mod_masks.shift);
	keys.set(GHOST_kModifierKeyRightShift, false);

	keys.set(GHOST_kModifierKeyOS, m_mod_state & m_mod_masks.super);
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

void
GHOST_SystemWayland::keymap(
	struct wl_keyboard *keyboard,
	uint32_t format,
	int32_t fd,
	uint32_t size)
{
	assert(keyboard == m_keyboard);
	char *map_str = NULL;
	xkb_mod_index_t n;
	if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1)
		goto end;

	map_str = static_cast<char *> (mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0));
	if (MAP_FAILED == map_str)
		goto end;

	m_xkb_keymap =
		xkb_map_new_from_string(
			m_xkb_context,
			map_str,
			XKB_KEYMAP_FORMAT_TEXT_V1,
			XKB_MAP_COMPILE_PLACEHOLDER);

	assert(m_xkb_keymap);

	if (!m_xkb_keymap)
		goto end;

	m_xkb_state = xkb_state_new(m_xkb_keymap);

	assert(m_xkb_state);

	if (!m_xkb_state) {
		xkb_map_unref(m_xkb_keymap);
		m_xkb_keymap = NULL;
	}

	m_mod_masks.rcontrol = 1 << xkb_map_mod_get_index(m_xkb_keymap, "RControl");
	m_mod_masks.lcontrol = 1 << xkb_map_mod_get_index(m_xkb_keymap, "LControl");
	m_mod_masks.ralt = 1 << xkb_map_mod_get_index(m_xkb_keymap, "RAlt");
	m_mod_masks.lalt = 1 << xkb_map_mod_get_index(m_xkb_keymap, "LAlt");
	m_mod_masks.shift = 1 << xkb_map_mod_get_index(m_xkb_keymap, "Shift");
	m_mod_masks.super = 1 << xkb_map_mod_get_index(m_xkb_keymap, "Super");

end:
	if (map_str)
		munmap(map_str, size);

	close(fd);
}

void
GHOST_SystemWayland::enter(
	struct wl_keyboard *keyboard,
	uint32_t serial,
	struct wl_surface *surface,
	struct wl_array *keys)
{
	assert(m_keyboard == keyboard);
	m_active_window = wl::get_user_data<GHOST_WindowWayland> (surface);
	pushEvent(new GHOST_Event(
		getMilliSeconds(),
		GHOST_kEventWindowActivate,
		m_active_window));
}

void
GHOST_SystemWayland::leave(
	struct wl_keyboard *keyboard,
	uint32_t serial,
	struct wl_surface *surface)
{
	assert(m_keyboard == keyboard);
	pushEvent(new GHOST_Event(
		getMilliSeconds(),
		GHOST_kEventWindowDeactivate,
		m_active_window));
	m_active_window = NULL;
}

#define GXMAP(x, y) case x: return y; break

static GHOST_TKey
convertXKBKey(xkb_keysym_t key)
{
	if ((key >= XKB_KEY_A) && (key <= XKB_KEY_Z)) {
		return GHOST_TKey(key - XKB_KEY_A + int(GHOST_kKeyA));
	}
	else if ((key >= XKB_KEY_a) && (key <= XKB_KEY_z)) {
		return GHOST_TKey(key - XKB_KEY_a + int(GHOST_kKeyA));
	}
	else if ((key >= XKB_KEY_0) && (key <= XKB_KEY_9)) {
		return (key == XKB_KEY_0) ? GHOST_kKey0 : GHOST_TKey(key - XKB_KEY_1 + int(GHOST_kKey1));
	}
	else if ((key >= XKB_KEY_F1) && (key <= XKB_KEY_F12)) {
		return GHOST_TKey(key - XKB_KEY_F1 + int(GHOST_kKeyF1));
	}
	else if ((key >= XKB_KEY_F13) && (key <= XKB_KEY_F24)) {
		return GHOST_TKey(key - XKB_KEY_F13 + int(GHOST_kKeyF13));
	}
	else {
		switch (key) {
			/* TODO XKB_KEY_NONUSBACKSLASH */

			GXMAP(XKB_KEY_BackSpace,      GHOST_kKeyBackSpace);
			GXMAP(XKB_KEY_Tab,            GHOST_kKeyTab);
			GXMAP(XKB_KEY_Return,         GHOST_kKeyEnter);
			GXMAP(XKB_KEY_Escape,         GHOST_kKeyEsc);
			GXMAP(XKB_KEY_space,          GHOST_kKeySpace);

			GXMAP(XKB_KEY_semicolon,      GHOST_kKeySemicolon);
			GXMAP(XKB_KEY_period,         GHOST_kKeyPeriod);
			GXMAP(XKB_KEY_comma,          GHOST_kKeyComma);
			GXMAP(XKB_KEY_apostrophe,     GHOST_kKeyQuote);
			GXMAP(XKB_KEY_grave,          GHOST_kKeyAccentGrave);
			GXMAP(XKB_KEY_minus,          GHOST_kKeyMinus);
			GXMAP(XKB_KEY_equal,          GHOST_kKeyEqual);

			GXMAP(XKB_KEY_slash,          GHOST_kKeySlash);
			GXMAP(XKB_KEY_backslash,      GHOST_kKeyBackslash);
			GXMAP(XKB_KEY_KP_Equal,       GHOST_kKeyEqual);
			GXMAP(XKB_KEY_bracketleft,    GHOST_kKeyLeftBracket);
			GXMAP(XKB_KEY_bracketright,   GHOST_kKeyRightBracket);
			GXMAP(XKB_KEY_Pause,          GHOST_kKeyPause);

			GXMAP(XKB_KEY_Shift_L,        GHOST_kKeyLeftShift);
			GXMAP(XKB_KEY_Shift_R,        GHOST_kKeyRightShift);
			GXMAP(XKB_KEY_Control_L,      GHOST_kKeyLeftControl);
			GXMAP(XKB_KEY_Control_R,      GHOST_kKeyRightControl);
			GXMAP(XKB_KEY_Alt_L,          GHOST_kKeyLeftAlt);
			GXMAP(XKB_KEY_Alt_R,          GHOST_kKeyRightAlt);
			GXMAP(XKB_KEY_Super_L,        GHOST_kKeyOS);
			GXMAP(XKB_KEY_Super_R,        GHOST_kKeyOS);

			GXMAP(XKB_KEY_Insert,         GHOST_kKeyInsert);
			GXMAP(XKB_KEY_Delete,         GHOST_kKeyDelete);
			GXMAP(XKB_KEY_Home,           GHOST_kKeyHome);
			GXMAP(XKB_KEY_End,            GHOST_kKeyEnd);
			GXMAP(XKB_KEY_Page_Up,        GHOST_kKeyUpPage);
			GXMAP(XKB_KEY_Page_Down,      GHOST_kKeyDownPage);

			GXMAP(XKB_KEY_Left,           GHOST_kKeyLeftArrow);
			GXMAP(XKB_KEY_Right,          GHOST_kKeyRightArrow);
			GXMAP(XKB_KEY_Up,             GHOST_kKeyUpArrow);
			GXMAP(XKB_KEY_Down,           GHOST_kKeyDownArrow);

			GXMAP(XKB_KEY_Caps_Lock,      GHOST_kKeyCapsLock);
			GXMAP(XKB_KEY_Scroll_Lock,    GHOST_kKeyScrollLock);
			GXMAP(XKB_KEY_Num_Lock,       GHOST_kKeyNumLock);


			/* keypad events */

			GXMAP(XKB_KEY_KP_0,           GHOST_kKeyNumpad0);
			GXMAP(XKB_KEY_KP_1,           GHOST_kKeyNumpad1);
			GXMAP(XKB_KEY_KP_2,           GHOST_kKeyNumpad2);
			GXMAP(XKB_KEY_KP_3,           GHOST_kKeyNumpad3);
			GXMAP(XKB_KEY_KP_4,           GHOST_kKeyNumpad4);
			GXMAP(XKB_KEY_KP_5,           GHOST_kKeyNumpad5);
			GXMAP(XKB_KEY_KP_6,           GHOST_kKeyNumpad6);
			GXMAP(XKB_KEY_KP_7,           GHOST_kKeyNumpad7);
			GXMAP(XKB_KEY_KP_8,           GHOST_kKeyNumpad8);
			GXMAP(XKB_KEY_KP_9,           GHOST_kKeyNumpad9);
			GXMAP(XKB_KEY_KP_Decimal,     GHOST_kKeyNumpadPeriod);

			GXMAP(XKB_KEY_KP_Enter,       GHOST_kKeyNumpadEnter);
			GXMAP(XKB_KEY_KP_Add,         GHOST_kKeyNumpadPlus);
			GXMAP(XKB_KEY_KP_Subtract,    GHOST_kKeyNumpadMinus);
			GXMAP(XKB_KEY_KP_Multiply,    GHOST_kKeyNumpadAsterisk);
			GXMAP(XKB_KEY_KP_Divide,      GHOST_kKeyNumpadSlash);

			/* Media keys in some keyboards and laptops with XFree86/Xorg */
			GXMAP(XKB_KEY_XF86AudioPlay,      GHOST_kKeyMediaPlay);
			GXMAP(XKB_KEY_XF86AudioStop,      GHOST_kKeyMediaStop);
			GXMAP(XKB_KEY_XF86AudioPrev,      GHOST_kKeyMediaFirst);
			GXMAP(XKB_KEY_XF86AudioRewind,    GHOST_kKeyMediaFirst);
			GXMAP(XKB_KEY_XF86AudioNext,      GHOST_kKeyMediaLast);

			default:
			fprintf(stderr, "Unknown\n");
			return GHOST_kKeyUnknown;
		}
	}
}

#undef GXMAP

void
GHOST_SystemWayland::key(
	struct wl_keyboard *keyboard,
	uint32_t serial,
	uint32_t time,
	uint32_t key,
	uint32_t state)
{
	assert(m_keyboard == keyboard);
	xkb_keysym_t sym;
	GHOST_TKey ghost_key;
	char utf8[8];
	wl_keyboard_key_state key_state =
		static_cast<wl_keyboard_key_state> (state);

	if (!m_xkb_state)
		return;

	sym = xkb_state_key_get_one_sym(m_xkb_state, key + 8);
	ghost_key = convertXKBKey(sym);

	int size = xkb_keysym_to_utf8(sym, utf8, sizeof(utf8)/sizeof(utf8[0]));

	if (size < 0) {
		fprintf(stderr, "xkb_keysym_to_utf8 failed\n");
		return;
	} else if (!size) {
		utf8[0] = '\0';
	}

	pushEvent(new GHOST_EventKey(
				getMilliSeconds(),
				WL_KEYBOARD_KEY_STATE_PRESSED == key_state
				? GHOST_kEventKeyDown
				: GHOST_kEventKeyUp,
				m_active_window,
				ghost_key,
				'\0',
				utf8));
}

void
GHOST_SystemWayland::modifiers(
	struct wl_keyboard *keyboard,
	uint32_t serial,
	uint32_t mods_depressed,
	uint32_t mods_latched,
	uint32_t mods_locked,
	uint32_t group)
{
	assert(m_keyboard == keyboard);

	if (!m_xkb_keymap)
		return;

	xkb_state_update_mask(
		m_xkb_state,
		mods_depressed,
		mods_latched,
		mods_locked,
		0,
		0,
		group);

	m_mod_state =
		xkb_state_serialize_mods(
			m_xkb_state,
			xkb_state_component(
				XKB_STATE_DEPRESSED
				| XKB_STATE_LATCHED));
}

