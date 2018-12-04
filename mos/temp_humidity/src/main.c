#include "mgos.h"
#include "mgos_mqtt.h"
#include "mgos_dht.h"
 
static struct mgos_dht *s_dht = NULL;
 
static void led_timer_cb(void *arg) {
  float t = mgos_dht_get_temp(s_dht);
  float h = mgos_dht_get_humidity(s_dht);
  LOG(LL_INFO, ("Temperature: %f, Humidity: %f",t, h));
  (void) arg;
}
 
static void net_cb(int ev, void *evd, void *arg) {
  switch (ev) {
    case MGOS_NET_EV_DISCONNECTED:
      LOG(LL_INFO, ("%s", "Net disconnected"));
      break;
    case MGOS_NET_EV_CONNECTING:
      LOG(LL_INFO, ("%s", "Net connecting..."));
      break;
    case MGOS_NET_EV_CONNECTED:
      LOG(LL_INFO, ("%s", "Net connected"));
      break;
    case MGOS_NET_EV_IP_ACQUIRED:
      LOG(LL_INFO, ("%s", "Net got IP address"));
      break;
  }
 
  (void) evd;
  (void) arg;
}
 
static void report_temperature(void *arg) {
  char topic[100], message[160];
  struct json_out out = JSON_OUT_BUF(message, sizeof(message));
   
  time_t now=time(0);
  struct tm *timeinfo = localtime(&now);
 
  snprintf(topic, sizeof(topic), "event/temp_humidity");
  json_printf(&out, "{temperature: %f, humidity: %f, device: \"%s\", timestamp: \"%02d:%02d:%02d\"}",
              (float) mgos_dht_get_temp(s_dht), 
              (float) mgos_dht_get_humidity(s_dht),
              (char *) mgos_sys_config_get_device_id(),
              (int) timeinfo->tm_hour,
              (int) timeinfo->tm_min,
              (int) timeinfo->tm_sec);
  bool res = mgos_mqtt_pub(topic, message, strlen(message), 1, false);
  LOG(LL_INFO, ("Published to MQTT: %s", res ? "yes" : "no"));
  (void) arg;
}
 
static void button_cb(int pin, void *arg) {
  float t = mgos_dht_get_temp(s_dht);
  float h = mgos_dht_get_humidity(s_dht);
  LOG(LL_INFO, ("Button presses on pin: %d", pin));
  LOG(LL_INFO, ("Temperature: %f *C Humidity: %f %%\n", t, h));
   
  report_temperature(NULL);
  (void) arg;
}
 
enum mgos_app_init_result mgos_app_init(void) {
  /* Blink built-in LED every second */
  mgos_gpio_set_mode(2, MGOS_GPIO_MODE_OUTPUT);
  mgos_set_timer(2000, MGOS_TIMER_REPEAT, led_timer_cb, NULL);
   
  /* Report temperature to AWS IoT Core every 5 mins */
  mgos_set_timer(5000, MGOS_TIMER_REPEAT, report_temperature, NULL);
 
  /* Publish to MQTT on button press */
  mgos_gpio_set_button_handler(0,
                               MGOS_GPIO_PULL_UP, MGOS_GPIO_INT_EDGE_NEG, 200,
                               button_cb, NULL);
                                
  if ((s_dht = mgos_dht_create(5, DHT11)) == NULL) {
    LOG(LL_INFO, ("Unable to initialize DHT11"));
  }
 
  /* Network connectivity events */
  mgos_event_add_group_handler(MGOS_EVENT_GRP_NET, net_cb, NULL);
 
  return MGOS_APP_INIT_SUCCESS;
}