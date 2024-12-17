#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>  // Library required for MQTT
#include <LiquidCrystal_I2C.h>


char ssid[] = "iPhone de Elián";
char pass[] = "eliancasa3";
// MQTT broker server. Note that the specific port is defined depending on the type of the
// used connection.
const char* server = "mqtt3.thingspeak.com";

// A macro is used to select between secure and nonsecure connection as it is hardware-dependent
// Certain IoT hardware platforms do not work with the WiFiClientSecure library.

//#define USESECUREMQTT             // Comment this line if nonsecure connection is used
#ifdef USESECUREMQTT
#include <WiFiClientSecure.h>
#define mqttPort 8883
WiFiClientSecure client;
#else
#include <WiFi.h>
#define mqttPort 1883
WiFiClient client;
#endif
//Credenciales únicas del esp para mqtt
const char mqttUserName[] = "KxkBLyEhARIuJzYTKhwNNR0";
const char clientID[] = "KxkBLyEhARIuJzYTKhwNNR0";
const char mqttPass[] = "RgxhdX0/bc3OrGbbMoci044U";
//Id canales
#define channelID 2681698   // Canal 1
#define channelID2 2733787  // Canal 2

// Since the target esp32-based board support secure connections, a thingspeak certificate is used.

const char* PROGMEM thingspeak_ca_cert =
  "-----BEGIN CERTIFICATE-----\n"
  "MIIDxTCCAq2gAwIBAgIQAqxcJmoLQJuPC3nyrkYldzANBgkqhkiG9w0BAQUFADBs\n"
  "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"
  "d3cuZGlnaWNlcnQuY29tMSswKQYDVQQDEyJEaWdpQ2VydCBIaWdoIEFzc3VyYW5j\n"
  "ZSBFViBSb290IENBMB4XDTA2MTExMDAwMDAwMFoXDTMxMTExMDAwMDAwMFowbDEL\n"
  "MAkGA1UEBhMCVVMxFTATBgNVBAoTDERpZ2lDZXJ0IEluYzEZMBcGA1UECxMQd3d3\n"
  "LmRpZ2ljZXJ0LmNvbTErMCkGA1UEAxMiRGlnaUNlcnQgSGlnaCBBc3N1cmFuY2Ug\n"
  "RVYgUm9vdCBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMbM5XPm\n"
  "+9S75S0tMqbf5YE/yc0lSbZxKsPVlDRnogocsF9ppkCxxLeyj9CYpKlBWTrT3JTW\n"
  "PNt0OKRKzE0lgvdKpVMSOO7zSW1xkX5jtqumX8OkhPhPYlG++MXs2ziS4wblCJEM\n"
  "xChBVfvLWokVfnHoNb9Ncgk9vjo4UFt3MRuNs8ckRZqnrG0AFFoEt7oT61EKmEFB\n"
  "Ik5lYYeBQVCmeVyJ3hlKV9Uu5l0cUyx+mM0aBhakaHPQNAQTXKFx01p8VdteZOE3\n"
  "hzBWBOURtCmAEvF5OYiiAhF8J2a3iLd48soKqDirCmTCv2ZdlYTBoSUeh10aUAsg\n"
  "EsxBu24LUTi4S8sCAwEAAaNjMGEwDgYDVR0PAQH/BAQDAgGGMA8GA1UdEwEB/wQF\n"
  "MAMBAf8wHQYDVR0OBBYEFLE+w2kD+L9HAdSYJhoIAu9jZCvDMB8GA1UdIwQYMBaA\n"
  "FLE+w2kD+L9HAdSYJhoIAu9jZCvDMA0GCSqGSIb3DQEBBQUAA4IBAQAcGgaX3Nec\n"
  "nzyIZgYIVyHbIUf4KmeqvxgydkAQV8GK83rZEWWONfqe/EW1ntlMMUu4kehDLI6z\n"
  "eM7b41N5cdblIZQB2lWHmiRk9opmzN6cN82oNLFpmyPInngiK3BD41VHMWEZ71jF\n"
  "hS9OMPagMRYjyOfiZRYzy78aG6A9+MpeizGLYAiJLQwGXFK3xPkKmNEVX58Svnw2\n"
  "Yzi9RKR/5CYrCsSXaQ3pjOLAEFe4yHYSkVXySGnYvCoCWw9E1CAx2/S6cCZdkGCe\n"
  "vEsXCS+0yx5DaMkHJ8HSXPfqIbloEpw8nL+e/IBcm2PN7EeqJSdnoDfzAIJ9VNep\n"
  "+OkuE6N36B9K\n"
  "-----END CERTIFICATE-----\n";


