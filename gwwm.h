#ifndef GWWM_H
#define GWWM_H
#include "client.h"
#include "util.h"
#include <X11/Xlib.h>
#include <libguile.h>
#include <stdbool.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_output_management_v1.h>
#include <wlr/xwayland.h>
#include <xkbcommon/xkbcommon.h>
/* macros */
#define MAX(a,b)               \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
#define MIN(a,b)               \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })
#define CLEANMASK(mask) (mask & ~WLR_MODIFIER_CAPS)
#define LENGTH(X) (sizeof X / sizeof X[0])
#define END(A) ((A) + LENGTH(A))
#define TAGMASK ((1 << LENGTH(tags)) - 1)
#define LISTEN(E, L, H) wl_signal_add((E), ((L)->notify = (H), (L)))
typedef struct Client Client;
typedef struct Monitor Monitor;
#ifdef XWAYLAND
enum {
  NetWMWindowTypeDialog,
  NetWMWindowTypeSplash,
  NetWMWindowTypeToolbar,
  NetWMWindowTypeUtility,
  NetLast
}; /* EWMH atoms */
#endif

typedef struct {
  uint32_t singular_anchor;
  uint32_t anchor_triplet;
  int *positive_axis;
  int *negative_axis;
  int margin;
} Edge;

#define WRAP_MONITOR(o) find_monitor(o)
#define UNWRAP_MONITOR(o)                                                      \
  (struct Monitor *)(TO_P(MAKE_P(scm_call_1(REFP("gwwm monitor", ".data"), o))))
#define MONITOR_WLR_OUTPUT(m)                                                  \
  (struct wlr_output *)(UNWRAP_WLR_OUTPUT(scm_call_1(                          \
      REFP("gwwm monitor", "monitor-wlr-output"), (WRAP_MONITOR(m)))))
#define MONITOR_LAYOUTS(m)                                                     \
  (REF_CALL_1("gwwm monitor", "monitor-layouts", (WRAP_MONITOR(m))))
#define MONITOR_SELLT(m)                                                       \
  scm_to_int((                                                                 \
      (scm_call_1(REFP("gwwm monitor", "monitor-sellt"), (WRAP_MONITOR(m))))))
#define SET_MONITOR_WLR_OUTPUT(m, o)                                           \
  scm_call_2(REFP("gwwm monitor", "set-.wlr-output!"), (WRAP_MONITOR(m)),      \
             WRAP_WLR_OUTPUT(o))
#define SET_MONITOR_SELLT(m, o)                                                \
  scm_call_2(REFP("gwwm monitor", "set-.monitor-sellt!"), (WRAP_MONITOR(m)),   \
             scm_from_int(o))
#define MONITOR_WINDOW_AREA(m)                                                 \
  (struct wlr_box *)(UNWRAP_WLR_BOX(                                           \
      REF_CALL_1("gwwm monitor", "monitor-window-area", (WRAP_MONITOR(m)))))
#define SET_MONITOR_WINDOW_AREA(m, o)                                          \
  scm_call_2(REFP("gwwm monitor", "set-.window-area!"), (WRAP_MONITOR(m)),     \
             REF_CALL_1("oop goops","shallow-clone",WRAP_WLR_BOX(o)))
#define MONITOR_AREA(m)                                                        \
  ((struct wlr_box *)(UNWRAP_WLR_BOX(                                          \
      REF_CALL_1("gwwm monitor", "monitor-area", (WRAP_MONITOR(m))))))
#define SET_MONITOR_AREA(m, o)                                                 \
  scm_call_2(REFP("gwwm monitor", "set-.area!"), (WRAP_MONITOR(m)),            \
             WRAP_WLR_BOX(o))

typedef struct {
  const char *name;
  float mfact;
  int nmaster;
  float scale;
  enum wl_output_transform rr;
} MonitorRule;

typedef struct {
  const char *id;
  const char *title;
  unsigned int tags;
  int isfloating;
  int monitor;
} Rule;

/* function declarations */
Monitor *current_monitor();
Monitor *client_monitor(void *c, Monitor *change);
void applyexclusive(struct wlr_box *usable_area, uint32_t anchor,
                    int32_t exclusive, int32_t margin_top, int32_t margin_right,
                    int32_t margin_bottom, int32_t margin_left);
void arrange(Monitor *m);
SCM find_monitor(Monitor *m);
Monitor *dirtomon(enum wlr_direction dir);
Client *focustop(Monitor *m);
void maplayersurfacenotify(struct wl_listener *listener, void *data);
void pointerfocus(Client *c, struct wlr_surface *surface, double sx, double sy,
                  uint32_t time);
void quitsignal(int signo);
Client *current_client(void);
void setfullscreen(Client *c, int fullscreen);
void setmon(Client *c, Monitor *m, unsigned int newtags);
void sigchld(int unused);
void logout_monitor(SCM m);
void virtualkeyboard(struct wl_listener *listener, void *data);
struct wlr_scene_node *xytonode(double x, double y,
                                struct wlr_surface **psurface, Client **pc,
                                Client **pl, double *nx, double *ny);
struct wlr_seat *get_gloabl_seat(void);
SCM get_gwwm_config(void);
#define GWWM_BORDERPX()                                                        \
  (scm_to_unsigned_integer(                                                    \
      REF_CALL_1("gwwm config", "config-borderpx", gwwm_config), 0, 1000))
#define GWWM_SLOPPYFOCUS_P()                                                   \
  (scm_to_bool(REF_CALL_1("gwwm config", "config-sloppyfocus?", gwwm_config)))

#define GWWM_LOCKFULLSCREEN_P()                                                \
  (scm_to_bool(                                                                \
      REF_CALL_1("gwwm config", "config-lockfullscreen?", gwwm_config)))
#define GWWM_CURSOR_NORMAL_IMAGE()                                             \
  (scm_to_utf8_string(                                                         \
      REF_CALL_1("gwwm config", "config-cursor-normal-image", gwwm_config)))
#ifdef XWAYLAND
void xwaylandready(struct wl_listener *listener, void *data);
Atom getatom(xcb_connection_t *xc, const char *name);
/* void xwaylandready(struct wl_listener *listener, void *data); */
Atom get_netatom_n(int n);
#endif

#endif // GWWM_H
