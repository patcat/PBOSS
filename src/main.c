#include <pebble.h>

#define COLORS       true
#define ANTIALIASING true

#define HAND_MARGIN  10
#define FINAL_RADIUS 55

#define ANIMATION_DURATION 500
#define ANIMATION_DELAY    600

#define PIXEL_COUNT 12

typedef struct {
  int hours;
  int minutes;
} Time;

static Window *s_main_window;
static Layer *s_canvas_layer;

static GPoint s_center;
static Time s_last_time, s_anim_time;
static int s_radius = 0, s_anim_hours_60 = 0, random_colors[20][20][3];
static bool s_animating = false;

static GPath *cat_nose_path = NULL;

static const GPathInfo CAT_NOSE_PATH_INFO = {
  .num_points = 3,
  .points = (GPoint []) {{0, 0}, {26, 0}, {26/2, 20}}
};

/*************************** AnimationImplementation **************************/

static void animation_started(Animation *anim, void *context) {
  s_animating = true;
}

static void animation_stopped(Animation *anim, bool stopped, void *context) {
  s_animating = false;
}

static void animate(int duration, int delay, AnimationImplementation *implementation, bool handlers) {
  Animation *anim = animation_create();
  animation_set_duration(anim, duration);
  animation_set_delay(anim, delay);
  animation_set_curve(anim, AnimationCurveEaseInOut);
  animation_set_implementation(anim, implementation);
  if(handlers) {
    animation_set_handlers(anim, (AnimationHandlers) {
      .started = animation_started,
      .stopped = animation_stopped
    }, NULL);
  }
  animation_schedule(anim);
}

/************************************ UI **************************************/

static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  // Store time
  s_last_time.hours = tick_time->tm_hour;
  s_last_time.hours -= (s_last_time.hours > 12) ? 12 : 0;
  s_last_time.minutes = tick_time->tm_min;

  /*for(int i = 0; i < 3; i++) {
    s_color_channels[i] = rand() % 256;
  }*/

  // Redraw
  if(s_canvas_layer) {
    layer_mark_dirty(s_canvas_layer);
  }
}

static int hours_to_minutes(int hours_out_of_12) {
  return (int)(float)(((float)hours_out_of_12 / 12.0F) * 60.0F);
}

static void update_proc(Layer *layer, GContext *ctx) {
  int i, j;
  GRect bounds = layer_get_bounds(layer);
  int cube_size = bounds.size.w / PIXEL_COUNT;

  for (i = 0; i < 20; i++) {
    for (j = 0; j < 20; j++) {
      random_colors[i][j][0] = rand() % 256;
      random_colors[i][j][1] = 0;
      random_colors[i][j][2] = 0;

      graphics_context_set_fill_color(ctx, GColorFromRGB(random_colors[i][j][0], random_colors[i][j][1], random_colors[i][j][2]));
      //graphics_context_set_fill_color(ctx, GColorFromRGB(0,0,255));
      int x = i*cube_size;
      int y = j*cube_size;
      graphics_fill_rect(ctx, GRect(x, y, (x+cube_size), (y+cube_size)), 0, GCornerNone);
    }
  }

  // Clock background
  GPoint clock_center = (GPoint) {
    .x = 100,
    .y = 50,
  };

  graphics_context_set_fill_color(ctx, GColorChromeYellow);
  //graphics_fill_rect(ctx, GRect(0, 0, (cube_size*3), (cube_size*3)), 0, GCornerNone);
  graphics_fill_circle(ctx, GPoint(42, 55), (cube_size*1.2));
  graphics_fill_circle(ctx, GPoint(100, 55), (cube_size*1.2));

  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_circle(ctx, GPoint(43, 55), (cube_size*1.1));
  graphics_fill_circle(ctx, GPoint(99, 55), (cube_size*1.1));

  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_circle(ctx, GPoint((43-cube_size*0.75), 50), (cube_size*0.1));
  graphics_fill_circle(ctx, GPoint((99-cube_size*0.75), 50), (cube_size*0.1));

  gpath_draw_filled(ctx, cat_nose_path);

  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 2);

  graphics_context_set_antialiased(ctx, ANTIALIASING);

  graphics_draw_line(ctx, GPoint(20, 25), GPoint(30, 7));
  graphics_draw_line(ctx, GPoint(30, 7), GPoint(51, 20));

  graphics_draw_line(ctx, GPoint(91, 20), GPoint(112, 9));
  graphics_draw_line(ctx, GPoint(112, 9), GPoint(122, 25));

  graphics_draw_line(ctx, GPoint(71, 97), GPoint(80, 110));
  graphics_draw_line(ctx, GPoint(80, 110), GPoint(100, 110));
  graphics_draw_line(ctx, GPoint(100, 110), GPoint(110, 97));

  graphics_draw_line(ctx, GPoint(71, 97), GPoint(62, 110));
  graphics_draw_line(ctx, GPoint(62, 110), GPoint(42, 110));
  graphics_draw_line(ctx, GPoint(42, 110), GPoint(32, 97));

  // White clockface
  //graphics_context_set_fill_color(ctx, GColorWhite);
  //graphics_fill_circle(ctx, s_center, s_radius);

  // Draw outline
  //graphics_draw_circle(ctx, s_center, s_radius);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  s_center = grect_center_point(&window_bounds);

  cat_nose_path = gpath_create(&CAT_NOSE_PATH_INFO);

  gpath_move_to(cat_nose_path, GPoint(58, 77));

  s_canvas_layer = layer_create(window_bounds);
  layer_set_update_proc(s_canvas_layer, update_proc);
  layer_add_child(window_layer, s_canvas_layer);
}

static void window_unload(Window *window) {
  layer_destroy(s_canvas_layer);
}

/*********************************** App **************************************/

static int anim_percentage(AnimationProgress dist_normalized, int max) {
  return (int)(float)(((float)dist_normalized / (float)ANIMATION_NORMALIZED_MAX) * (float)max);
}

static void radius_update(Animation *anim, AnimationProgress dist_normalized) {
  s_radius = anim_percentage(dist_normalized, FINAL_RADIUS);

  layer_mark_dirty(s_canvas_layer);
}

static void hands_update(Animation *anim, AnimationProgress dist_normalized) {
  s_anim_time.hours = anim_percentage(dist_normalized, hours_to_minutes(s_last_time.hours));
  s_anim_time.minutes = anim_percentage(dist_normalized, s_last_time.minutes);

  layer_mark_dirty(s_canvas_layer);
}

static void init() {
  srand(time(NULL));

  time_t t = time(NULL);
  struct tm *time_now = localtime(&t);
  tick_handler(time_now, MINUTE_UNIT);

  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_main_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  // Prepare animations
  AnimationImplementation radius_impl = {
    .update = radius_update
  };
  animate(ANIMATION_DURATION, ANIMATION_DELAY, &radius_impl, false);

  AnimationImplementation hands_impl = {
    .update = hands_update
  };
  animate(2 * ANIMATION_DURATION, ANIMATION_DELAY, &hands_impl, true);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}
