#include <Arduino.h>

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <WiFiClient.h>
  #include <ESP8266WebServer.h>
#elif defined(ESP32)
  #include <WiFi.h>
  #include <WiFiClient.h>
  #include <WebServer.h>
#endif

#include <ElegantOTA.h>

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>   
#include <ArduinoJson.h>  
#include <PubSubClient.h>

#include "poolheater.h"


// TODO: migrate to Preferences
//https://unsinnsbasis.de/esp32-preferences/
//#include <Preferences.h>
//Preferences prefs;

// Set the name of the autoconnect AP
#define autoconnect_ap_name "AutoConnectAP"

// Set the filename for the config
#define config_filename   "/config.json"

// Set the name of the MQTT
#define mqtt_name         "PoolHeater"

// The time in milliseconds to force an update
#define mqtt_force_update 30000

// The IP address of the MQTT server
char mqtt_server[40] = "192.168.178.56";

// The port of the MQTT server
char mqtt_port[6] = "1883";

// The username for the MQTT server
char mqtt_user[20] = "";

// The password for the MQTT server
char mqtt_pass[20] = "";

WiFiClient espClient;
PubSubClient client(espClient);
poolheater heater;
ESP8266WebServer *otaserver = NULL;


// Set up custom parameters for MQTT server, port, user, and password
WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
WiFiManagerParameter custom_mqtt_user("user", "mqtt user", mqtt_user, 20);
WiFiManagerParameter custom_mqtt_pass("pass", "mqtt pass", mqtt_pass, 20);

// Add custom parameters to WiFiManager
WiFiManager wifiManager;


bool rawmode = false;

// stores the last received DD frame (temperatures)
byte tempframe[10];   // DD
// received time in seconds
unsigned long tempframetime = 0;

// stores the last received D2 frame (ctrl)
byte ctrlframe[10];   // D2
// received time in seconds
unsigned long ctrlframetime = 0;

unsigned long lastmqttupdate = 0;

unsigned long timestampoffset = 0;

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}


/**
 * Checks if the MQTT update should be forced based on the time interval.
 *
 * @return true if MQTT update should be forced, false otherwise.
 */
bool checkForceMqttUpdate(){
    // Check if the time elapsed since last MQTT update is greater than the MQTT force update interval
    if(millis() - lastmqttupdate > mqtt_force_update){
        // Update the last MQTT update time to the current time
        lastmqttupdate = millis();
        // Return true to indicate that MQTT update should be forced
        return true;
    }
    // Return false to indicate that MQTT update should not be forced
    return false;
}

//#####################################################################################################

/**
 * Saves the current configuration to a file.
 * The configuration includes the MQTT server, port, username, and password.
 * The configuration file is stored in SPIFFS with the filename provided by config_filename.
 * If the file does not exist, it will be created. If it does exist, it will be overwritten.
 * 
 * @param None
 * @return None
 */
void saveConfiguration(){    
    // Print a message to indicate that the configuration is being saved.
    Serial.println("saving config");

    // Create a new JSON object with a capacity of 1024 bytes.
    DynamicJsonDocument json(1024);    

    // Add MQTT server, port, username, and password to the JSON object.
    json["mqtt_server"] = mqtt_server;
    json["mqtt_port"] = mqtt_port;
    json["mqtt_user"] = mqtt_user;
    json["mqtt_pass"] = mqtt_pass;

    // Open the configuration file with the specified filename in write mode.
    File configFile = SPIFFS.open(config_filename, "w");

    // Check if the file was opened successfully.
    if (!configFile) {
      // If the file cannot be opened, print an error message.
      Serial.println("failed to open config file for writing");
    }else{
      // If the file was opened successfully, serialize the JSON object and write it to the file.
      if (serializeJson(json, configFile) == 0) {
        // If the serialization fails, print an error message.
        Serial.println(F("Failed to write to file"));
      }
      // Close the file.
      configFile.close();  
    }
}


/**
 * loads configuration settings from a JSON file in the SPIFFS filesystem
 */
