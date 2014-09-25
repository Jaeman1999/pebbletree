#include <pebble.h>
#define ROLLOUT_KEY_DEFAULT 0
#define ROLLOUT_KEY 0
	
Window *my_window, *rollout_window;
GBitmap *def, *staged, *bulb1, *bulb2, *bulb3, *green;
BitmapLayer *def_layer, *staged_layer, *bulb1_layer, *bulb2_layer, *bulb3_layer, *green_layer;
TextLayer *text, *rollout_text, *rollout_title;
int reactiontime = 0;
int start_time = 0;
int now = 0;
int rollout = ROLLOUT_KEY;
bool countdown_flag = true;
bool timer_flag = true;
bool stopper = true;
bool value_change = true;
char string[5];
char rollout_string[5];
AppTimer *timer;

double time_count() {
	time_t seconds;
	uint16_t mill;
	time_ms(&seconds, &mill);
	//Returns the current time
	return (seconds * 1000 + mill);
}

void counter() {
	//Gets the current time
	now = time_count();
	//Start only if the tree countdown has given it permission
	if (timer_flag == true) {
		reactiontime = now - start_time - 2500 + rollout;
		timer = app_timer_register(1, counter, NULL);
	}
	return;
}

void reset() {
	layer_set_hidden(bitmap_layer_get_layer(green_layer), true);
	layer_set_hidden(bitmap_layer_get_layer(staged_layer), true);
	
	//Tells the system that the countdown has ended
	countdown_flag = true;
	layer_add_child(window_get_root_layer(my_window), text_layer_get_layer(text));
}

void light_green() {
	layer_set_hidden(bitmap_layer_get_layer(bulb3_layer), true);
	layer_set_hidden(bitmap_layer_get_layer(green_layer), false);
	timer = app_timer_register(1000, reset, NULL);
}

void light_bulb3() {
	layer_set_hidden(bitmap_layer_get_layer(bulb2_layer), true);
	layer_set_hidden(bitmap_layer_get_layer(bulb3_layer), false);
	timer = app_timer_register(500, light_green, NULL);
}

void light_bulb2() {
	//Hides the first bulb bitmap
	layer_set_hidden(bitmap_layer_get_layer(bulb1_layer), true);
	
	layer_set_hidden(bitmap_layer_get_layer(bulb2_layer), false);
	timer = app_timer_register(500, light_bulb3, NULL);
}

void light_bulb1() {
	//Makes the first bulb visible
	layer_set_hidden(bitmap_layer_get_layer(bulb1_layer), false);
	
	//Starts the next timer
	timer = app_timer_register(500, light_bulb2, NULL);
}

void start_tree(ClickRecognizerRef detector, void *data) {
	//Checks if countdown has already started
	if (countdown_flag) {
		
		layer_set_hidden(text_layer_get_layer(text), true);
		//Pastes the staged bitmap to the window
		layer_set_hidden(bitmap_layer_get_layer(staged_layer), false);
	
		//Starts the timer to call the function that will light the first bulb
		timer = app_timer_register(1000, light_bulb1, NULL);
		
		//Tells the system that it can start the reaction time loop
		timer_flag = true;
		
		//Gets the current time, which is also the start time
		start_time = time_count();
		
		//Starts the counter function
		counter();
		
		//Tells the system that the countdown has started; now it knows not to fire again until it is finished
		countdown_flag = false;
	}
}

