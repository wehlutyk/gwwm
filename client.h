#include <libguile.h>
/*
 * Attempt to consolidate unavoidable suck into one file, away from dwl.c.  This
 * file is not meant to be pretty.  We use a .h file with static inline
 * functions instead of a separate .c module, or function pointers like sway, so
 * that they will simply compile out if the chosen #defines leave them unused.
 */

/* Leave these functions first; they're used in the others */

#define MAKE_CLIENT(o) (scm_call_3(REF("oop goops","make"), \
                                  REF("gwwm client","<gwwm-client>"), \
                                  scm_from_utf8_keyword("data"), \
                                  FROM_P(o)))

#define WRAP_CLIENT(o) find_client(o)
#define UNWRAP_CLIENT(o) unwrap_client_1(o)

#define INNER_CLIENTS_HASH_TABLE REFP("gwwm client", "%clients")
#define CLIENT_IS_FULLSCREEN(c) scm_to_bool(REF_CALL_1("gwwm client" ,"client-fullscreen?",WRAP_CLIENT(c)))
#define CLIENT_SET_FULLSCREEN(c ,f) \
  (REF_CALL_2("gwwm client","client-set-fullscreen!",(WRAP_CLIENT(c)), (scm_from_bool(f))))
#define CLIENT_IS_FLOATING(c) scm_to_bool(REF_CALL_1("gwwm client" ,"client-floating?",WRAP_CLIENT(c)))
#define CLIENT_SET_FLOATING(c ,f) \
  (REF_CALL_2("gwwm client","client-set-floating!",(WRAP_CLIENT(c)), (scm_from_bool(f))))
#define CLIENT_IS_URGENT_P(c) scm_to_bool(REF_CALL_1("gwwm client" ,"client-urgent?",WRAP_CLIENT(c)))
#define CLIENT_SET_URGENT(c ,f) \
  (REF_CALL_2("gwwm client","client-set-urgent!",(WRAP_CLIENT(c)), (scm_from_bool(f))))
#define CLIENT_SCENE(c) ((struct wlr_scene_node *) (UNWRAP_WLR_SCENE_NODE(REF_CALL_1("gwwm client" ,"client-scene",WRAP_CLIENT(c)))))
#define CLIENT_SET_SCENE(c,s) (REF_CALL_2("gwwm client","client-set-scene!",\
                                          (WRAP_CLIENT(c)),             \
                                          (WRAP_WLR_SCENE_NODE(s))))
#define CLIENT_BW(c) (scm_to_unsigned_integer(REF_CALL_1("gwwm client" ,"client-border-width",WRAP_CLIENT(c)),0,100))
#define CLIENT_SET_BW(c,b) (REF_CALL_2("gwwm client","client-set-border-width!", \
                                       (WRAP_CLIENT(c)),                \
                                       scm_from_unsigned_integer(b)))

#define CLIENT_TYPE(c) (scm_to_utf8_string(REF_CALL_1("gwwm client" ,"client-type",WRAP_CLIENT(c))))
#define CLIENT_SET_TYPE(c,b) (scm_call_2(REFP("gwwm client","client-set-type!"), \
                                       (WRAP_CLIENT(c)),                \
                                       scm_from_utf8_string(b)))

#define CLIENT_SURFACE(c) ((struct wlr_surface *)UNWRAP_WLR_SURFACE(REF_CALL_1("gwwm client" ,"client-surface",WRAP_CLIENT(c))))
#define CLIENT_SET_SURFACE(c,b) (scm_call_2(REF_CALL_1("guile" ,"setter",(REF("gwwm client","client-surface"))), \
                                            (WRAP_CLIENT(c)), \
                                            (WRAP_WLR_SURFACE(b))))

#define CLIENT_IS_LAYER_SHELL(cl) SCM_IS_A_P(cl,REFP("gwwm client","<gwwm-layer-client>"))
#define CLIENT_IS_XDG_SHELL(cl) (scm_string_equal_p((REF_CALL_1("gwwm client" ,"client-type",WRAP_CLIENT(c))), scm_from_utf8_string("XDGShell")))
#define CLIENT_IS_MANAGED(c) (scm_string_equal_p((REF_CALL_1("gwwm client" ,"client-type",WRAP_CLIENT(c))) , scm_from_utf8_string("X11Managed")))
#define CLIENT_SCENE_SURFACE(c) c->scene_surface
SCM gwwm_client_type(SCM c);
static inline SCM
find_client(void *c) {
  SCM p =(scm_pointer_address(scm_from_pointer(c ,NULL)));
  return scm_hashq_ref(INNER_CLIENTS_HASH_TABLE, p ,
                       scm_hashq_ref(REFP("gwwm client", "%layer-clients"),
                                     p ,SCM_BOOL_F));
}