// Initial state of the wifi connection
int status = WL_IDLE_STATUS;

// The MQTT client is liked to the wifi connection
PubSubClient mqttClient(client);

// Variables are defined to control the timing of connections and to define the
// update rate of sensor readings (in milliseconds)

int connectionDelay = 4;     // Delay (s) between trials to connect to WiFi
long lastPublishMillis = 0;  // To hold the value of last call of the millis() function
int updateInterval = 15;     // Sensor readings are published every 15 seconds o so.

// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;
const int buzzer = 14;   // Buzzer pin
const int alarmLed = 4;  // Leds de alarma pin
// set LCD address, number of columns and rows
// if you don't know your display address, run an I2C scanner sketch
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);
bool StateNodo1 = 0;
bool StateNodo2 = 0;
int field1Value = 0;
/*******************************************************************************************************
* Function prototypes mainly grouped by funcionality and dependency
********************************************************************************************************/

// Function to connect to WiFi.
void connectWifi();

// Function to connect to MQTT server, i.e., mqtt3.thingspeak.com
void mqttConnect();

// Function to subscribe to ThingSpeak channel for updates.
void mqttSubscribe(long subChannelID);

// Function to handle messages from MQTT subscription to the ThingSpeak broker.
void mqttSubscriptionCallback(char* topic, byte* payload, unsigned int length);

// Function to publish messages to a ThingSpeak channel.
void mqttPublish(long pubChannelID, String message);


void setup() {
  // Inicializar LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(3, 0);
  lcd.print("Iniciando");
  lcd.setCursor(4, 1);
  lcd.print("sistema");

  pinMode(buzzer, OUTPUT);
  pinMode(alarmLed, OUTPUT);
  Serial.begin(115200);
  delay(3000);
  // Connect to the specified Wi-Fi network.
  connectWifi();
  // Configure the MQTT client to connect with ThinkSpeak broker
  mqttClient.setServer(server, mqttPort);
  // Set the MQTT message handler function.
  mqttClient.setCallback(mqttSubscriptionCallback);
  // Set the buffer to handle the returned JSON.
  // NOTE: A buffer overflow of the message buffer will result in your callback not being invoked.
  mqttClient.setBufferSize(2048);
// Use secure MQTT connections if defined.
#ifdef USESECUREMQTT
  // Handle functionality differences of WiFiClientSecure library for different boards.
  // Herein we are dealing with esp32-based IoT boards.
  client.setCACert(thingspeak_ca_cert);
#endif
}


void loop() {
  // After everythins is set up, go the perception-action loop
  // Reconnect to WiFi if it gets disconnected.
  if (WiFi.status() != WL_CONNECTED) {
    connectWifi();
  }
  // Connect if MQTT client is not connected and resubscribe to channel updates.
  // ThinkSpeak broaker server : suscribe to the specified channel
  if (!mqttClient.connected()) {
    mqttConnect();
    mqttSubscribe(channelID);
    mqttSubscribe2(channelID2);
  }
  // Call the loop to maintain connection to the server.
  mqttClient.loop();
}
// Function to connect to WiFi.
void connectWifi() {
  Serial.println("Connecting to Wi-Fi...");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting to");
  lcd.setCursor(0, 1);
  lcd.print("Wi-Fi...");

  // Loop until WiFi connection is successful
  while (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, pass);
    delay(connectionDelay * 1000);
    Serial.println(WiFi.status());
  }
  Serial.println("Connected to Wi-Fi.");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connected ");
  lcd.setCursor(0, 1);
  lcd.print("to Wi-Fi");
  digitalWrite(alarmLed, HIGH);
  delay(100);
  digitalWrite(alarmLed, LOW);
  delay(50);
  digitalWrite(buzzer, HIGH);
  delay(1000);
  digitalWrite(buzzer, LOW);
  delay(500);
  digitalWrite(alarmLed, HIGH);
  delay(100);
  digitalWrite(alarmLed, LOW);
}