void loadConfiguration(){
  //read configuration from FS json
  Serial.println("mounting FS...");

  // check if SPIFFS filesystem was successfully mounted
  if (SPIFFS.begin()) {
    Serial.println("mounted file system");

    // check if the configuration file exists
    if (SPIFFS.exists(config_filename)) {
      //file exists, reading and loading
      Serial.println("reading config file");

      // open the configuration file
      File configFile = SPIFFS.open(config_filename, "r");

      // check if the file was successfully opened
      if (configFile) {
        Serial.println("opened config file");

        // get the file size and allocate a buffer to store its contents
        size_t size = configFile.size();
        std::unique_ptr<char[]> buf(new char[size]);

        // read the file contents into the buffer
        configFile.readBytes(buf.get(), size);

        // parse the JSON data from the buffer
        DynamicJsonDocument json(1024);
        DeserializationError err = deserializeJson(json,buf.get());

        // check if there was an error parsing the JSON data
        if (err) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(err.f_str());
            return;
        }                    

        // copy the configuration settings from the JSON data to global variables
        strcpy(mqtt_server, json["mqtt_server"]);
        strcpy(mqtt_port, json["mqtt_port"]);
        strcpy(mqtt_user, json["mqtt_user"]);
        strcpy(mqtt_pass, json["mqtt_pass"]);
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
}



//#####################################################################################################

byte sendframe[10];
/**
 * Sends a control frame to the heater.
 * @param tag The tag of the control frame.
 * @param value The value of the control frame.
 **/
void sendFrame(u8 tag, u8 value){
  // Check if the last control message is too old
  if(ctrlframetime + 10 < millis()/1000){
    return;
  }

  // Copy the control frame to the send frame
  memcpy(sendframe,ctrlframe, framesize);

  switch (tag)
  {
  case 0:
    // Set on/off value
    sendframe[7] = (sendframe[7] & 0xBF) | (value>0?0x40:0);
    break;


  case 1:
    // set mode
    /*
      00 01 02 03 04 05 06 07 08 09
      d2 0c 28 2d 07 0d a0 4c 9c fd    auto on
      d2 0c 28 2d 07 0d a0 4c 1c 7d    cool on
      d2 0c 28 2d 07 0d a0 6c 1c 9d    heat on
      d2 0c 28 2d 07 0d a0 2c 1c 5d    heat off

      
                           XX XX
    */


    if(value==1){
      // Auto mode
      sendframe[7] = sendframe[7] & 0xdf;
      sendframe[8] = sendframe[8] | 0x80;
    }
    else if(value==2){
      // Cool mode  
      sendframe[7] = sendframe[7] & 0xdf;
      sendframe[8] = sendframe[8] & 0x7f;
      
    }
    else if(value==3){
      // Heat mode
      sendframe[7] = sendframe[7] | 0x20;
      sendframe[8] = sendframe[8] & 0x7f;
    }else
    {
      return;
    }
    break;  
  case 2:
    // Set target temperature
    sendframe[8] = (sendframe[8]&0x80)|(value & 0x7F);
    break;
  default:
    return;
  }

  sendframe[0] = 0xcc; 
  if(rawmode){
    client.publish("PoolHeater/raw/lastsend", byteArrayToHexString(sendframe,framesize).c_str());
  }
  // Send the frame to the heater
  int result = heater.sendFrame(sendframe, 4000);   

  if(rawmode){
    client.publish("PoolHeater/raw/result",result==1?"success":"fail");
  }   

  if(!result){
    // Handle send error
    Serial.println("send error: timeout");
  } 
}

/**
 * Callback function for handling MQTT messages
 * @param topic - the MQTT topic the message was received on
 * @param payload - the message payload
 * @param length - the length of the message payload
 */
void mqtt_callback(char* topic, byte* payload, unsigned int length) {

  if(length==0){
    return;
  }

  // Convert payload to string
  payload[length] = '\0';
  String spayload = String((char*) payload);

  // Print MQTT message arrival information
  Serial.print("mqtt message arrived on topic:");
  Serial.print(topic);

  // Print message payload
  Serial.print(" message: ");
  Serial.println(spayload);    

  // Handle message based on topic
  if(strcmp("PoolHeater/raw/send",topic)==0){
    // Convert payload to byte array
    if(hexStringToByteArray((char*)payload,length,sendframe,10)==10){    
      
      client.publish("PoolHeater/raw/lastsend", byteArrayToHexString(sendframe,framesize).c_str());
      // Send frame and publish result
      int result = heater.sendFrame(sendframe, 4000);
      client.publish("PoolHeater/raw/result",result==1?"success":"fail");

      // Log send error if there was a timeout
      if(!result){
        Serial.println("send error: timeout");
      }      
    }
  }
  else if(strcmp("PoolHeater/set/rawmode",topic)==0){
    // Set raw mode based on message payload
    bool on = spayload.toInt()!=0;
    rawmode = on;
  }
  else if(strcmp("PoolHeater/set/power",topic)==0){
    // Set power state based on message payload
    bool on = spayload.toInt()!=0;
    sendFrame(0, on);
  }
  else if(strcmp("PoolHeater/set/mode",topic)==0){
    // Set mode based on message payload
    payload[length] = '\0';
    int mode = 0;
    if (strcmp((char*) payload, "auto") == 0)
    {
        mode = 1;
    }
    else if (strcmp((char*) payload, "cool") == 0)
    {
        mode = 2;
    }
    else if (strcmp((char*) payload, "heat") == 0)
    {
        mode = 3;
    }            
    sendFrame(1, mode);    
  }  
  else if(strcmp("PoolHeater/set/target",topic)==0){
      // Set target temperature based on message payload      
      int temp = spayload.toInt();
      if (temp >= 15 && temp <= 33)
      {
        sendFrame(2, temp);          
      }    
  }
  else if(strcmp("PoolHeater/set/timestamp",topic)==0){
      timestampoffset = spayload.toInt() - (millis()/1000);
  }
}


void checkButton(){
  if(digitalRead(D5)==LOW){
      delay(3000); // reset delay hold
      if(digitalRead(D5)==LOW){
        Serial.println("reset config and reboot...");
        wifiManager.resetSettings();
        delay(2000);
        ESP.restart(); 
      }

      if (otaserver!=NULL) {
        otaserver->stop();
        otaserver = NULL;
      }

      Serial.println("Starting config portal");
      wifiManager.setConfigPortalTimeout(120);
      
      if (!wifiManager.startConfigPortal(autoconnect_ap_name)) {
        Serial.println("failed to connect or hit timeout");
        delay(3000);
        // ESP.restart();
      } else {
        //if you get here you have connected to the WiFi
        Serial.println("connected...yeey :)");
        // Read updated MQTT parameters
        strcpy(mqtt_server, custom_mqtt_server.getValue());
        strcpy(mqtt_port, custom_mqtt_port.getValue());
        strcpy(mqtt_user, custom_mqtt_user.getValue());
        strcpy(mqtt_pass, custom_mqtt_pass.getValue());

        // Save configuration if needed
        if (shouldSaveConfig) {
          saveConfiguration();
        }  
      }        
  }
}

//#####################################################################################################
/**
 * Initializes the WiFiManager and sets up custom parameters for MQTT server, port, user, and password.
 * If the D5 pin is pulled LOW, the WiFiManager settings will be reset and the ESP will reboot.
 * If WiFi connection fails, the ESP will reset.
 * If configuration needs to be saved, it will be saved.
 * Initializes an OTAServer and an HTTP server.
 */
void initWiFiManager() {
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_user);
  wifiManager.addParameter(&custom_mqtt_pass);

  // Check if D5 pin is pulled LOW, and reset WiFiManager settings and reboot ESP if it is
  pinMode(D5, INPUT_PULLUP);
  checkButton();

  // Attempt to auto-connect to WiFi, reset ESP if connection fails
  if (!wifiManager.autoConnect(autoconnect_ap_name)) {
    Serial.println("failed to connect and hit timeout");    
    delay(3000);    
    ESP.reset();    
  }

  // Read updated MQTT parameters
  strcpy(mqtt_server, custom_mqtt_server.getValue());
  strcpy(mqtt_port, custom_mqtt_port.getValue());
  strcpy(mqtt_user, custom_mqtt_user.getValue());
  strcpy(mqtt_pass, custom_mqtt_pass.getValue());

  // Save configuration if needed
  if (shouldSaveConfig) {
    saveConfiguration();
  }  

  // Initialize an OTAServer and HTTP server
  otaserver = new ESP8266WebServer(80);
  ElegantOTA.begin(otaserver);    // Start ElegantOTA
  otaserver->begin();
  Serial.println("HTTP server started");
}



/**
 * @brief Initializes the MQTT client with the specified server and port
 * 
 */
void initMQTT() {
  // Set the MQTT server and port
  client.setServer(mqtt_server,atoi(mqtt_port));

  // Set the MQTT callback
  client.setCallback(mqtt_callback);
}



//#####################################################################################################

/**
 * This function is responsible for reconnecting the MQTT client to the broker
 * in case the connection is lost
 */
void reconnectMQTT() {  
  if (!client.connected()) { // Keep trying to connect until connection is established
    Serial.print("Attempting MQTT connection...");
    if((strlen(mqtt_user) > 0 && strlen(mqtt_pass) > 0 && client.connect(mqtt_name, mqtt_user, mqtt_pass))
        || client.connect(mqtt_name)){ // Try to connect with username and password, if provided
      Serial.println("connected");

      // Subscribe to relevant MQTT topics
      client.subscribe("PoolHeater/raw/send");
      client.subscribe("PoolHeater/set/timestamp");
      client.subscribe("PoolHeater/set/power");
      client.subscribe("PoolHeater/set/mode");
      client.subscribe("PoolHeater/set/target");
      client.subscribe("PoolHeater/set/rawmode");

    }else {
      Serial.print("failed, rc=");
      Serial.print(client.state()); // Print the error code returned by the connection attempt
      Serial.println(" try again in 5 seconds");    
      delay(5000); // Wait for 5 seconds before trying again
    }
  }
}


//#####################################################################################################

/**
 * Initializes the system and sets up all necessary components.
 */
void setup() {
  // Start serial communication at 115200 baud rate
  Serial.begin(115200);

  // Load the configuration for the system
  loadConfiguration();

  // Initialize WiFi manager to manage WiFi connectivity
  initWiFiManager();

  // Initialize MQTT client to communicate with server
  initMQTT();

  // Enable receiver for heater
  heater.enableRx(true);

  // Set pin D6 as output and set it high
  pinMode(D6, OUTPUT);
  digitalWrite(D6, 1);  
}


//#####################################################################################################
/**
 * Publishes a JSON message to an MQTT topic with information about the current state of the pool heater.
 */
void publishMQTT(){
  // create a new JSON document with a fixed size of 128 bytes
  StaticJsonDocument<128> doc;

  // create a string to hold the serialized JSON output
  String output;

  // set the properties of the JSON document
  doc["power"] = (ctrlframe[7] & 0x40) == 0x40;
  
  if((ctrlframe[8] & 0x80) == 0x80){
    doc["mode"] = "auto";
  }
  else if((ctrlframe[7]&0x20) == 0 ){
    doc["mode"] = "cool";
  }else{
    doc["mode"] = "heat";
  }
  
  doc["temp_in"] = tempframe[1];
  doc["temp_out"] = tempframe[2];
  doc["temp_ambient"] = tempframe[5];
  doc["errorcode"] = tempframe[7];
  doc["temp_target"] = ctrlframe[8] & 0x7F;
  doc["wifi_rssi"] = WiFi.RSSI();
  doc["timestamp"] = timestampoffset + (millis()/1000);

  // serialize the JSON document to a string
  serializeJson(doc, output);

  // publish the JSON message to the specified MQTT topic
  client.publish("PoolHeater/get/current",  output.c_str(), false);
}


// Main loop function
void loop() {
  byte frame[10]; // = {0xcc,0x0c,0x1c,0x2d,0x07,0x0d,0xa0,0x4c,0x9d,0xf2};  

  checkButton();

  // Check if MQTT client is not connected and reconnect if necessary
  if (!client.connected()) {
    reconnectMQTT();
    return;
  }

  // Handle incoming MQTT messages
  client.loop();

  // Handle incoming heater frames
  while (heater.receiveFrame(frame)) {
    // Publish raw frame data to MQTT if rawmode is enabled
    if (rawmode) {
      client.publish("PoolHeater/raw/recv", byteArrayToHexString(frame, sizeof(frame)).c_str(), false);
      client.publish("PoolHeater/raw/lastrecvtime", String(heater.lastframerecvtime).c_str(), false);
    }

    // Handle different types of incoming frames
    switch (frame[0]) {
      // Temperature frame
      case 0xdd:
        // Check if incoming frame is different from the last received frame
        if (memcmp(tempframe, frame, framesize) != 0 || checkForceMqttUpdate()) {
          // Copy incoming frame to tempframe
          memcpy(tempframe, frame, framesize);

          // Check if it has been at least 10 seconds since last control frame was published
          if (ctrlframetime + 10 > millis() / 1000) {
            // Publish control frame to MQTT
            publishMQTT();
          }
        }

        // Update tempframetime to current time
        tempframetime = millis() / 1000;
        break;

      // Control frame
      case 0xd2:
        // Check if incoming frame is different from the last received frame
        if (memcmp(ctrlframe, frame, framesize) != 0 || checkForceMqttUpdate()) {
          // Copy incoming frame to ctrlframe
          memcpy(ctrlframe, frame, framesize);

          // Check if it has been at least 10 seconds since last temperature frame was published
          if (tempframetime + 10 > millis() / 1000) {
            // Publish temperature frame to MQTT
            publishMQTT();
          }
        }

        // Update ctrlframetime to current time
        ctrlframetime = millis() / 1000;
        break;
    }
  }

  // Handle OTA updates if OTA server is enabled
  if (otaserver!=NULL) {
    otaserver->handleClient();
  }
}