/* Programa V1.0.3 para conexion del Dispositivo
 *  esp32 al Broker EMQX
 *  Diseñado por Ismael Mamaní
 *  Fecha 12/12/2022
    Proyecto: SIGFA AWS | D1 ESP8266
    Bajo Licencia Creative Commons
    mail:tgeekinformatica.com
    site:https://ismael9m4.github.io/site-tgeek/
*/
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h >
#include <WiFiManager.h>
#include <PubSubClient.h>


 // Credenciales de conexion 
#define httpPort 80
#define SENSOR D5
#define WIFI_SSID "Process_Lite" //SSID del WiFi
#define WIFI_PASS "arribaDerchi033"//Contrasenia

#define MQTT_CLIENT_NAME "D1_ESP32_0000022"  //Cliente ID Unico MQTT,BaseESP32_+DEVICE_ID
#define MQTT_CLIENT_USER "isma22" //User MQTT
#define MQTT_CLIENTE_PASSWORD "38655155iam" //Password MQTT

/***********************************
 * Constantes y Variables
 ***********************************/
String serie= String(ESP.getChipId());//Obtenemos el numero unico de serie por placa
unsigned long tiempo1=0;
const String device_id = "8050238";
long dimmer;
char mqttBroker[]="192.168.10.112";
char payload[150];//Tamaño del mensaje
char topico1[150];
char topico2[150];
long lastMsg =0; //Para iniciar el mensaje
WiFiClient espClient;
PubSubClient client(espClient);
long currentMillis = 0;
long previousMillis;
int interval = 1000;
float calibrationFactor = 5.5;// FS-300A
volatile byte pulseCount;
byte pulse1Sec = 0;
float flowRate;
unsigned long flowMilliLitres;
unsigned int totalMilliLitres;
float flowLitres;
float totalLitres;

float distancia=0;
char salida [10]; 

/**************************************************************
  * Funciones Auxiliares Frecuencia de Pulsos por interrupcion
***************************************************************/

void IRAM_ATTR pulseCounter(){
  pulseCount++;
}

/*************************************************
  * Funcion Reconectar PubSubClient - revisar
*************************************************/
  void reconnect(){
    while(!client.connected()){
      Serial.println("Intentando conexion MQTT");
      if(client.connect(MQTT_CLIENT_NAME,MQTT_CLIENT_USER,MQTT_CLIENTE_PASSWORD)){
       Serial.println("Conectado a Servidor MQTT Sigfa IOT");
      }else{
        Serial.println("Fallo la conexion MQTT, Client_Status");
        Serial.println(client.state());
        Serial.println("Intentare de nuevo en 2 seg");
        delay(2000);
      }
    }
  }

/***************************
 * Funcion Conexion WiFi
 ****************************/
void setup_wifi() {
  delay(10);
  // Inicializamos Wifi
  Serial.println();
  Serial.print("Conectandose a la red WiFi: ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Estado: Conectado");
  Serial.println("Direccion IP : ");
  Serial.println(WiFi.localIP());
  tiempo1= millis();
  client.setServer(mqttBroker, 1883);//Nos conectamos al Broker: Servidor, Puerto
}

/***********************************/

void setup() {  
  Serial.begin(115200);
  //Inicializo el WATER FLOW METER:
  pinMode(SENSOR, INPUT_PULLUP);
  pulseCount = 0;
  attachInterrupt(digitalPinToInterrupt(SENSOR), pulseCounter, FALLING);//Activacion de interrupcion
  //Conexion WiFi
  setup_wifi();
}

void loop() {
  currentMillis = millis(); //Publicar cada 6 segundos
  if (currentMillis - previousMillis > interval){
    pulse1Sec = pulseCount;
    pulseCount = 0;
    flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
    previousMillis = millis();

    flowMilliLitres = (flowRate / 60) * 1000;
    flowLitres = (flowRate / 60);
    // Sumar los mililitros pasados ​​en este segundo al total acumulado
    totalMilliLitres += flowMilliLitres;
    totalLitres += flowLitres;
    
    // Caudal de este segundo en litros/minuto
    Serial.print("Flow rate: ");
    Serial.print(float(flowLitres));  // Convierte e imprime el valor en flotante
    Serial.print("L/min");
    Serial.print("\t");       // Imprime un espacio

    // Total acumulado de litros fluidos desde el inicio
    Serial.print("Output Liquid Quantity - Este seria el Total de Litros pasados: ");
    Serial.print(totalLitres);
    Serial.println("L");
  }
    if(!client.connected()){
    reconnect();
    }
  /******** Codigo Publicacionb cada 30 seg *******/
  if(flowLitres>0.0001){
    tiempo1=millis();
    distancia=float(flowLitres);
    dtostrf(distancia, 7, 5, salida);
    String to_send = String(salida)+","+ String(serie)+","+String(device_id);
    to_send.toCharArray(payload,50);
    String topico_aux = device_id+"/valores";
    topico_aux.toCharArray(topico1,50);
    client.publish(topico1,payload); //Publicar por MQTT: Topico, Mensaje
  }else{
    Serial.println("Se ha leido valor nulo");
  }
  delay(30000);   
  client.loop();
  
}
