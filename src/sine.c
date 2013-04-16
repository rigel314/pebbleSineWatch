#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"


#define MY_UUID { 0x83, 0x38, 0x3F, 0xEC, 0x6E, 0x06, 0x45, 0xE8, 0xB8, 0x52, 0x06, 0xE1, 0xF0, 0xA6, 0x27, 0xF3 }
PBL_APP_INFO(MY_UUID,
             "Sine Wave", "Computing Eureka",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_WATCH_FACE);

Window window;
Layer wave;


void wave_update_callback(Layer *me, GContext* ctx) {
  (void)me;

  PblTm Ctime;
  GPoint p0 = GPoint(144/2,167);
  GPoint p1;

  get_time(&Ctime);

  graphics_context_set_stroke_color(ctx, GColorBlack);

  for(int i=0;i<129;i++)
  {// I use 129 pixels here because 128 divides perfectly into the number of samples in the sine lookup table.  The 129th data point is to have the same value as the 0th on the half hour.
    p1 = GPoint(144/2+sin_lookup(i*(128*(Ctime.tm_hour%12+1) + 4.27*Ctime.tm_min))/1000,168-i-1);
    graphics_draw_line(ctx, p0, p1);
    p0 = p1;

    // sin_lookup(i*128*hours) gives signed numbers between -32768 and 32767. Division by 1000 makes it +/- 66 pixels, perfect for using almost the whole screen.
    // hours % 12 + 1 makes sure there is always 1-12 antinodes on screen.  I like 24 hour time, but 23 antinodes looks like crap, so 12 hour it is.
    // The minutes are less accurate since they don't fall on even samples in the lookup table.  
    // I don't know how I'd do this with only minutes, but I like retaining the accuracy this way.  The least accurate is at the end of each hour, rather that the end of each day.
    // We take the sine of an angle.  The angle is a function of i, hours, and minutes.
    //   i is the pixel we are drawing at.
    //   hours is the number of half cycles plus 1.
    //   minutes is the number of 120ths of cycles beyond that. 60ths of a half cycle.
  }
}

void handle_minute_tick(AppContextRef ctx, PebbleTickEvent* t)
{
  (void) ctx;
  (void) t;
  layer_mark_dirty(&wave);
}

void handle_init(AppContextRef ctx) {
  (void)ctx;

  window_init(&window, "Sine Wave");
  window_stack_push(&window, true);

  layer_init(&wave,window.layer.frame);
  wave.update_proc = &wave_update_callback;

  layer_add_child(&window.layer, &wave);
  
  handle_minute_tick(ctx, NULL);
}


void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .tick_info = {
      .tick_handler = &handle_minute_tick,
      .tick_units = MINUTE_UNIT
      }
  };
  app_event_loop(params, &handlers);
}