static inline Client* unwrap_client_1(SCM o)
{
  SCM a=scm_call_1(REFP("gwwm client",".data"),o);
  if (scm_is_false(a)) {
    scm_error(scm_misc_error_key,"unwrap-client","client is delated" ,SCM_EOL,SCM_EOL);
    return NULL;
  }
  return (TO_P(MAKE_P(a)));
}

static inline void
register_client(void *c, char *type) {
  scm_hashq_set_x((type=="<gwwm-layer-client>"
                   ? REFP("gwwm client", "%layer-clients")
                   : INNER_CLIENTS_HASH_TABLE),
                  (scm_pointer_address(FROM_P(c))),
                  (scm_call_3(REF("oop goops","make"),
                              REF("gwwm client",type),
                              scm_from_utf8_keyword("data"),
                              scm_pointer_address(FROM_P(c)))));
}

static inline void
logout_client(void *c){
  scm_call_1(REFP("gwwm client","logout-client") ,WRAP_CLIENT(c));
  /* scm_hashq_remove_x(((scm_to_utf8_string(gwwm_client_type(WRAP_CLIENT(c)))== "LayerShell") */
  /*                     ? REFP("gwwm client", "%layer-clients") */
  /*                     : INNER_CLIENTS_HASH_TABLE), */
  /*                    scm_pointer_address(scm_from_pointer(c ,NULL))); */
    /* scm_slot_set_x(WRAP_CLIENT(c),scm_from_utf8_symbol("data"),SCM_BOOL_F); */
    free(c);
}
static inline struct wlr_scene_rect *
client_border_n(Client *c, int n)
{
  /* return c->border[n]; */
  return (UNWRAP_WLR_SCENE_RECT(scm_list_ref(scm_slot_ref(WRAP_CLIENT(c), scm_from_utf8_symbol("borders")),scm_from_int(n))));
}

#define CLIENT_SET_BORDER_COLOR(c,color) scm_call_2(REFP("gwwm client" ,"client-set-border-color"),WRAP_CLIENT(c),color)

void
client_init_border(Client *c)
{
  PRINT_FUNCTION;
  SCM list =scm_make_list(scm_from_int(0), SCM_UNSPECIFIED);
  for (int i = 0; i < 4; i++) {
    struct wlr_scene_rect *rect= wlr_scene_rect_create(CLIENT_SCENE(c), 0, 0, GWWM_BORDERCOLOR());
    rect->node.data = c;
    wlr_scene_rect_set_color(rect, GWWM_BORDERCOLOR());
    list=scm_cons(WRAP_WLR_SCENE_RECT(rect), list);
  }
  scm_slot_set_x(WRAP_CLIENT(c), scm_from_utf8_symbol("borders"), list);
}

static inline int
client_is_x11(Client *c)
{
  return (scm_to_bool(REF_CALL_1("gwwm client","client-is-x11?", WRAP_CLIENT(c))));
}

static inline Client *
client_from_wlr_surface(struct wlr_surface *s)
{
	struct wlr_xdg_surface *surface;
	struct wlr_surface *parent;

#ifdef XWAYLAND
	struct wlr_xwayland_surface *xsurface;
	if (s && wlr_surface_is_xwayland_surface(s)
			&& (xsurface = wlr_xwayland_surface_from_wlr_surface(s)))
		return xsurface->data;
#endif
	if (s && wlr_surface_is_xdg_surface(s)
			&& (surface = wlr_xdg_surface_from_wlr_surface(s))
			&& surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL)
		return surface->data;

	if (s && wlr_surface_is_subsurface(s))
		return client_from_wlr_surface(wlr_surface_get_root_surface(s));
	return NULL;
}