void leave(ClickRecognizerRef detector, void *data) {
	//Tells the system to not repeat this loop
	timer_flag = false;
	
	//Displays the reaction time on the screen
	layer_set_hidden(text_layer_get_layer(text), false);
	
	//This gets complicated; basically all of this is just using logic to tell the system to display the reaction time in 0.000 format
	if (0 <= reactiontime) {
		//Converts the raw reaction time(a number) into a printable string format
		snprintf(string, 10, "0.00%d", reactiontime);
	}
	if (10 <= reactiontime) {
		snprintf(string, 10, "0.0%d", reactiontime);
	}
	if (100 <= reactiontime) {
		snprintf(string, 10, "0.%d", reactiontime);
	}
	if (1000 <= reactiontime) {
		int xyz = reactiontime / 1000;
		int xxyyz = xyz * 1000;
		int xxyz = reactiontime - xxyyz;
		if (xxyz < 1000) {
			snprintf(string, 10, "%d.%d", xyz, xxyz);
		}
		if (xxyz < 100) {
			snprintf(string, 10, "%d.0%d", xyz, xxyz);
		}
		if (xxyz < 10) {
			snprintf(string, 10, "%d.00%d", xyz, xxyz);
		}
	}
	if (reactiontime < -1000) {
		reactiontime *= -1;
		int xyz = reactiontime / 1000;
		int xxyyz = xyz * 1000;
		int xxyz = reactiontime - xxyyz;
		if (xxyz < 1000) {
			snprintf(string, 10, "-%d.%d", xyz, xxyz);
		}
		if (xxyz < 100) {
			snprintf(string, 10, "-%d.0%d", xyz, xxyz);
		}
		if (xxyz < 10) {
			snprintf(string, 10, "-%d.00%d", xyz, xxyz);
		}
	}
	if (reactiontime < -100) {
		reactiontime *= -1;
		snprintf(string, 10, "-0.%d", reactiontime);
	}
	if (reactiontime <= -10) {
		reactiontime *= -1;
		snprintf(string, 10, "-0.0%d", reactiontime);
	}
	if ((-10 < reactiontime) && (reactiontime < 0)) {
		reactiontime *= -1;
		snprintf(string, 10, "-0.00%d", reactiontime);
	}
	
	//Sets the reaction time to the text layer
	text_layer_set_text(text, string);
	
	//Changes the text font
	text_layer_set_font(text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
	
	//Pastes the text layer to the window
	layer_add_child(window_get_root_layer(my_window), text_layer_get_layer(text));
}

void rollout_up(ClickRecognizerRef detector, void *data) {
	//Adds 1 to the rollout value
	rollout += 1;
	
	//Tells the system that the rollout value is being changed
	value_change = true;
	
	//If the rollout is larger than 0.999, set it back to 0.999
	if (rollout > 1000) {
		rollout = 999;
	}
}

void rollout_down(ClickRecognizerRef detector, void *data) {
	//Subtract 1 from the rollout value
	rollout -= 1;
	
	//Tells the system that the rollout value is being changed
	value_change = true;
	
	//If the rollout is less than 0.000, set it back to 0.000
	if (rollout < 0) {
		rollout = 0;
	}
}

void exit_rollout(ClickRecognizerRef detector, void *data) {
	//Tells the system that the rollout value is no longer being changed
	value_change = false;
	
	//Stops the system from pasting the rollout after the rollout menu is closed
	stopper = true;
	
	window_destroy(rollout_window);
	text_layer_destroy(rollout_text);
	text_layer_destroy(rollout_title);
	window_stack_pop(true);
}
void lock(ClickRecognizerRef detector, void *data) {
	//This callback is solely for the purpose of preventing the user from exiting the rollout menu without saving
}

void rollout_clicks(void *data) {
	window_single_repeating_click_subscribe(BUTTON_ID_UP, 100, rollout_up);
	window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 100, rollout_down);
	window_long_click_subscribe(BUTTON_ID_SELECT, 200, exit_rollout, NULL);
	window_single_click_subscribe(BUTTON_ID_BACK, lock);
}

void paste(){
	//Only start the function if the rollout menu is open
	if (stopper == false) {
		//Restarts the function unless a button has been pressed
		if (value_change) {
			if (rollout < 1000) {
				snprintf(rollout_string, 10, "0.%d", rollout);
			}
			if (rollout < 100) {
				snprintf(rollout_string, 10, "0.0%d", rollout);
			}
			if (rollout < 10) {
				snprintf(rollout_string, 10, "0.00%d", rollout);
			}
			text_layer_set_text(rollout_text, rollout_string);
			layer_add_child(window_get_root_layer(rollout_window), text_layer_get_layer(rollout_text));
			
			value_change = false;
			
			timer = app_timer_register(1000, paste, NULL);
		}
		timer = app_timer_register(100, paste, NULL);
	}
	return;
}

