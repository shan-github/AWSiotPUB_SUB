load('api_config.js');
load('api_mqtt.js');
load('api_sys.js');
load('api_timer.js');

let topic = 'metrics/esp';
let led      = NodeMCU.D4;  
let state    = true;        


Timer.set(2000 , Timer.REPEAT, function() {
  let msg = JSON.stringify({free: Sys.free_ram(), total: Sys.total_ram()});
  print(topic, '->', msg);
  MQTT.pub(topic, msg, 1);
  digitalWrite(led, state);
  state = !state;
}, null);