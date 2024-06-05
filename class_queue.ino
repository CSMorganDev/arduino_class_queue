#include <driver/gpio.h>
#include <driver/rtc_io.h>
#include <WiFi.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <ArduinoJson.h>
#include "config.h"
#include "classroom.h"

#define GPIO_LED_GREEN  GPIO_NUM_18   /* Briefly flash if door is closed */
#define GPIO_LED_RED    GPIO_NUM_19   /* Briefly flash if door is open */

#define GPIO_SWITCH     GPIO_NUM_15   /* Door switch connected to this GPIO */

Classroom classroom;

int ticketnumber = 0;

WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and 
// MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// Setup a feed called‘feed’for publishing
Adafruit_MQTT_Subscribe feed = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/" AIO_FEED_SUBSCRIBE);
Adafruit_MQTT_Publish feed_u = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/" AIO_FEED_UPDATE);

void setup() {
  Serial.begin(115200);

  /* LED outputs. */
  gpio_set_direction(GPIO_LED_GREEN, GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_LED_RED, GPIO_MODE_OUTPUT);

  rtc_gpio_deinit(GPIO_SWITCH);
  const int pin_state = digitalRead(GPIO_SWITCH);
  flash_led(pin_state);
  delay(10);

  // Connect to WiFi
  wifi_connect();

  // Connect to MQTT broker
  mqtt.subscribe(&feed);
  flash_led(pin_state);

  /* Make sure to pull up the sensor input. */
  rtc_gpio_pullup_en(GPIO_SWITCH);

  classroom.printClassroomInfo();

}

void loop() {

  MQTT_connect();

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &feed) {
      Serial.print(F("Got: "));
      Serial.println((char *)feed.lastread);
      const int pin_state = digitalRead(GPIO_SWITCH);
      Serial.println(pin_state);
      String msgtype = getMessageType((char *)feed.lastread);
      if (msgtype == "add") {
        Student new_student = createStudentFromJson((char *)feed.lastread);        
        classroom.addStudentToQueue(new_student);
        publish_update();
        classroom.printClassroomInfo();
      } else if (msgtype == "cancel") {
        String student_number = getStudentNumber((char *)feed.lastread);        
        publish_cancel(student_number);
        publish_update();     
        classroom.printClassroomInfo(); 
      } else if (msgtype == "next") {
        String student_number = getStudentNumber((char *)feed.lastread);          
        publish_next(student_number);
        publish_update();
        classroom.printClassroomInfo();
      }
      //Serial.println(classroom.queue_size);
      
      //mqtt_update(pin_state);
      //flash_led(pin_state);
    }
  }

}

void flash_led(const int pin_state) {
  gpio_set_level(pin_state ? GPIO_LED_RED : GPIO_LED_GREEN, 1);
  gpio_set_level(pin_state ? GPIO_LED_GREEN : GPIO_LED_RED, 0);
  delay(50);
  gpio_set_level(GPIO_LED_RED, 0);
  gpio_set_level(GPIO_LED_GREEN, 0);
}

void wifi_connect() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1100);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.print("IP address: "); 
  Serial.println(WiFi.localIP());
}

void mqtt_update(const int pin_state) {
  MQTT_connect();
  /*
  Adafruit_MQTT_Publish feed_u = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/" AIO_FEED);

    if (!feed_u.publish((uint32_t)pin_state)) {
      Serial.println(F("Failed"));
    } else {
      Serial.println(F("OK!"));
    }
  */
  //publish_update();
  //publish_cancel("20103560");
  //publish_next("20103560");
}