// Function to connect to MQTT server.
void mqttConnect() {
  // Loop until the client is connected to the server.
  while (!mqttClient.connected()) {
    // Connect to the MQTT broker.
    if (mqttClient.connect(clientID, mqttUserName, mqttPass)) {
      Serial.print("MQTT to ");
      Serial.print(server);
      Serial.print(" at port ");
      Serial.print(mqttPort);
      Serial.println(" successful.");
    } else {
      Serial.print("MQTT connection failed, rc = ");
      Serial.print(mqttClient.state());
      Serial.println(" Will try the connection again in a few seconds");
      delay(connectionDelay * 1000);
    }
  }
}

// Función para suscribirse al canal 1
void mqttSubscribe(long subChannelID) {
  String myTopic = "channels/" + String(subChannelID) + "/subscribe";
  mqttClient.subscribe(myTopic.c_str());
  Serial.println("Suscribed to 1 ");
}
// Función para suscribirse al canal 2
void mqttSubscribe2(long subChannelID) {
  String myTopic2 = "channels/" + String(subChannelID) + "/subscribe";
  mqttClient.subscribe(myTopic2.c_str());
  Serial.println("Suscribed to 2 ");
}
void mqttSubscriptionCallback(char* topic, byte* payload, unsigned int length) {
  // Convertir el payload a una cadena terminada en '\0'
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';
  // Imprimir el mensaje completo recibido
  Serial.print("Payload recibido: ");
  Serial.println(message);

  // Buscar el campo "field1" en el mensaje
  char* field1Position = strstr(message, "\"field1\":\"");
  if (field1Position != NULL) {
    field1Position += 10;                             // Mover el puntero al inicio del valor (después de "\"field1\":\"")
    char* endPosition = strchr(field1Position, '"');  // Buscar el cierre del valor
    if (endPosition != NULL) {
      *endPosition = '\0';  // Terminar la cadena temporalmente
      // Convertir el valor de "field1" a entero
      int field1Value = atoi(field1Position);  // Convertir la cadena a entero
      Serial.print("field1 value: ");
      Serial.println(field1Value);
      // Determinar el canal en función del tópico
      if (strcmp(topic, "channels/2681698/subscribe") == 0) {
        StateNodo1 = field1Value;
        Serial.print("StateNodo1: ");
        Serial.println(StateNodo1);
      } else if (strcmp(topic, "channels/2733787/subscribe") == 0) {
        StateNodo2 = field1Value;
        Serial.print("StateNodo2: ");
        Serial.println(StateNodo2);
      }
    } else {
      Serial.println("Error: No se encontró el cierre del valor de field1.");
    }
  } else {
    Serial.println("Error: No se encontró el campo field1.");
  }

  //Basado en el estado de cada nodo se decide si la alarma se activa o no
  //Nodo 1
  if (StateNodo1) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Fuego detectado");
    lcd.setCursor(5, 1);
    lcd.print("Nodo 1");
    for (int i = 0; i < 15; i++) {
      digitalWrite(alarmLed, HIGH);
      for (int i = 0; i < 90; i++) {
        digitalWrite(buzzer, HIGH);
        delay(4);
        digitalWrite(buzzer, LOW);
        delay(6);
      }
      digitalWrite(alarmLed, LOW);
      delay(100);
    }
    StateNodo1 = 0;
  } 
  //Nodo 2
  else if (StateNodo2) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Fuego detectado");
    lcd.setCursor(5, 1);
    lcd.print("Nodo 2");
    for (int i = 0; i < 15; i++) {
      digitalWrite(alarmLed, HIGH);
      for (int i = 0; i < 90; i++) {
        digitalWrite(buzzer, HIGH);
        delay(4);
        digitalWrite(buzzer, LOW);
        delay(6);
      }
      digitalWrite(alarmLed, LOW);
      delay(100);
    }
    StateNodo2 = 0;
  } 
  //Si ningun nodo muestra incendio 
  else {
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print("Fuego no ");
    lcd.setCursor(3, 1);
    lcd.print("detectado");
    digitalWrite(alarmLed, LOW);
    digitalWrite(buzzer, LOW);
  }
  Serial.println("------Callback terminado-------");
}
// Function to publish messages to a ThingSpeak channel.
void mqttPublish(long pubChannelID, String message) {
  String topicString = "channels/" + String(pubChannelID) + "/publish";
  mqttClient.publish(topicString.c_str(), message.c_str());
}
