#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
WiFiServer server(80); 

/*Define LiquidCrystal_I2C - lcd */
LiquidCrystal_I2C lcd(0x3F, 20, 4);

char   host[] = "api.thingspeak.com"; // ThingSpeak address
String APIkey = "624367";             // Thingspeak Read Key, works only if a PUBLIC viewable channel
const int httpPort = 80;
 
const char *ssid     = "N8"; 
const char *password = "2133@@16"; 
WiFiClient client;
const unsigned long HTTP_TIMEOUT = 10000;  // max respone time from server

void setup(){
  Serial.begin(9600);
  WiFi.begin(ssid,password);
  lcd.init();
  lcd.begin(20, 4);
  lcd.backlight();
}

void loop(){
  RetrieveTSChannelData();
  delay(10000); //Wait before we request again
}

void RetrieveTSChannelData() {  // Receive data from Thingspeak
  static char responseBuffer[3*1024]; // Buffer for received data
  client = server.available(); 
  if (!client.connect(host, httpPort)) { 
    Serial.println("connection failed"); 
    return; 
  } 
  String url = "/channels/" + APIkey; // Start building API request string
  url += "/feeds.json?results=1";  // 1 is the results request number
  client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
  while (!skipResponseHeaders());                      // Wait until there is some data and skip headers
  while (client.available()) {                         // Now receive the data
    String line = client.readStringUntil('\n');
    if ( line.indexOf('{',0) >= 0 ) {                  // Ignore data that is not likely to be JSON formatted, so must contain a '{'
      Serial.println(line);                            // Show the text received
      line.toCharArray(responseBuffer, line.length()); // Convert to char array for the JSON decoder
      decodeJSON(responseBuffer);                      // Decode the JSON text
    }
  }
  client.stop();
}

bool skipResponseHeaders() { 
  char endOfHeaders[] = "\r\n\r\n"; // HTTP headers end with an empty line 
  client.setTimeout(HTTP_TIMEOUT); 
  bool ok = client.find(endOfHeaders); 
  if (!ok) { Serial.println("No response or invalid response!"); } 
  return ok; 
} 

bool decodeJSON(char *json) {
  StaticJsonBuffer <3*1024> jsonBuffer;
  char *jsonstart = strchr(json, '{'); // Skip characters until first '{' found and ignore length, if present
  if (jsonstart == NULL) {
    Serial.println("JSON data missing");
    return false;
  }
  json = jsonstart;
  JsonObject& root = jsonBuffer.parseObject(json); // Parse JSON
  if (!root.success()) {
    Serial.println(F("jsonBuffer.parseObject() failed"));
    Serial.println("Convert Json Failed.");  
    return false;
  }else{
    Serial.println("Convert Json Successs.");
  }
  
  JsonObject& root_data = root["channel"]; // Begins and ends within first set of { }
  String id   = root_data["id"];
  String name = root_data["name"];
  String field1_name = root_data["field1"]; // Extracts field name in this example field1 is called 'Total'
  String field2_name = root_data["field2"]; // Extracts field name in this example field2 is called 'NumOfMale'
  String field3_name = root_data["field3"]; // Extracts field name in this example field3 is called 'NumOfFemale'
  String field4_name = root_data["field4"]; // Extracts field name in this example field2 is called 'NumOfHappy'
  String field5_name = root_data["field5"]; // Extracts field name in this example field3 is called 'NumOfAnger'
  String datetime    = root_data["updated_at"];
  Serial.println("\n\n Channel id: "+id+" Name: "+ name);
  Serial.println(" Readings last updated at: "+datetime);
  JsonObject& channel = root["feeds"][0]; // Now we can read 'feeds' values and so-on
  String entry_id     = channel["entry_id"];
  String field1value  = channel["field1"];
  String field2value  = channel["field2"];
  String field3value  = channel["field3"];
  String field4value  = channel["field4"];
  String field5value  = channel["field5"];
  Serial.print(" Field1 entry number ["+entry_id+"] had a value of: ");Serial.println(field1value);
  Serial.print(" Field2 entry number ["+entry_id+"] had a value of: ");Serial.println(field2value);
  Serial.print(" Field3 entry number ["+entry_id+"] had a value of: ");Serial.println(field3value);
  Serial.print(" Field4 entry number ["+entry_id+"] had a value of: ");Serial.println(field4value);
  Serial.print(" Field5 entry number ["+entry_id+"] had a value of: ");Serial.println(field5value);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(name);
  lcd.setCursor(0, 1);
  lcd.print("Total:"+field1value);
  lcd.setCursor(0, 2);
  lcd.print("Male/Femala:"+field2value+"/"+field3value);
  lcd.setCursor(0, 3);
  lcd.print("Happy/Anger:"+field4value+"/"+field5value);
}