void publish_update() {    
  StaticJsonDocument<1024> doc;
  JsonObject root = doc.to<JsonObject>();
  root["messageType"] = "update";
  root["currentName"] = classroom.current_name;
  root["currentQuestion"] = classroom.current_question;
  root["currentStudentNumber"] = classroom.current_student_number;
  root["currentTicketNumber"] = std::to_string(classroom.current_ticket_number);
  root["awayFromDesk"] = classroom.away_from_desk;
  JsonArray queueArray = root.createNestedArray("queue");

  for (int i = 0; i < classroom.queue_size; i++) {
    JsonObject jobj = queueArray.createNestedObject();
    jobj["name"] = classroom.queue[i].name;
    jobj["studentNumber"] = classroom.queue[i].student_number;
    jobj["question"] = classroom.queue[i].question;
    jobj["ticketNumber"] = std::to_string(classroom.queue[i].ticket_number);
  }
             
  char jsonBuffer[1024];
  size_t n = serializeJson(doc, jsonBuffer);
  Serial.println(jsonBuffer);

      
  if (!feed_u.publish(jsonBuffer)) {
      Serial.println(F("Publish udpate failed"));
  } else {
    Serial.println(F("Publish update OK!"));
  }      
  //delay(10000);
}

void publish_cancel(const String &student_number) {
  if (classroom.removeStudentByNumber(student_number)) {
      publish_feed(student_number, "cancel");
  }
  Serial.print("The current queue size is ");
  Serial.println(classroom.queue_size);
}

void publish_next(const String &student_number) {
  if (classroom.setCurrentStudentByNumber(student_number)) {
      classroom.removeStudentByNumber(student_number);
      publish_feed(student_number, "next");      
  }  
}

void publish_feed(const String &student_number, const String &msgType) {
    StaticJsonDocument<1024> doc;
  JsonObject root = doc.to<JsonObject>();
  root["messageType"] = msgType;
  root["studentNumber"] = student_number;
             
  char jsonBuffer[1024];
  size_t n = serializeJson(doc, jsonBuffer);
  Serial.println(jsonBuffer);

      
  if (!feed_u.publish(jsonBuffer)) {
      Serial.println(F("Publish failed"));
  } else {
    Serial.println(F("Publish OK!"));
  }         
}

void MQTT_connect() {
  int8_t ret;
  Serial.print(".");
  // Stop if already connected
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // Connect will return 0 for success
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0) {
      // basically die and wait for WDT to reset me
      while (1);
    }
  }
  Serial.println("MQTT Connected!");
}

String getMessageType(const String& jsonString) {
  DynamicJsonDocument doc(1024);

    // Parse the JSON string
    DeserializationError error = deserializeJson(doc, jsonString, DeserializationOption::NestingLimit(100));
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return "error" ; 
    }
    const char* msgtype = doc["messageType"];
    return msgtype;
}

String getStudentNumber(const String& jsonString) {
  DynamicJsonDocument doc(1024);

    // Parse the JSON string
    DeserializationError error = deserializeJson(doc, jsonString, DeserializationOption::NestingLimit(100));
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return "error" ; 
    }
    const char* student_number = doc["studentNumber"];
    return student_number;
}

Student createStudentFromJson(const String& jsonString) {
    // Allocate a temporary JsonDocument
    DynamicJsonDocument doc(1024);

    // Parse the JSON string
    DeserializationError error = deserializeJson(doc, jsonString);
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return Student(); // Return an empty Student object on error
    }

    // Extract values
    String name;
    String student_number;
    String question;
    int ticket_number;
    
      name = doc["student"]["name"].as<String>();
      student_number = doc["student"]["studentNumber"].as<String>();
      question = doc["student"]["question"].as<String>();
      ticketnumber++;

    // Create and return the Student object
    return Student(name, student_number, question, ticketnumber);
}

String extractStudentNumberFromJson(const String& jsonString) {
    // Allocate a temporary JsonDocument
    DynamicJsonDocument doc(1024);

    // Parse the JSON string
    DeserializationError error = deserializeJson(doc, jsonString, DeserializationOption::NestingLimit(100));
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return "0"; 
    }

    // Extract values    
    String student_number;    

    JsonArray queueArray = doc["queue"];
    for (int i = 0; i < queueArray.size(); i++) {
      JsonObject jobj = queueArray[i];    
      student_number = jobj["studentNumber"].as<String>();
    }

    return student_number;
}




