#include <WiFi.h>
#include <PubSubClient.h> //comunicação com um servidor MQTT
#include "DHT.h"

const char* ssid = "ifce-espacoMaker";
const char* password = "CR1AT1V1UM";
const char* mqtt_server = "broker.hivemq.com"; //ndereço do servidor MQTT

WiFiClient espClient; //objeto para gerenciar a conexão Wi-Fi.
PubSubClient client(espClient); //objeto para se conectar ao servidor MQTT, utilizando a conexão Wi-Fi 
//unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50) //tamanho máximo de uma mensagem que pode ser enviada ou recebida.
char msg[MSG_BUFFER_SIZE]; //array de caracteres que será usado para armazenar as mensagens.

#define BUZZER_PIN 18
#define DHTPIN 13 // pino DHT11
#define DHTTYPE DHT11 // DHT 11
#define LED 2
float humidadeAnterior=0,temperaturaAnterior=0;

DHT dht(DHTPIN, DHTTYPE);

//Toda vez que uma nova mensagem chega no tópico que o ESP32 está "ouvindo", essa função é automaticamente chamada
void callback(char* nomeDoTopico, byte* dadosEnviados, unsigned int tamanhoDoConteudoDaMensagem){
  Serial.print("Message arrived [");
  Serial.print(nomeDoTopico);
  Serial.print("] ");
  for (int i=0; i<tamanhoDoConteudoDaMensagem; i++) {
    Serial.print((char)dadosEnviados[i]);
  }
  Serial.println();

  if((char)dadosEnviados[0]=='L' || (char)dadosEnviados[0]=='l'){
    digitalWrite(LED, HIGH);
    snprintf(msg, MSG_BUFFER_SIZE, "O LED está acesso");
    Serial.print("Publica mensagem: ");
    Serial.println(msg);
    client.publish("engeasier/led",msg);
  }

  if ((char)dadosEnviados[0]=='D' || (char)dadosEnviados[0]=='d') {
    digitalWrite(LED, LOW);
    snprintf(msg, MSG_BUFFER_SIZE, "O LED está apagado");
    Serial.print("Publica mensagem: ");
    Serial.println(msg);
    client.publish("engeasier/led",msg);
  }

  if ((char)dadosEnviados[0]=='A' || (char)dadosEnviados[0]=='a') {
    digitalWrite(BUZZER_PIN, HIGH);
    snprintf(msg, MSG_BUFFER_SIZE, "A sirene está Ligada");
    Serial.print("Publica mensagem: ");
    Serial.println(msg);
    client.publish("engeasier/sirene",msg);
  }

  if ((char)dadosEnviados[0]=='P' || (char)dadosEnviados[0]=='p') {
    digitalWrite(BUZZER_PIN, LOW);
    snprintf(msg, MSG_BUFFER_SIZE, "A sirene está Desligada");
    Serial.print("Publica mensagem: ");
    Serial.println(msg);
    client.publish("engeasier/sirene",msg);
  }

}


void setup_wifi(){
  delay(10);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}

void reconnect(){
  while(!client.connected()){
    Serial.print("Attempting MQTT connection...");

    String clientId = "ENGEASIER_MQTT";
    clientId += String(random(0xffff),HEX);

    if(client.connect(clientId.c_str())){
      Serial.println("Conectado");

      client.subscribe("engeasier/publisher");
    }else{
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void dht11(){
  float humidade = dht.readHumidity(); //Ler a humidade
  float temperatura = dht.readTemperature(); //Ler a temperatura
  
  
  // testa se retorno é valido, caso contrário algo está errado.
  if (isnan(temperatura) || isnan(humidade)) 
  {
    char* mensagem = "Failed to read from DHT";
    Serial.println(mensagem);
    sprintf(msg, "%s", mensagem);
    client.publish("engeasier/temperatura",mensagem);
    client.publish("engeasier/humidade",mensagem);
  }
  else
  {
    if(humidade != humidadeAnterior){
      humidadeAnterior = humidade;
      Serial.print("Umidade: ");
      Serial.print(humidade);
      Serial.println(F(" °%"));
      sprintf(msg,"%.2f",humidade);
      client.publish("engeasier/humidade",msg);
    }

    if(temperatura != temperaturaAnterior){
      temperaturaAnterior = temperatura;
      Serial.print("Temperatura: ");
      Serial.print(temperatura);
      Serial.println((" °C"));
      sprintf(msg,"%.2f",temperatura);
      client.publish("engeasier/temperatura",msg);
    }
    
  }
}

void setup() {
  // put your setup code here, to run once:

  pinMode(LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  dht.begin(); //Inicializa o DHT11

  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server,1883);
  client.setCallback(callback);

}

void loop() {
  
  dht11();

  if(!client.connected()){
    reconnect();
  }

  client.loop();
}