void set_rollout(ClickRecognizerRef detector, void *data) {
	//Stops the system from constantly re-pasting the rollout
	stopper = false;
	
	//Stops the system from pasting the rollout unless a button is pressed
	value_change = false;
	
	rollout_window = window_create();
	rollout_text = text_layer_create(GRect(0, 30, 144, 168));
	rollout_title = text_layer_create(GRect(0, 0, 144, 20));
	text_layer_set_text(rollout_title, "The rollout is set to:");
	
	//Again, just putting the displayed text into 0.000 format
	if (rollout < 1000) {
		snprintf(rollout_string, 10, "0.%d", rollout);
	}
	if (rollout < 100) {
		snprintf(rollout_string, 10, "0.0%d", rollout);
	}
	if (rollout < 10) {
		snprintf(rollout_string, 10, "0.00%d", rollout);
	}
	
	text_layer_set_text(rollout_text, rollout_string);
	text_layer_set_text_alignment(rollout_text, GTextAlignmentCenter);
	text_layer_set_text_alignment(rollout_title, GTextAlignmentCenter);
	text_layer_set_font(rollout_text, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
	
	window_set_click_config_provider(rollout_window, rollout_clicks);
	
	//Pastes the rollout and the title to the window
	layer_add_child(window_get_root_layer(rollout_window), text_layer_get_layer(rollout_text));
	layer_add_child(window_get_root_layer(rollout_window), text_layer_get_layer(rollout_title));
	
	//Starts the paste function
	paste();
	
	//Pushes the rollout_window to the screen
	window_stack_push(rollout_window, true);
}

void click_config(void *data) {
	window_long_click_subscribe(BUTTON_ID_SELECT, 100, start_tree, leave);
	window_long_click_subscribe(BUTTON_ID_DOWN, 100, set_rollout, NULL);
}

void handle_init(void) {
	my_window = window_create();
	text = text_layer_create(GRect(84, 52, 53, 18));
	text_layer_set_text_alignment(text, GTextAlignmentCenter);
	
	//Setting the GBitmaps to their resources
	def = gbitmap_create_with_resource(RESOURCE_ID_def);
	staged = gbitmap_create_with_resource(RESOURCE_ID_staged);
	bulb1 = gbitmap_create_with_resource(RESOURCE_ID_bulb);
	bulb2 = gbitmap_create_with_resource(RESOURCE_ID_bulb);
	bulb3 = gbitmap_create_with_resource(RESOURCE_ID_bulb);
	green = gbitmap_create_with_resource(RESOURCE_ID_bulb);
	
	//Setting the bitmap layers to the variables
	def_layer = bitmap_layer_create(GRect(0, 0, 144, 154));
	staged_layer = bitmap_layer_create(GRect(0, 0, 144, 154));
	bulb1_layer = bitmap_layer_create(GRect(-25, -21, 144, 154));
	bulb2_layer = bitmap_layer_create(GRect(-25, 5, 144, 154));
	bulb3_layer = bitmap_layer_create(GRect(-25, 31, 144, 154));
	green_layer = bitmap_layer_create(GRect(-25, 56, 144, 154));
	
	//Linking the GBitmaps(the pictures) to their physical layers
	bitmap_layer_set_bitmap(def_layer, def);
	bitmap_layer_set_bitmap(staged_layer, staged);
	bitmap_layer_set_bitmap(bulb1_layer, bulb1);
	bitmap_layer_set_bitmap(bulb2_layer, bulb2);
	bitmap_layer_set_bitmap(bulb3_layer, bulb3);
	bitmap_layer_set_bitmap(green_layer, green);
	
	//Setting the allignment of the bitmap layers
	bitmap_layer_set_alignment(def_layer, GAlignBottom);
	bitmap_layer_set_alignment(staged_layer, GAlignBottom);
	bitmap_layer_set_alignment(bulb1_layer, GAlignCenter);
	bitmap_layer_set_alignment(bulb2_layer, GAlignCenter);
	bitmap_layer_set_alignment(bulb3_layer, GAlignCenter);
	bitmap_layer_set_alignment(green_layer, GAlignCenter);
	
	//Pasting the def bitmap and the text layer onto the window
	layer_add_child(window_get_root_layer(my_window), bitmap_layer_get_layer(def_layer));
	layer_add_child(window_get_root_layer(my_window), text_layer_get_layer(text));
	
	//Puts all the bulbs on the screen, and then hides them
	layer_add_child(window_get_root_layer(my_window), bitmap_layer_get_layer(staged_layer));
	layer_set_hidden(bitmap_layer_get_layer(staged_layer), true);
	layer_add_child(window_get_root_layer(my_window), bitmap_layer_get_layer(bulb1_layer));
	layer_set_hidden(bitmap_layer_get_layer(bulb1_layer), true);
	layer_add_child(window_get_root_layer(my_window), bitmap_layer_get_layer(bulb2_layer));
	layer_set_hidden(bitmap_layer_get_layer(bulb2_layer), true);
	layer_add_child(window_get_root_layer(my_window), bitmap_layer_get_layer(bulb3_layer));
	layer_set_hidden(bitmap_layer_get_layer(bulb3_layer), true);
	layer_add_child(window_get_root_layer(my_window), bitmap_layer_get_layer(green_layer));
	layer_set_hidden(bitmap_layer_get_layer(green_layer), true);

	//Checks if the ROLLOUT_KEY value exists; if it does, then the rollout is set to whatever the value is; if it doesn't, then the rollout is set to the ROLLOUT_KEY_DEFAULT value
	rollout = persist_exists(ROLLOUT_KEY) ? persist_read_int(ROLLOUT_KEY) : ROLLOUT_KEY_DEFAULT;
	
	window_set_click_config_provider(my_window, click_config);
	
	//Pushes the window to the screen
	window_stack_push(my_window, true);
}

void handle_deinit(void) {
	//Deletes the bitmaps
	gbitmap_destroy(def);
	gbitmap_destroy(staged);
	gbitmap_destroy(bulb1);
	gbitmap_destroy(bulb2);
	gbitmap_destroy(bulb3);
	gbitmap_destroy(green);
	
	//Deletes the bitmap layers
	bitmap_layer_destroy(def_layer);
	bitmap_layer_destroy(staged_layer);
	bitmap_layer_destroy(bulb1_layer);
	bitmap_layer_destroy(bulb2_layer);
	bitmap_layer_destroy(bulb3_layer);
	bitmap_layer_destroy(green_layer);

	//Deletes the window
	window_destroy(my_window);
	
	//Deletes the text boxes
	text_layer_destroy(text);
	
	//Writes the current value to the persistent storage value so the app remembers it
	persist_write_int(ROLLOUT_KEY, rollout);
}

int main(void) {
	handle_init();
	app_event_loop();
	handle_deinit();
}