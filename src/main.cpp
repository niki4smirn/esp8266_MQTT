#include <Arduino.h>
#include <ESP8266WiFi.h>  // библиотека для работы с wi-fi модулем
#include <PubSubClient.h>  // библиотека для работы с mqtt

const char* ssid = "........";  // ssid wi-fi сети 
const char* password = "........";  // пароль wi-fi сети
const char* mqtt_server = "........";  // ip mqtt сервера (пример: 192.168.31.155)
const int mqtt_port = 1883;  // порт mqtt сервера

WiFiClient espClient;  // создаем Wi-Fi клиент
PubSubClient client(espClient);  // создаем клиент mqtt, используя 
int lastMsg = 0;  // время последнего сообщения в миллисекундах
char msg[50];  // строка сообщения, которое мы будем отправлять
int value = 0;  

// номер пина, на который мы подаем сигнал (в моём случае это светодиод на плате)
const int LED_PIN = 2; 

// функция подключения к wi-fi сети
void setup_wifi() {
  delay(10);
  // подключение к wi-fi сети
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  // ожидание подключения
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // успешное подключение
  Serial.println("");
  Serial.println("WiFi connected");
  //вывод в serial ip адреса
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


// функция, которая вызывается, когда в топике появляется новое сообщение
void callback(char* topic, byte* payload, unsigned int length) {
  // вывод в serial топика и сообщения 
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (u_int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // если сообщение начинается с единицы, то включаем светодиод
  if ((char)payload[0] == '1') {
    digitalWrite(LED_PIN, LOW);
  } 
  // если сообщение начинается с нуля, то выключаем светодиод
  if ((char)payload[0] == '0') {
    digitalWrite(LED_PIN, HIGH);  
  }
  // т.к. я использую светодиод, расположенный на плате, LOW - включить, HIGH - выключить

}

void reconnect() {
  // цикл выполняется пока плата не подключится по mqtt
  while (!client.connected()) {

    Serial.print("Attempting MQTT connection...");
    // обратите внимание, что у всех устройств, которые подключены к брокеру, должен быть
    // уникальный id, поэтому, чтобы подключить вторую плату к сети, в ёё прошивке нужно будет 
    // указать другой номер (не 01)
    String clientId = "ESP8266Client-01"; 

    // пробуем подключиться
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // публикуем в топик outTopic сообщение hello world
      client.publish("outTopic", "hello world");
      // подписываемся на топик inTopic
      client.subscribe("inTopic");
    } 
    else {
      // подключение не удалось, выводим сообщение в serial и ждём 5 секунд
      Serial.println("connection failed");
      delay(5000);
    }
  }
}

void setup() {
  pinMode(LED_PIN, OUTPUT);     // инициализируем LED_PIN как выходной
  Serial.begin(115200);
  setup_wifi();

  // настраиваем параметры mqtt клиента 
  // (ip, порт mqtt и функцию, которая будет вызываться при появлении сообщения
  // в топике, на который мы подписаны)
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {

  // пытаемся подключиться 
  if (!client.connected()) {
    reconnect();
  }

  client.loop();  //обновляет приходящие на клиент данные 

  // время сейчас (в миллисекундах)
  long now = millis();

  // если с момента последнего отправленного сообщения прошло
  // больше 2000 миллисекунд (2 секунд)
  if (now - lastMsg > 2000) {
    // выводим сообщение в serial и публикуем в топик outTopic
    lastMsg = now;
    ++value;
    snprintf (msg, 75, "hello world #%d", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("outTopic", msg);
  }
}