/* The others */
static inline void
client_activate_surface(struct wlr_surface *s, int activated)
{
	struct wlr_xdg_surface *surface;
#ifdef XWAYLAND
	struct wlr_xwayland_surface *xsurface;
	if (wlr_surface_is_xwayland_surface(s)
			&& (xsurface = wlr_xwayland_surface_from_wlr_surface(s))) {
		wlr_xwayland_surface_activate(xsurface, activated);
		return;
	}
#endif
	if (wlr_surface_is_xdg_surface(s)
			&& (surface = wlr_xdg_surface_from_wlr_surface(s))
			&& surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL)
		wlr_xdg_toplevel_set_activated(surface, activated);
}

static inline void
client_for_each_surface(Client *c, wlr_surface_iterator_func_t fn, void *data)
{
	wlr_surface_for_each_surface(CLIENT_SURFACE(c), fn, data);
#ifdef XWAYLAND
	if (client_is_x11(c))
		return;
#endif
	wlr_xdg_surface_for_each_popup_surface(wlr_xdg_surface_from_wlr_surface(CLIENT_SURFACE(c)), fn, data);
}

static inline const char *
client_get_appid(Client *c)
{
  return scm_to_utf8_string(REF_CALL_1("gwwm client", "client-get-appid",(WRAP_CLIENT(c))));
}

static inline void
client_get_geometry(Client *c, struct wlr_box *geom)
{
#ifdef XWAYLAND
	if (client_is_x11(c)) {
      geom->x = wlr_xwayland_surface_from_wlr_surface(CLIENT_SURFACE(c))->x;
		geom->y = wlr_xwayland_surface_from_wlr_surface(CLIENT_SURFACE(c))->y;
		geom->width = wlr_xwayland_surface_from_wlr_surface(CLIENT_SURFACE(c))->width;
		geom->height = wlr_xwayland_surface_from_wlr_surface(CLIENT_SURFACE(c))->height;
		return;
	}
#endif
	wlr_xdg_surface_get_geometry(wlr_xdg_surface_from_wlr_surface(CLIENT_SURFACE(c)), geom);
}

static inline void
client_get_size_hints(Client *c, struct wlr_box *max, struct wlr_box *min)
{
	struct wlr_xdg_toplevel *toplevel;
	struct wlr_xdg_toplevel_state *state;
#ifdef XWAYLAND
	if (client_is_x11(c)) {
		struct wlr_xwayland_surface_size_hints *size_hints;
		size_hints = wlr_xwayland_surface_from_wlr_surface(CLIENT_SURFACE(c))->size_hints;
		if (size_hints) {
			max->width = size_hints->max_width;
			max->height = size_hints->max_height;
			min->width = size_hints->min_width;
			min->height = size_hints->min_height;
		}
		return;
	}
#endif
	toplevel = wlr_xdg_surface_from_wlr_surface(CLIENT_SURFACE(c))->toplevel;
	state = &toplevel->current;
	max->width = state->max_width;
	max->height = state->max_height;
	min->width = state->min_width;
	min->height = state->min_height;
}

SCM_DEFINE (gwwm_client_get_size_hints,"client-get-size-hints",1,0,0,
            (SCM c),"")
#define FUNC_NAME s_gwwm_client_get_size_hints
{
  GWWM_ASSERT_CLIENT_OR_FALSE(c ,1);
  struct wlr_box min = {0}, max = {0};
  Client *cl =UNWRAP_CLIENT(c);
  client_get_size_hints(cl, &max, &min);
  return scm_cons(WRAP_WLR_BOX(&max),WRAP_WLR_BOX(&min));
}
#undef FUNC_NAME

static inline const char *
client_get_title(Client *c)
{
  return (scm_to_utf8_string(REF_CALL_1("gwwm client","client-get-title", WRAP_CLIENT(c))));
}

static inline Client *
client_get_parent(Client *c)
{
  PRINT_FUNCTION;
	Client *p;
#ifdef XWAYLAND
	if (client_is_x11(c) && wlr_xwayland_surface_from_wlr_surface(CLIENT_SURFACE(c))->parent){
      return client_from_wlr_surface(wlr_xwayland_surface_from_wlr_surface(CLIENT_SURFACE(c))->parent->surface);
    }
#endif
    PRINT_FUNCTION;
	if (wlr_surface_is_xdg_surface(CLIENT_SURFACE(c)) && wlr_xdg_surface_from_wlr_surface(CLIENT_SURFACE(c))->toplevel->parent){
      PRINT_FUNCTION;
      return client_from_wlr_surface(wlr_xdg_surface_from_wlr_surface(CLIENT_SURFACE(c))->toplevel->parent->surface);
    }

	return NULL;
}

