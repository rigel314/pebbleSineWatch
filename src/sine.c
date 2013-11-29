#include <pebble.h>

Window* window;
Layer* wave;

void wave_update_callback(Layer *me, GContext* ctx)
{
	static struct tm* Ctime;
	time_t unix_now;
	
	GPoint p0 = GPoint(144/2,167);
	GPoint p1;

	unix_now = time(NULL);
	Ctime = localtime(&unix_now);

	graphics_context_set_stroke_color(ctx, GColorBlack);

	for(int i=0;i<129;i++)
	{// I use 129 pixels here because 128 divides perfectly into the number of samples in the sine lookup table.  The 129th data point is to have the same value as the 0th on the half hour.
		p1 = GPoint(144/2+sin_lookup(i*(256*(Ctime->tm_hour%12) + 4.27*Ctime->tm_min))/1000,168-i-1);
		//p1 = GPoint(144/2+sin_lookup(i*(256*(0) + 4.27*Ctime->tm_min))/1000,168-i-1);
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

void handle_minute_tick(struct tm *now, TimeUnits units_changed)
{
	layer_mark_dirty(wave);
}

void init(void)
{
	window = window_create();
	window_stack_push(window, true);

	wave = layer_create(GRect(0,0,144,168));
	layer_set_update_proc(wave, &wave_update_callback);

	layer_add_child(window_get_root_layer(window), wave);
	
	tick_timer_service_subscribe(MINUTE_UNIT, &handle_minute_tick);

	handle_minute_tick(NULL, MINUTE_UNIT);
}

void deinit(void)
{
	layer_destroy(wave);
	window_destroy(window);
}

int main(void)
{
	init();

	app_event_loop();
	
	deinit();
}
