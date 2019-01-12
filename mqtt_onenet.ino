#include <ESP8266WiFi.h>
#include "SSD1306Wire.h"
#include <Wire.h>
#include <EEPROM.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "DHT.h"
#define DHTPIN 12 // what digital pin we're connected to NodeMCU (D6)
#define DHTTYPE DHT11 
SSD1306Wire  display(0x3c, D1, D2);
DHT dht(DHTPIN, DHTTYPE);
#define wifi_ssid "maker_space"
#define wifi_password "chuangke666"
#define mqtt_server "183.230.40.39"
#define humidity_topic "sensor/humidity"
#define temperature_celsius_topic "sensor/temperature_celsius"
#define temperature_fahrenheit_topic "sensor/temperature_fahrenheit"
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
char msg_buf[200];
int value = 0;
float h;
float t;
float f;
char  c[] ="";
char dataTemplete[]="{\"wendu\":%d,\"shidu\":%d}";
char msgJson[75];  //存储json数据
char debug_buf[200];  //打印调试数据
int i;
unsigned short json_len=0;
uint8_t* packet_p;
uint8_t debug_buffer_start_index = 0;
struct CONFIG {    //账号密码结构体
char ssid[32];
char password[32];
};
void smartConfig()
{
  WiFi.mode(WIFI_STA);
  Serial.println("\r\nWait for Smartconfig");
  WiFi.beginSmartConfig();
  while (1)
  {
    Serial.print(".");
    digitalWrite(2, 0);
    delay(500);
    digitalWrite(2, 1);
    delay(500);
    if (WiFi.smartConfigDone())
    {
      EEPROM.begin(512);
      CONFIG buf;
      Serial.println("SmartConfig Success");
      Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
      Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
      strcpy(buf.ssid,  WiFi.SSID().c_str());
      strcpy(buf.password, WiFi.psk().c_str());
      EEPROM.put<CONFIG>(0, buf);
      EEPROM.commit();
      Serial.println(buf.ssid);
      Serial.println(buf.password);
      break;
    }
  }
}
  void peiwang()
{
  EEPROM.begin(512);
  CONFIG buf;
  EEPROM.get<CONFIG>(0, buf);
  Serial.println(buf.ssid);
  Serial.println(buf.password);
  EEPROM.commit();
  WiFi.begin(buf.ssid, buf.password);
  long lastMsg = millis();
  while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  Serial.print(".");
   long now = millis();
    if (now - lastMsg > 10000) {
      smartConfig();  //微信智能配网 
      break;
    }
  }
  }
void setup() {
  Serial.begin(115200);
    pinMode(2, OUTPUT); 
  dht.begin();
  peiwang();
  client.setServer(mqtt_server, 6002);
  client.connect("505657690","194929","4UB1QMXM6jq7yZRBezuTnrPhP88=");
   client.setCallback(callback);
    display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  drawFontFaceDemo();
}
void drawFontFaceDemo() {
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 0, "Hello world");
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 10, "Hello world");
    display.setFont(ArialMT_Plain_24);
    display.drawString(0, 26, "Hello world");
    display.display();
    delay(1000);
    display.clear();
}
void callback(char* topic, byte* payload, unsigned int length)  //接收消息
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  // Switch on the LED if an 1 was received as first character    //消息处理
  if ((char)payload[0] == '1') {
    digitalWrite(2, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(2, HIGH);  // Turn the LED off by making the voltage HIGH
  }
}
String macToStr(const uint8_t* mac)
{
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  return result;
}
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
      Serial.print("Attempting MQTT connection...");
      String clientName;  
      clientName += "esp8266-";
      uint8_t mac[6];
      WiFi.macAddress(mac);
      clientName += macToStr(mac);
      clientName += "-";
      clientName += String(micros() & 0xff, 16);
      Serial.print("Connecting to ");
      Serial.print(mqtt_server);
      Serial.print(" as ");
      Serial.println(clientName);
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
  if (client.connect("505657690","194929","4UB1QMXM6jq7yZRBezuTnrPhP88=")) {   //这是onenet的协议方式
    //if (client.connect((char*) clientName.c_str()), mqtt_user, mqtt_password)) {
      Serial.println("connected");
      client.publish("outTopic", "hello world"); // 发布消息
      // ... and resubscribe
      client.subscribe("inTopic");//订阅主题
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
  void dht11()
  {
     float h = dht.readHumidity();
      // Read temperature as Celsius (the default)
      float t = dht.readTemperature();
      // Read temperature as Fahrenheit (isFahrenheit = true)
      float f = dht.readTemperature(true);
      delay(50);
     display.setFont(ArialMT_Plain_16);
    display.drawString(0, 0,String(t).c_str());
    display.drawString(0, 10,String(h).c_str());
    display.display();
     if (isnan(h) || isnan(t) || isnan(f)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
      }
    }
void loop() {

      if (!client.connected()) {
        reconnect();
      }
    client.loop();
    long now = millis();
    if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
 int h = dht.readHumidity();
 int t = dht.readTemperature();
 //下面进行封包
  snprintf(msgJson,40,dataTemplete,t ,h);
    json_len=strlen(msgJson); //packet length count the end char '\0'
    msg_buf[0]=char(0x03);  //palyLoad packet byte 1, one_net mqtt Publish packet payload byte 1, type3 , json type2 
    msg_buf[1]=char(json_len>>8);  //high 8 bits of json_len (16bits as short int type)
    msg_buf[2]=char(json_len&0xff); //low 8 bits of json_len (16bits as short int type)
    memcpy(msg_buf+3,msgJson,strlen(msgJson));
   msg_buf[3+strlen(msgJson)] = 0;
    Serial.print("Publish message: ");
    Serial.println(msgJson);
    client.publish("$dp",msg_buf,3+strlen(msgJson),false);   
    //debug one_net packet
    packet_p = client.getBufferPointer();
    for(i = 0 ; i< 200;i++)
    {
     //sprintf(debug_buf,"0x%02x ",msg_buf[i]);
     sprintf(debug_buf,"0x%02x ",packet_p[i]);
     Serial.print(debug_buf);
    }
    Serial.println();
    for(i = 0 ; i< 200;i++)
    {
     sprintf(debug_buf,"0x%02x ",msg_buf[i]);
     //sprintf(debug_buf,"0x%02x ",packet_p[i]);
     Serial.print(debug_buf);
    }
    Serial.println();
    debug_buffer_start_index=client.getDebugVar();
    Serial.println(debug_buffer_start_index);
  } 
 display.clear();
}