static inline int
client_is_float_type(Client *c)
{
	struct wlr_box min = {0}, max = {0};
	client_get_size_hints(c, &max, &min);

#ifdef XWAYLAND
	if (client_is_x11(c)) {
		struct wlr_xwayland_surface *surface = wlr_xwayland_surface_from_wlr_surface(CLIENT_SURFACE(c));
		if (surface->modal)
			return 1;

		for (size_t i = 0; i < surface->window_type_len; i++)
			if (surface->window_type[i] == netatom[NetWMWindowTypeDialog]
					|| surface->window_type[i] == netatom[NetWMWindowTypeSplash]
					|| surface->window_type[i] == netatom[NetWMWindowTypeToolbar]
					|| surface->window_type[i] == netatom[NetWMWindowTypeUtility])
				return 1;

		return ((min.width > 0 || min.height > 0 || max.width > 0 || max.height > 0)
			&& (min.width == max.width || min.height == max.height))
			|| wlr_xwayland_surface_from_wlr_surface(CLIENT_SURFACE(c))->parent;
	}
#endif

	return ((min.width > 0 || min.height > 0 || max.width > 0 || max.height > 0)
		&& (min.width == max.width || min.height == max.height))
		|| wlr_xdg_surface_from_wlr_surface(CLIENT_SURFACE(c))->toplevel->parent;
}

SCM_DEFINE (gwwm_client_is_float_type_p,"client-is-float-type?",1,0,0,
            (SCM c),"")
#define FUNC_NAME s_gwwm_client_is_float_type_p
{
  GWWM_ASSERT_CLIENT_OR_FALSE(c ,1);
  return scm_from_bool(client_is_float_type(UNWRAP_CLIENT(c)));
}
#undef FUNC_NAME

static inline int
client_is_mapped(Client *c)
{
#ifdef XWAYLAND
	if (client_is_x11(c))
		return wlr_xwayland_surface_from_wlr_surface(CLIENT_SURFACE(c))->mapped;
#endif
	return wlr_xdg_surface_from_wlr_surface(CLIENT_SURFACE(c))->mapped;
}

static inline int
client_wants_fullscreen(Client *c)
{
#ifdef XWAYLAND
	if (client_is_x11(c))
		return wlr_xwayland_surface_from_wlr_surface(CLIENT_SURFACE(c))->fullscreen;
#endif
	return wlr_xdg_surface_from_wlr_surface(CLIENT_SURFACE(c))->toplevel->requested.fullscreen;
}

static inline int
client_is_unmanaged(Client *c)
{
  return (scm_to_bool(REF_CALL_1("gwwm client","client-is-unmanaged?", WRAP_CLIENT(c))));
}

static inline void
client_notify_enter(struct wlr_surface *s, struct wlr_keyboard *kb)
{
	if (kb)
		wlr_seat_keyboard_notify_enter(seat, s, kb->keycodes,
				kb->num_keycodes, &kb->modifiers);
	else
		wlr_seat_keyboard_notify_enter(seat, s, NULL, 0, NULL);
}

static inline void
client_send_close(Client *c)
{
  REF_CALL_1("gwwm client","client-send-close" ,WRAP_CLIENT(c));
}

static inline void
client_set_fullscreen(Client *c, int fullscreen)
{
  scm_call_2(REFP("gwwm client","client-do-set-fullscreen"),
             WRAP_CLIENT(c),
             scm_from_bool(fullscreen));
}

SCM_DEFINE (gwwm_client_xdg_surface ,"client-xdg-surface",1,0,0,(SCM c),"")
#define FUNC_NAME s_gwwm_client_xdg_surface
{
  GWWM_ASSERT_CLIENT_OR_FALSE(c ,1);
  Client *cl= (UNWRAP_CLIENT(c));
  return WRAP_WLR_XDG_SURFACE(wlr_xdg_surface_from_wlr_surface(CLIENT_SURFACE(cl) ));
}
#undef FUNC_NAME

