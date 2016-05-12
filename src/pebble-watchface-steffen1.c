#include <pebble.h>

#define KEY_BUTTON_UP   0
#define KEY_BUTTON_DOWN 1

static Window    *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_output_layer;
static TextLayer *s_status_layer;
static GFont      s_font_terminus_24;
static GFont      s_font_terminus_12;
static int        s_battery_level;

static void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
  time_t temp = time(NULL);

  static char s_buffer[8], date_buffer[14];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);

  text_layer_set_text(s_time_layer, s_buffer);

  strftime(date_buffer, sizeof(date_buffer), "%d.%m.%Y", tick_time);

  text_layer_set_text(s_date_layer, date_buffer);
}

static void bluetooth_callback(bool connected) {
  static char buf[60];
  snprintf(buf, sizeof(buf), "%s\nbattery: %d%%", connected ? "connected" : "disconnected", s_battery_level);
  if(!connected) {
    vibes_double_pulse();
  }
  text_layer_set_text(s_status_layer, buf);
}

static void battery_callback(BatteryChargeState state) {
  s_battery_level = state.charge_percent;
}

static void main_window_load(Window *window)
{
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);


  s_font_terminus_24 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_TERMINUS_24));
  s_font_terminus_12 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_TERMINUS_12));

  s_time_layer = text_layer_create(GRect(0, 0, bounds.size.w, 30));
  
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "Time");
  text_layer_set_font(s_time_layer, s_font_terminus_24);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  s_date_layer = text_layer_create(GRect(0, 30, bounds.size.w, 20));
  
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorBlack);
  text_layer_set_text(s_date_layer, "Date");
  text_layer_set_font(s_date_layer, s_font_terminus_12);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);

  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

  s_output_layer = text_layer_create(GRect(0, 50, bounds.size.w, 48));
  
  text_layer_set_background_color(s_output_layer, GColorClear);
  text_layer_set_text_color(s_output_layer, GColorBlack);
  text_layer_set_text(s_output_layer, "Output Layer");
  text_layer_set_font(s_output_layer, s_font_terminus_12);
  text_layer_set_text_alignment(s_output_layer, GTextAlignmentCenter);

  layer_add_child(window_layer, text_layer_get_layer(s_output_layer));

  s_status_layer = text_layer_create(GRect(0, 100, bounds.size.w, 50));
  
  text_layer_set_background_color(s_status_layer, GColorClear);
  text_layer_set_text_color(s_status_layer, GColorBlack);
  text_layer_set_text(s_status_layer, "Status Layer");
  text_layer_set_font(s_status_layer, s_font_terminus_12);
  text_layer_set_text_alignment(s_status_layer, GTextAlignmentCenter);

  layer_add_child(window_layer, text_layer_get_layer(s_status_layer));

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  battery_callback(battery_state_service_peek());
  bluetooth_callback(connection_service_peek_pebble_app_connection());
}

static void main_window_unload(Window *window)
{
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_output_layer);
}

static void send(int key, int value) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  dict_write_int(iter, key, &value, sizeof(int), true);

  app_message_outbox_send();
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_output_layer, "Up");
  send(KEY_BUTTON_UP, 0);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_output_layer, "Down");
  send(KEY_BUTTON_DOWN, 0);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void outbox_sent_handler(DictionaryIterator *iter, void *context) {
  text_layer_set_text(s_output_layer, "Press up or down.");
}

static void outbox_failed_handler(DictionaryIterator *iter, AppMessageResult reason, void *context) {
  text_layer_set_text(s_output_layer, "Send failed!");
  APP_LOG(APP_LOG_LEVEL_ERROR, "Fail reason: %d", (int)reason);
}

static void init()
{
  s_main_window = window_create();

  window_set_window_handlers(s_main_window, (WindowHandlers)
      {
        .load = main_window_load,
        .unload = main_window_unload
      });

  window_set_click_config_provider(s_main_window, click_config_provider);

  const int animated = true;
  window_stack_push(s_main_window, animated);

  app_message_register_outbox_sent(outbox_sent_handler);
  app_message_register_outbox_failed(outbox_failed_handler);

  const int inbox_size = 128;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);

  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_callback
  });

  battery_state_service_subscribe(battery_callback);
}

static void deinit()
{
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

