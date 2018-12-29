#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(9, 10);

// Update these with values suitable for your network.
byte mac[] = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };

// MQTT
const char *mqtt_server = "m12.cloudmqtt.com";
const int mqtt_port = 11008;
const char *mqtt_user = "getzwaca";
const char *mqtt_pass = "b_E4xS_t5WQQ";
const char *out_topic = "spring-test";
const char *in_topic = "inTopic";

const char* READY_MESSAGE = "Arduino is ready";

// MQTT Topics
const char* OUT_ALARM_STATE = "alarm/state";
const char* OUT_ALARM_FIRED = "alarm/fired";
const char* IN_ALARM_COMMAND = "alarm/command";

// Alarm States
const char* ALARM_ARMED = "1";
const char* ALARM_DISARMED = "0";
const char* RECEIVED = "10";
const char* CALLING_CENTRAL = "11";
const char* ESTABLISHED = "12";
const char* SENDING_COMMANDS = "13";
const char* WAITING = "14";

// Alarm Fired
const char* ALARM_FIRED = "2";
const char* ALARM_NOT_FIRED = "3";

// Alarm Commands
const char* ON = "1";
const char* OFF = "0";

char* alarmState = ALARM_DISARMED;
char* alarmFired = ALARM_NOT_FIRED;

// Alarm Input 
const int STATE_PIN = 2;
const int TRIGGER_PIN = 3;
const unsigned long NO_PULSE = 0 ;
const unsigned long TIMEOUT = 15000000;
unsigned long duration;

void callback(char* topic, byte* payload, unsigned int length) {
  char msg[length+1];
  byteToChar(msg, payload, length);
  Serial.print("Command:");
  Serial.print(msg);
  Serial.println("");
  if(strcmp(topic,IN_ALARM_COMMAND) == 0){ 
    Serial.print("Handling Alarm Command...");
    handleAlarmCommand(msg);
  }
}

EthernetClient ethClient;
PubSubClient client(mqtt_server, mqtt_port, callback, ethClient);

void setup()
{
   mySerial.begin(9600);   // Setting the baud rate of GSM Module 
   Serial.begin(9600);    // Setting the baud rate of Serial Monitor (Arduino)
   Ethernet.begin(mac);

   if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    for(;;)
    ;
  }

  // Get the initial alarm state.
  pinMode(STATE_PIN, INPUT);
  pinMode(TRIGGER_PIN, INPUT);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClient", mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      client.subscribe(IN_ALARM_COMMAND);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  readAlarmState();
  pushAlarmState();
  readAlarmFired();
  pushAlarmFired();
  client.loop();
  delay(1000);
}

void handleAlarmCommand(char* command) {
  publish(OUT_ALARM_STATE,RECEIVED);
  delay(2000);
  if(strcmp(command,ON) == 0){ 
     arm();
  }
  if(strcmp(command,OFF) == 0){ 
     disarm();
  }
}

void arm() {
  publish(OUT_ALARM_STATE,CALLING_CENTRAL);
  delay(5000);
  publish(OUT_ALARM_STATE,ESTABLISHED);
  delay(2000);
  publish(OUT_ALARM_STATE,SENDING_COMMANDS);
  delay(5000);
  publish(OUT_ALARM_STATE,WAITING);
  delay(5000);
}

void disarm() {
  publish(OUT_ALARM_STATE,CALLING_CENTRAL);
  delay(5000);
  publish(OUT_ALARM_STATE,ESTABLISHED);
  delay(2000);
  publish(OUT_ALARM_STATE,SENDING_COMMANDS);
  delay(5000);
  publish(OUT_ALARM_STATE,WAITING);
  delay(5000);
}

void readAlarmState() {
   duration = pulseIn(STATE_PIN, HIGH, TIMEOUT);
   if(duration == NO_PULSE) {
      alarmState = ALARM_ARMED;
   } else {
      alarmState = ALARM_DISARMED;
   }
}

void readAlarmFired() {
   if(digitalRead(TRIGGER_PIN) == HIGH) {
     alarmFired = ALARM_FIRED;
   } else {
     alarmFired = ALARM_NOT_FIRED;
   }   
}

void pushAlarmState() {
  Serial.print("Sending alarm state: ");
  Serial.print(alarmState);
  Serial.println("");
  publish(OUT_ALARM_STATE,alarmState);
}

void pushAlarmFired() {
  Serial.print("Sending alarm fired: ");
  Serial.print(alarmFired);
  Serial.println("");
  publish(OUT_ALARM_FIRED,alarmFired);
}

void publish(char* mytopic, char* mymessage) { 
  client.publish(mytopic, mymessage);
}

void byteToChar(char* msg, byte* payload, unsigned int length) {
  for (int i=0;i<length;i++) {
    msg[i] = (char)payload[i];
  }
  msg[length]='\0';  
}