SCM_DEFINE (gwwm_client_xwayland_surface ,"client-xwayland-surface",1,0,0,(SCM c),"")
#define FUNC_NAME s_gwwm_client_xwayland_surface
{
  GWWM_ASSERT_CLIENT_OR_FALSE(c ,1);
  Client *cl=(UNWRAP_CLIENT(c));
  return WRAP_WLR_XWAYLAND_SURFACE(wlr_xwayland_surface_from_wlr_surface(CLIENT_SURFACE(cl)));
}
#undef FUNC_NAME



static inline uint32_t
client_set_size(Client *c, uint32_t width, uint32_t height)
{
#ifdef XWAYLAND
	if (client_is_x11(c)) {
		wlr_xwayland_surface_configure(wlr_xwayland_surface_from_wlr_surface(CLIENT_SURFACE(c)),
				c->geom.x, c->geom.y, width, height);
		return 0;
	}
#endif
	return wlr_xdg_toplevel_set_size(wlr_xdg_surface_from_wlr_surface(CLIENT_SURFACE(c)), width, height);
}

static inline void
client_set_tiled(Client *c, uint32_t edges)
{
  REF_CALL_2("gwwm client","client-set-tiled" ,WRAP_CLIENT(c), scm_from_uint32(edges));
}

static inline struct wlr_surface *
client_surface_at(Client *c, double cx, double cy, double *sx, double *sy)
{
#ifdef XWAYLAND
	if (client_is_x11(c))
		return wlr_surface_surface_at(CLIENT_SURFACE(c),
				cx, cy, sx, sy);
#endif
	return wlr_xdg_surface_surface_at(wlr_xdg_surface_from_wlr_surface(CLIENT_SURFACE(c)), cx, cy, sx, sy);
}

static inline void
client_restack_surface(Client *c)
{
#ifdef XWAYLAND
	if (client_is_x11(c))
		wlr_xwayland_surface_restack(wlr_xwayland_surface_from_wlr_surface(CLIENT_SURFACE(c)), NULL,
				XCB_STACK_MODE_ABOVE);
#endif
	return;
}
static inline void
client_set_resizing(Client *c,int resizing)
{
  REF_CALL_2("gwwm client","client-set-resizing!" ,WRAP_CLIENT(c), scm_from_bool(resizing));
}

static inline void *
toplevel_from_popup(struct wlr_xdg_popup *popup)
{
	struct wlr_xdg_surface *surface = popup->base;

	while (1) {
		switch (surface->role) {
		case WLR_XDG_SURFACE_ROLE_POPUP:
			if (wlr_surface_is_layer_surface(surface->popup->parent))
				return wlr_layer_surface_v1_from_wlr_surface(surface->popup->parent)->data;
			else if (!wlr_surface_is_xdg_surface(surface->popup->parent))
				return NULL;

			surface = wlr_xdg_surface_from_wlr_surface(surface->popup->parent);
			break;
		case WLR_XDG_SURFACE_ROLE_TOPLEVEL:
				return surface->data;
		case WLR_XDG_SURFACE_ROLE_NONE:
			return NULL;
		}
	}
}

SCM_DEFINE (gwwm_client_get_parent, "client-get-parent" ,1,0,0,
            (SCM c), "")
#define FUNC_NAME s_gwwm_client_get_parent
{
  GWWM_ASSERT_CLIENT_OR_FALSE(c ,1);
  Client *cl = UNWRAP_CLIENT(c);
  Client *p = client_get_parent(cl);
  if (p) {
    return WRAP_CLIENT(p);
  };
  return SCM_BOOL_F;
}
#undef FUNC_NAME

SCM_DEFINE (gwwm_client_wants_fullscreen_p , "client-wants-fullscreen?",1,0,0,
            (SCM client), "")
#define FUNC_NAME s_gwwm_client_wants_fullscreen_p
{
  GWWM_ASSERT_CLIENT_OR_FALSE(client ,1);
  return scm_from_bool(client_wants_fullscreen(UNWRAP_CLIENT(client)));
}
#undef FUNC_NAME

SCM_DEFINE (gwwm_setfullscreen, "%setfullscreen",2,0,0,(SCM c,SCM yes),"")
#define FUNC_NAME s_gwwm_setfullscreen
{
  GWWM_ASSERT_CLIENT_OR_FALSE(c ,1);
  setfullscreen(UNWRAP_CLIENT(c),scm_to_bool(yes));
  return SCM_UNSPECIFIED;
}
#undef FUNC_NAME
