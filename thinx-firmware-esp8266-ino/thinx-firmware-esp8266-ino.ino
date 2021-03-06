/*
 * THiNX Example with all features
 *
 * - Set your own WiFi credentials for development purposes.
 * - When THiNX finalizes checkin, update device status over MQTT.
 * - Use the `Push Configuration` function in website's action menu to trigger pushConfigCallback() [limit 512 bytes so far]
 */

#include <Arduino.h>

#define ARDUINO_IDE

#include <THiNXLib.h>

const char *apikey = "1a32f961cafcd0a277a28822d266b135c53d31c8dc1d4b60ae05b6db58853b9e";
const char *owner_id = "cedc16bb6bb06daaa3ff6d30666d91aacd6e3efbf9abbc151b4dcade59af7c12";v

THiNX thx;

//
// Example of using injected Environment variables to change WiFi login credentials.
//

void pushConfigCallback (String config) {

  // Set MQTT status (unretained)
  thx.publishStatusUnretained("{ \"status\" : \"push configuration received\"}");

  // Convert incoming JSON string to Object
  DynamicJsonBuffer jsonBuffer(512);
  JsonObject& root = jsonBuffer.parseObject(config.c_str());
  JsonObject& configuration = root["configuration"];

  if ( !configuration.success() ) {
    Serial.println(F("Failed parsing configuration."));
  } else {

    // Parse and apply your Environment vars
    const char *ssid = configuration["THINX_ENV_SSID"];
    const char *pass = configuration["THINX_ENV_PASS"];

    // password may be empty string
    if ((strlen(ssid) > 2) && (strlen(pass) > 0)) {
      WiFi.disconnect();
      WiFi.begin(ssid, pass);
      long timeout = millis() + 20000;
      Serial.println("Attempting WiFi migration...");
      while (WiFi.status() != WL_CONNECTED) {
        if (millis() > timeout) break;
      }
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi migration failed."); // TODO: Notify using publish() to device status channel
      } else {
        Serial.println("WiFi migration successful."); // TODO: Notify using publish() to device status channel
      }
    }
  }
}

void setup() {

  Serial.begin(115200);

#ifdef __DEBUG__
  while (!Serial); // wait for debug console connection
#endif

  // If you need to inject WiFi credentials once...
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  //
  // Static pre-configuration
  //

  THiNX::accessPointName = "THiNX-AP";
  THiNX::accessPointPassword = "PASSWORD";

  //
  // Initialization
  //

  thx = THiNX(apikey, owner_id);

  //
  // App and version identification
  //

  // Override versioning with your own app before checkin
  thx.thinx_firmware_version = "ESP8266-INO-ALL-1.0.0";
  thx.thinx_firmware_version_short = "1.0.0";

  //
  // Callbacks
  //

  // Called after library gets connected and registered.
  thx.setFinalizeCallback([]{
    Serial.println("*INO: Finalize callback called.");
  thx.publishStatus("STATUS:RETAINED"); // set MQTT status (unretained)
  });

  // Called when new configuration is pushed OTA
  thx.setPushConfigCallback(pushConfigCallback);

  // Callbacks can be defined inline
  thx.setMQTTCallback([](String message) {
    Serial.println(message);
  });

}

/* Loop must call the thx.loop() in order to pickup MQTT messages and advance the state machine. */
void loop()
{
  thx.loop();
}
