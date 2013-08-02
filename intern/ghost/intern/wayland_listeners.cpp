#include "wayland_listeners.h"

namespace wl {

void display_listener::error(
	struct wl_display *display,
	struct wl_object *object_id,
	uint32_t code,
	const char * message)
{
}

void display_listener::delete_id(
	struct wl_display *display,
	uint32_t id)
{
}

void registry_listener::global(
	struct wl_registry *registry,
	uint32_t name,
	const char *interface,
	uint32_t version)
{
}

void registry_listener::global_remove(
	struct wl_registry *registry,
	uint32_t name)
{
}

void callback_listener::done(
	struct wl_callback *callback,
	uint32_t serial)
{
}

void shm_listener::format(
	struct wl_shm *shm,
	uint32_t format)
{
}

void buffer_listener::release(struct wl_buffer *buffer)
{
}

void data_offer_listener::offer(struct wl_data_offer *data_offer, const char *type)
{
}

void data_source_listener::target(
	struct wl_data_source *data_source,
	const char *mime_type)
{
}

void data_source_listener::send(
	struct wl_data_sourcce *data_source,
	const char *mime_type,
	int32_t fd)
{
}

void data_source_listener::cancelled(struct wl_data_source *data_source)
{
}

void data_device_listener::data_offer(
	struct wl_data_device *data_device,
	struct wl_data_offer *id)
{
}

void data_device_listener::enter(
	struct wl_data_device *data_device,
	uint32_t serial,
	struct wl_surface *surface,
	wl_fixed_t x,
	wl_fixed_t y,
	struct wl_data_offer *id)
{
}

void data_device_listener::leave(struct wl_data_device *data_device)
{
}

void data_device_listener::motion(
	struct wl_data_device *wl_data_device,
	uint32_t time,
	wl_fixed_t x,
	wl_fixed_t y)
{
}

void data_device_listener::drop(struct wl_data_device *data_device)
{
}

void data_device_listener::selection(
	struct wl_data_device *data_device,
	struct wl_data_offer *id)
{
}

void shell_surface_listener::ping(
	struct wl_shell_surface *shell_surface,
	uint32_t serial)
{
}

void shell_surface_listener::configure(
	struct wl_shell_surface *shell_surface,
	uint32_t edges,
	int32_t width,
	int32_t height)
{
}

void shell_surface_listener::popup_done(struct wl_shell_surface *shell_surface)
{
}

void surface_listener::enter(
	struct wl_surface *surface,
	struct wl_output *output)
{
}

void surface_listener::leave(
	struct wl_surface *wl_surface,
	struct wl_output *output)
{
}

void seat_listener::capabilities(
	 struct wl_seat *seat,
	 uint32_t capabilities)
{
}

void pointer_listener::enter(
	struct wl_pointer *pointer,
	uint32_t serial,
	struct wl_surface *surface,
	wl_fixed_t surface_x,
	wl_fixed_t surface_y)
{
}

void pointer_listener::leave(
	struct wl_pointer *pointer,
	uint32_t serial,
	struct wl_surface *surface)
{
}

void pointer_listener::motion(
	struct wl_pointer *pointer,
	uint32_t time,
	wl_fixed_t surface_x,
	wl_fixed_t surface_y)
{
}

void pointer_listener::button(
	struct wl_pointer *pointer,
	uint32_t serial,
	uint32_t time,
	uint32_t button,
	uint32_t state)
{
}

void pointer_listener::axis(
	struct wl_pointer *pointer,
	uint32_t time,
	uint32_t axis,
	wl_fixed_t value)
{
}

void keyboard_listener::keymap(
	struct wl_keyboard *keyboard,
	uint32_t format,
	int32_t fd,
	uint32_t size)
{
}

void keyboard_listener::enter(
	struct wl_keyboard *keyboard,
	uint32_t serial,
	struct wl_surface *surface,
	struct wl_array *keys)
{
}

void keyboard_listener::leave(
	struct wl_keyboard *keyboard,
	uint32_t serial,
	struct wl_surface *surface)
{
}

void keyboard_listener::key(
	struct wl_keyboard *keyboard,
	uint32_t serial,
	uint32_t time,
	uint32_t key,
	uint32_t state)
{
}

void keyboard_listener::modifiers(
	struct wl_keyboard *keyboard,
	uint32_t serial,
	uint32_t mods_depressed,
	uint32_t mods_latched,
	uint32_t mods_locked,
	uint32_t group)
{
}

void touch_listener::down(
	struct wl_touch *touch,
	uint32_t serial,
	uint32_t time,
	struct wl_surface *surface,
	int32_t id,
	wl_fixed_t x,
	wl_fixed_t y)
{
}

void touch_listener::up(
	struct wl_touch *touch,
	uint32_t serial,
	uint32_t time,
	int32_t id)
{
}

void touch_listener::motion(
	struct wl_touch *touch,
	uint32_t time,
	int32_t id,
	wl_fixed_t x,
	wl_fixed_t y)
{
}

void touch_listener::frame(struct wl_touch *touch)
{
}

void touch_listener::cancel(struct wl_touch *touch)
{
}

void output_listener::geometry(
	struct wl_output *output,
	int32_t x,
	int32_t y,
	int32_t physical_width,
	int32_t physical_height,
	int32_t subpixel,
	const char *make,
	const char *model,
	int32_t transform)
{
}

void output_listener::mode(
	struct wl_output *output,
	uint32_t flags,
	int32_t width,
	int32_t height,
	int32_t refresh)
{
}

}
