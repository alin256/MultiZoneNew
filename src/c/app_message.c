#include <pebble.h>
#include "place_list.h"

static const uint16_t TOTAL_LAYERS = 5;
Window *window;	
TextLayer **text_layers;
static uint16_t curent_layer = 0;
TextLayer *s_output_layer;
  
// Key values for AppMessage Dictionary
enum {
	STATUS_KEY = 0,	
	MESSAGE_KEY = 1,
  LOCATION_KEY = 2,
  OFFSET_KEY = 3,
  PLACE_KEY = 4
};

// Write message to buffer & send
void send_message(void){
	DictionaryIterator *iter;
	
	app_message_outbox_begin(&iter);
	dict_write_uint8(iter, STATUS_KEY, 0x1);
  char* str = malloc(TIMEZONE_NAME_LENGTH);
  if (clock_is_timezone_set())
  {
    clock_get_timezone(str, TIMEZONE_NAME_LENGTH);
    text_layer_set_text(s_output_layer, str);
  }else{
    str = "Saratov";
  }
  //str = "Saratov";
  
  dict_write_cstring(iter, PLACE_KEY, str);
	
	dict_write_end(iter);
  app_message_outbox_send();
  
}

// Called when a message is received from PebbleKitJS
static void in_received_handler(DictionaryIterator *received, void *context) {
	Tuple *tuple;
	text_layer_set_text(s_output_layer, "recieved");
  
	tuple = dict_find(received, STATUS_KEY);
	if(tuple) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Received Status: %s", tuple->value->cstring); 
    text_layer_set_text(s_output_layer,  tuple->value->cstring);
	}
	
	tuple = dict_find(received, MESSAGE_KEY);
	if(tuple) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Received Message: %s", tuple->value->cstring);
    text_layer_set_text(s_output_layer, "message");
	}
  
  tuple = dict_find(received, LOCATION_KEY);
  if(tuple)
  {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Received location: %s", tuple->value->cstring);
    text_layer_set_text(s_output_layer, tuple->value->cstring);
  }
  
  tuple = dict_find(received, OFFSET_KEY);
  if(tuple)
  {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Received location: %i", (int)tuple->value->int32);
    int buf_len = 30;
    char* str = malloc(buf_len);
    int res_bytes = snprintf(str, buf_len, "%i", (int)tuple->value->int32);
    text_layer_set_text(s_output_layer, str);
  }

}

// Called when an incoming message from PebbleKitJS is dropped
static void in_dropped_handler(AppMessageResult reason, void *context) {	
}

// Called when PebbleKitJS does not acknowledge receipt of a message
static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  // Create output TextLayer
  s_output_layer = text_layer_create(GRect(5, 0, window_bounds.size.w - 5, window_bounds.size.h));
  text_layer_set_font(s_output_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text(s_output_layer, "No button pressed yet.");
  text_layer_set_overflow_mode(s_output_layer, GTextOverflowModeWordWrap);
  layer_add_child(window_layer, text_layer_get_layer(s_output_layer));
  
  int32_t shift = 30;
  text_layers = malloc(TOTAL_LAYERS*sizeof(TextLayer*));
  for (uint16_t i = 0; i<TOTAL_LAYERS; ++i){
    text_layers[i] = text_layer_create(GRect(5, i*shift, window_bounds.size.w - 5, shift));
    text_layer_set_font(text_layers[i], fonts_get_system_font(FONT_KEY_GOTHIC_24));
    text_layer_set_text(text_layers[i], "Layer");
    text_layer_set_overflow_mode(text_layers[i], GTextOverflowModeTrailingEllipsis);
    layer_add_child(window_layer, text_layer_get_layer(text_layers[i]));
  }
}


static void main_window_unload(Window *window) {
  // Destroy output TextLayer
  text_layer_destroy(s_output_layer);
  
  for (uint16_t i = 0; i<TOTAL_LAYERS; ++i){
    text_layer_destroy(text_layers[i]);
  }
  free(text_layers);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  //text_layer_set_text(s_output_layer, "Down pressed!");
  //sending message on click
  //send_message();
  while (curent_layer >= TOTAL_LAYERS){
    curent_layer -= TOTAL_LAYERS;
  }
  int16_t total_places = sizeof(places)/sizeof(places[0]);
  int16_t city_index = rand() % total_places;
  text_layer_set_text(text_layers[curent_layer], places[city_index]);
  curent_layer++;
}


static void click_config_provider(void *context) {
  // Register the ClickHandlers
  //window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  //window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

void init(void) {
	window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_set_click_config_provider(window, click_config_provider);
	window_stack_push(window, true);
	
	// Register AppMessage handlers
	app_message_register_inbox_received(in_received_handler); 
	app_message_register_inbox_dropped(in_dropped_handler); 
	app_message_register_outbox_failed(out_failed_handler);
		
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
	
	//send_message();
}

void deinit(void) {
	app_message_deregister_callbacks();
	window_destroy(window);
}

int main( void ) {
	init();
	app_event_loop();
	deinit();
}