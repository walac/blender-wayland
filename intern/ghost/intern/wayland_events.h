#ifndef GHOST_WAYLAND_EVENTS_H_
#define GHOST_WAYLAND_EVENTS_H_

#include <wayland-client.h>
#include <wayland-client-protocol.h>
#include <cassert>

namespace wl {

class display_events {
	virtual void error(
		struct wl_display *display,
		struct wl_object *object_id,
		uint32_t code,
		const char * message);

	virtual void delete_id(
		struct wl_display *display,
		uint32_t id);
};

class registry_events {
	virtual void global(
		struct wl_registry *registry,
		uint32_t name,
		const char *interface,
		uint32_t version);

	virtual void global_remove(
		struct wl_registry *registry,
		uint32_t name);
};

class callback_events {
	virtual void done(
		struct wl_callback *callback,
		uint32_t serial);
};

class shm_events {
	virtual void format(
		struct wl_shm *shm,
		uint32_t format);
};

class buffer_events {
	virtual void release(struct wl_buffer *buffer);
};

class data_offer_events {
	virtual void offer(struct wl_data_offer *data_offer, const char *type);
};

class data_source_events {
	virtual void target(
		struct wl_data_source *data_source,
		const char *mime_type);

	virtual void send(
		struct wl_data_sourcce *data_source,
		const char *mime_type,
		int32_t fd);

	virtual void cancelled(struct wl_data_source *data_source);
};

class data_device_events {
	virtual void data_offer(
		struct wl_data_device *data_device,
		struct wl_data_offer *id);

	virtual void enter(
		struct wl_data_device *data_device,
		uint32_t serial,
		struct wl_surface *surface,
		wl_fixed_t x,
		wl_fixed_t y,
		struct wl_data_offer *id);

	virtual void leave(struct wl_data_device *data_device);

	virtual void motion(
		struct wl_data_device *wl_data_device,
		uint32_t time,
		wl_fixed_t x,
		wl_fixed_t y);

	virtual void drop(struct wl_data_device *data_device);

	virtual void selection(
		struct wl_data_device *data_device,
		struct wl_data_offer *id);
};

class shell_surface_events {
	virtual void ping(
		struct wl_shell_surface *shell_surface,
		uint32_t serial);

	virtual void configure(
		struct wl_shell_surface *shell_surface,
		uint32_t edges,
		int32_t width,
		int32_t height);

	virtual void popup_done(struct wl_shell_surface *shell_surface);
};

class surface_events {
	virtual void enter(
		struct wl_surface *surface,
		struct wl_output *output);

	virtual void leave(
		struct wl_surface *wl_surface,
		struct wl_output *output);
};

class seat_events {
	virtual void capabilities(
		 struct wl_seat *seat,
		 uint32_t capabilities);

};

class pointer_events {
	virtual void enter(
		struct wl_pointer *pointer,
		uint32_t serial,
		struct wl_surface *surface,
		wl_fixed_t surface_x,
		wl_fixed_t surface_y);

	virtual void leave(
		struct wl_pointer *pointer,
		uint32_t serial,
		struct wl_surface *surface);

	virtual void motion(
		struct wl_pointer *pointer,
		uint32_t time,
		wl_fixed_t surface_x,
		wl_fixed_t surface_y);

	virtual void button(
		struct wl_pointer *pointer,
		uint32_t serial,
		uint32_t time,
		uint32_t button,
		uint32_t state);

	virtual void axis(
		struct wl_pointer *pointer,
		uint32_t time,
		uint32_t axis,
		wl_fixed_t value);
};

class keyboard_events {
	virtual void keymap(
		struct wl_keyboard *keyboard,
		uint32_t format,
		int32_t fd,
		uint32_t size);

	virtual void enter(
		struct wl_keyboard *keyboard,
		uint32_t serial,
		struct wl_surface *surface,
		struct wl_array *keys);

	virtual void leave(
		struct wl_keyboard *keyboard,
		uint32_t serial,
		struct wl_surface *surface);

	virtual void key(
		struct wl_keyboard *keyboard,
		uint32_t serial,
		uint32_t time,
		uint32_t key,
		uint32_t state);

	virtual void modifiers(
		struct wl_keyboard *keyboard,
		uint32_t serial,
		uint32_t mods_depressed,
		uint32_t mods_latched,
		uint32_t mods_locked,
		uint32_t group);
};

class touch_events {
	virtual void down(
		struct wl_touch *touch,
		uint32_t serial,
		uint32_t time,
		struct wl_surface *surface,
		int32_t id,
		wl_fixed_t x,
		wl_fixed_t y);

	virtual void up(
		struct wl_touch *touch,
		uint32_t serial,
		uint32_t time,
		int32_t id);

	virtual void motion(
		struct wl_touch *touch,
		uint32_t time,
		int32_t id,
		wl_fixed_t x,
		wl_fixed_t y);

	virtual void frame(struct wl_touch *touch);

	virtual void cancel(struct wl_touch *touch);
};

class output_events {
	virtual void geometry(
		struct wl_output *output,
		int32_t x,
		int32_t y,
		int32_t physical_width,
		int32_t physical_height,
		int32_t subpixel,
		const char *make,
		const char *model,
		int32_t transform);

 	virtual void mode(
		struct wl_output *output,
		uint32_t flags,
		int32_t width,
		int32_t height,
		int32_t refresh);
};

template<typename B, typename D, typename Proxy>
void add_listener(D *pthis, Proxy *object)
{
	wl_proxy *proxy = reinterpret_cast<wl_proxy *> (object);
	B *b = dynamic_cast<B*> (pthis);
	assert(b);
	wl_proxy_add_listener(proxy,
		*reinterpret_cast<void(***)(void)> (b), b);
}

template<typename T, typename U>
void set_user_data(T *object, U *data)
{
	wl_proxy_set_user_data(
		reinterpret_cast<wl_proxy *> (object),
		static_cast<void *> (data));
}

}

#endif // GHOST_WAYLAND_EVENTS_H_
