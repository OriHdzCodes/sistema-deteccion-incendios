#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <WiFi.h>
#include <Vector.h>
#include <PubSubClient.h>  // Library required for MQTT

// Definición de sensor de temperatura
#define DHTPIN 2           // Digital esp32 pin to receive data from DHT.
#define DHTTYPE DHT11      // DHT 11 is used.
DHT dht(DHTPIN, DHTTYPE);  // DHT sensor set up.
//Definición variable temperatura
float t;
//Definicion de vector de temperaturas
float TempVect[15];
//Pin del sensor de gases
const int sensorPin = 0;
//Contador de repeticion de loop
int contador = 0;
//Indicador de incendio
bool IncendioBool = 0;
//Configuración de red
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

//Credenciales de thingspeak
const char mqttUserName[] = "PBwOFS80OB4LATEnMQkDCgM";
const char clientID[] = "PBwOFS80OB4LATEnMQkDCgM";
const char mqttPass[] = "UaheQckNLUcT37x6ORtB42o1";
//Canal de thingspeak al que se mandan datos
#define channelID 2681698

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
  // Recall that this code is executed only once.
  // Establish the serial transmission rate and set up the communication
  Serial.begin(115200);
  // Some delay to allow serial set up.
  delay(3000);

  dht.begin();  // Initialize DHT11 sensor

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
  }

  // Call the loop to maintain connection to the server.
  mqttClient.loop();

  // Leer la temperatura
  t = dht.readTemperature();

  // Verificar que la lectura sea válida
  if (!isnan(t)) {
    // Almacenar la temperatura en el arreglo
    if (contador < 15) {
      TempVect[contador] = t;  // Almacena la lectura en el índice actual en el vector
      contador++;
    } else {
      // Desplazar las temperaturas y agregar la nueva lectura
      for (int i = 1; i < 15; i++) {
        TempVect[i - 1] = TempVect[i];  // Desplazar los valores
      }
      TempVect[14] = t;  // Agregar nueva temperatura en la última posición
    }

    // Calcular el promedio
    float Suma = 0;
    for (int i = 0; i < 15; i++) {
      Suma += TempVect[i];
    }
    float TempProm = Suma / 15;
    Serial.println("Promedio: " + String(TempProm));
    Serial.println(analogRead(sensorPin));

    // Detectar humo
    //Se analiza los datos de los sensores cuando el ultimo dato del vector de temperatura es diferente a 0
    //En caso de que el sensor de humo supere 3400 en su lectura o la temperatura actual sea mayor a la temperatura promedio+2
    if (TempVect[14] != 0) {
      if (analogRead(sensorPin) > 3400 || (TempProm + 2) < t) {
        Serial.println("Humo detectado");
        IncendioBool = 1;
        delay(5000);//En caso de detectar incendio espera 5 segundos antes de volver a leer datos de los sensores
      }
    }

  } else {
    Serial.println("Error al leer la temperatura");
  }
  delay(100);

  // Publicar en ThingSpeak cada 15 segundos 
  if ((millis() - lastPublishMillis) > updateInterval * 1000) {
    mqttPublish(channelID, (String("field1=") + String(IncendioBool))); // Se publica en el campo 1 el bool de incendio
    lastPublishMillis = millis();
    IncendioBool = 0; //Despues de mandar el dato sobre incendio se establece en 0
  }
}
// Function to connect to WiFi.
void connectWifi() {
  Serial.println("Connecting to Wi-Fi...");
  // Loop until WiFi connection is successful
  while (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, pass);
    delay(connectionDelay * 1000);
    Serial.println(WiFi.status());
  }
  Serial.println("Connected to Wi-Fi.");
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
      // See https://pubsubclient.knolleary.net/api.html#state for the failure code explanation.
      Serial.print(mqttClient.state());
      Serial.println(" Will try the connection again in a few seconds");
      delay(connectionDelay * 1000);
    }
  }
}

// Function to subscribe to ThingSpeak channel for updates.
void mqttSubscribe(long subChannelID) {
  String myTopic = "channels/" + String(subChannelID) + "/subscribe";
  mqttClient.subscribe(myTopic.c_str());
}

// Function to handle messages from MQTT subscription to the ThingSpeak broker.
void mqttSubscriptionCallback(char* topic, byte* payload, unsigned int length) {
  // Print the message details that was received to the serial monitor.
  Serial.print("Message arrived from ThinksSpeak broker [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}
// Function to publish messages to a ThingSpeak channel.
void mqttPublish(long pubChannelID, String message) {
  String topicString = "channels/" + String(pubChannelID) + "/publish";
  mqttClient.publish(topicString.c_str(), message.c_str());
}
