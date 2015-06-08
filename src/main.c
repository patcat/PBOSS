#include <pebble.h>

#define COLORS       true
#define ANTIALIASING true

#define PIXEL_COUNT 48

typedef struct {
  int hours;
  int minutes;
} Time;

static Window *s_main_window;
static Layer *s_canvas_layer;
static char time_text[] = "00:00";

static int random_colors[50][50][3];

/* --------------------------------------------
 *  Cat paths
 * --------------------------------------------*/
static GPath *cat_nose_path = NULL;
static GPath *cat_inner_ear_left_path = NULL;
static GPath *cat_inner_ear_right_path = NULL;

static const GPathInfo CAT_NOSE_PATH_INFO = {
  .num_points = 3,
  .points = (GPoint []) {{0, 0}, {26, 0}, {26/2, 20}}
};

static const GPathInfo CAT_INNER_EAR_LEFT = {
  .num_points = 3,
  .points = (GPoint []) {{22, 27}, {32, 10}, {53, 23}}
};

static const GPathInfo CAT_INNER_EAR_RIGHT = {
  .num_points = 3,
  .points = (GPoint []) {{93, 22}, {112, 12}, {120, 25}}
};

/* --------------------------------------------
 *  Cat feature functions
 * --------------------------------------------*/
static void init_cat_custom_shapes() {
  cat_nose_path = gpath_create(&CAT_NOSE_PATH_INFO);

  cat_inner_ear_left_path = gpath_create(&CAT_INNER_EAR_LEFT);
  cat_inner_ear_right_path = gpath_create(&CAT_INNER_EAR_RIGHT);

  gpath_move_to(cat_nose_path, GPoint(58, 77));
}

static void draw_cat(GContext *ctx) {
  // Eyes
  graphics_context_set_fill_color(ctx, GColorVividCerulean);
  graphics_fill_circle(ctx, GPoint(42, 55), (17));
  graphics_fill_circle(ctx, GPoint(100, 55), (17));

  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_circle(ctx, GPoint(43, 55), 15);
  graphics_fill_circle(ctx, GPoint(99, 55), 15);

  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_circle(ctx, GPoint((43-9), 50), 1.5);
  graphics_fill_circle(ctx, GPoint((99-9), 50), 1.5);

  // Nose
  graphics_context_set_fill_color(ctx, GColorFolly);
  gpath_draw_filled(ctx, cat_nose_path);

  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, 2);

  graphics_context_set_antialiased(ctx, ANTIALIASING);
  
  // Ear left
  graphics_draw_line(ctx, GPoint(20, 25), GPoint(30, 7));
  graphics_draw_line(ctx, GPoint(30, 7), GPoint(51, 20));

  // Ear right
  graphics_draw_line(ctx, GPoint(91, 20), GPoint(112, 9));
  graphics_draw_line(ctx, GPoint(112, 9), GPoint(122, 25));

  graphics_context_set_stroke_color(ctx, GColorWhite);

  graphics_context_set_fill_color(ctx, GColorMelon);
  gpath_draw_filled(ctx, cat_inner_ear_left_path);
  gpath_draw_filled(ctx, cat_inner_ear_right_path);

  // Inner ear left
  graphics_draw_line(ctx, GPoint(22, 27), GPoint(32, 10));
  graphics_draw_line(ctx, GPoint(32, 10), GPoint(53, 23));

  // Inner ear right
  graphics_draw_line(ctx, GPoint(93, 22), GPoint(112, 12));
  graphics_draw_line(ctx, GPoint(112, 12), GPoint(120, 25));

  graphics_context_set_stroke_color(ctx, GColorDarkCandyAppleRed);

  // Mouth
  graphics_draw_line(ctx, GPoint(71, 95), GPoint(71, 105));
  graphics_draw_line(ctx, GPoint(71, 105), GPoint(80, 118));
  graphics_draw_line(ctx, GPoint(71, 105), GPoint(60, 118));
}

/* --------------------------------------------
 *  Keep the time and refresh each minute
 * --------------------------------------------*/
static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  strftime(time_text, sizeof(time_text), "%I:%M", tick_time);

  // Redraw
  if(s_canvas_layer) {
    layer_mark_dirty(s_canvas_layer);
  }
}

/* --------------------------------------------
 *  Draw functions
 * --------------------------------------------*/
static void draw_pixel_background(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  int i, j;
  
  int cube_size = bounds.size.w / PIXEL_COUNT;
  int cube_count_x = PIXEL_COUNT;
  int cube_count_y = bounds.size.h / cube_size;

  for (i = 0; i < cube_count_x; i++) {
    for (j = 0; j < cube_count_y; j++) {
      random_colors[i][j][0] = 255;
      random_colors[i][j][1] = rand() % 160 + 96;
      random_colors[i][j][2] = rand() % 100;

      graphics_context_set_fill_color(ctx, GColorFromRGB(random_colors[i][j][0], random_colors[i][j][1], random_colors[i][j][2]));

      int x = i*cube_size;
      int y = j*cube_size;
      graphics_fill_rect(ctx, GRect(x, y, (x+cube_size), (y+cube_size)), 0, GCornerNone);
    }
  }
}

static void draw_time(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  graphics_context_set_text_color(ctx, GColorBlack);
  graphics_draw_text(ctx, time_text, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD), GRect(2, (bounds.size.h - 48), 144, bounds.size.h), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_text(ctx, time_text, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD), GRect(0, (bounds.size.h - 50), 144, bounds.size.h), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
}

static void update_proc(Layer *layer, GContext *ctx) {
  draw_pixel_background(layer, ctx);

  draw_cat(ctx);

  draw_time(layer, ctx);
}

/* --------------------------------------------
 *  Init, load and unload functions
 * --------------------------------------------*/
static void init_clock() {
  srand(time(NULL));

  time_t t = time(NULL);
  struct tm *time_now = localtime(&t);
  tick_handler(time_now, MINUTE_UNIT);
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  init_cat_custom_shapes();

  s_canvas_layer = layer_create(window_bounds);
  layer_set_update_proc(s_canvas_layer, update_proc);
  layer_add_child(window_layer, s_canvas_layer);
}

static void window_unload(Window *window) {
  layer_destroy(s_canvas_layer);
}

static void init() {
  init_clock();

  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_main_window, true);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}
