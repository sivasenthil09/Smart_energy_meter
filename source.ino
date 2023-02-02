#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include <PZEM004Tv30.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <ThingSpeak.h>
#include <SPI.h>

WiFiClient  client;
PZEM004Tv30 pzem(D7, D8); //RX/TX
LiquidCrystal_I2C lcd(0x27, 16, 2);
ESP8266WebServer server(80);

float voltage, current, power, energy, frequency, pf, units;

unsigned long myChannelNumber = 1519974;
const char * myWriteAPIKey = "9OYRK9BAB6SV9HG1";

float number1 = 0.0000f;
float number2 = 0.0000f;
float number3 = 0.0000f;
float number4 = 0.0000f;
float number5 = 0.0000f;
float number6 = 0.0000f;
float number7 = 0.000000f;

String myStatus = "";

/*Put your SSID & Password*/
const char* ssid = "Lucky";  // Enter SSID here
const char* password = "12345678";  //Enter Password here

uint8_t LED1pin = D6;         //GPIO 12 OR D6
bool LED1status = LOW;

float consum = 0.000000;
int timer = 0;
int testPin = 0;
int ExternalPin = D5;          //GPIO 14 OR D5
int eeAddress = 10;
uint32_t tsLastReport = 0;

volatile unsigned long timer0_millis;
String page = "";
String Units = "";
int dis = 0;
float eepromdata=0.00000f;
int loopcheck=0;
int thingspeak =0;
float floatingpoint_variable = 0.000000;
int EEaddress = 0;


ICACHE_RAM_ATTR void detectsMovement() {
    Serial.println("EEPROM Data Writing.... ");
    floatingpoint_variable= eepromdata;

    EEPROM.put(EEaddress,floatingpoint_variable); // Writes the value 3.141592654 to EEPROM
    EEPROM.commit();

    Serial.print("\t eepromdata EEPROM data: ");
    Serial.print(float(floatingpoint_variable),6);
}

void EEPROMloop() {
    Serial.println("EEPROM Data Writing.... ");
    floatingpoint_variable= eepromdata;

    EEPROM.put(EEaddress,floatingpoint_variable); // Writes the value 3.141592654 to EEPROM
    EEPROM.commit();

    Serial.print("\t eepromdata EEPROM data: ");
    Serial.print(float(floatingpoint_variable),6);
}


void setup() {
    Serial.begin(115200);
    EEPROM.begin(32);

    attachInterrupt(digitalPinToInterrupt(ExternalPin), detectsMovement, FALLING);
    delay(3000);
    Serial.print("Connecting to ");
    Serial.print(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected..!");
    Serial.print("Got IP: ");
    Serial.println(WiFi.localIP());

    Serial.print(" ");
    Serial.print(" ");
    Serial.print("EEPROM contents at Address=8 is  : ");
    floatingpoint_variable = 0; // To prove it read from EEPROM!
    EEPROM.get(EEaddress,floatingpoint_variable);
    Serial.println(floatingpoint_variable,8);

    float readParam = floatingpoint_variable;
    Serial.print("\t Consum EEPROM: ");
    Serial.print(float(readParam),6);
    consum = readParam;

      lcd.init();                      // initialize the lcd 
  lcd.init();
  lcd.backlight();

  lcd.setCursor(5, 0);
  lcd.print("NGN LAB..!");
  lcd.setCursor(1, 1);
  lcd.print("POWER METER");
  


    timer = millis();

    testPin = digitalRead(ExternalPin);
    delay(200);
    server.on("/", handle_OnConnect);
    server.on("/led1on", handle_led1on);
    server.on("/led1off", handle_led1off);
    server.onNotFound(handle_NotFound);

    server.begin();
    ThingSpeak.begin(client);
    Serial.println("HTTP server started");
    consum = 80;
}


void loop() {
    timer = millis() - tsLastReport;
    int timercheck = millis();
    float sec = timer;

//    voltage = pzem.voltage();
//    current = pzem.current();
//    power = pzem.power();
//    energy = pzem.energy();
//    frequency = pzem.frequency();
//    pf = pzem.pf();

    voltage = random(224,230);
    current = random(1.5,2.3);  
    power = random(100,130);
    frequency = 50;
    pf = 1;


    float Seconds = 0.00;
    float Minutes = 0.00;
    float Hours = 0.00;
    float Energy = 0.0000;

    Seconds = ((float)sec / 1000);

    tsLastReport = millis();

    Minutes = (float)Seconds / 60;
    Hours = (float)Minutes / 60;
    Energy = (float)power * Hours / 1000;
    energy = Energy;
    String a= String(Energy);
    String b = String(voltage);


//    if(a=="nan" && b=="nan") {
//        if(loopcheck==0) {
//            EEPROMloop();
//            loopcheck=1;
//        }
//    }
    if(a == "nan") {
        Energy = 0.000;
    }

    else {
        consum = float(consum) + float(Energy);
        if(consum !=0) {
            eepromdata=float(consum);
            loopcheck=0;
        }
        else {
            detectsMovement();
        }
    }

    Serial.print("\t Consum Test: ");
    Serial.print(float(consum),5);
    Serial.print("c");

    Units = String(consum,5);

    Serial.print("\t Voltage: ");
    Serial.print(voltage);
    Serial.print("V");
    Serial.print("\t Current: ");
    Serial.print(current);
    Serial.print("A");
    Serial.print("\t Power: ");
    Serial.print(power);
    Serial.print("W");
    Serial.print("\tFrequency: ");
    Serial.print(frequency, 1);
    Serial.print("Hz");
    Serial.print("\tPF: ");
    Serial.print(pf);
    Serial.print("%");
    Serial.print("\t Energy: ");
    Serial.print(Energy, 5);
    Serial.println("kWh");
    Serial.print("\t UNITS: ");
    Serial.print(consum, 5);
    Serial.println("kWh");
    Serial.println("");
    Serial.print("\t Seconds : ");
    Serial.print(Seconds, 5);
    Serial.print("\t Minutes : ");
    Serial.print(Minutes, 5);
    Serial.print("\t Hours : ");
    Serial.println(Hours, 5);

    server.handleClient();
   
    lcd.clear();

    number1 = voltage;
    lcd.setCursor(0, 0);
    lcd.print("V=");
    lcd.print(voltage, 2);

    number2 = current;
    lcd.setCursor(9, 0);
    lcd.print("A=");
    lcd.print(current, 4);

  

    number3 = power;
    lcd.setCursor(0, 1);
    lcd.print("P=");
    lcd.print(power);

    number7 = consum;
    lcd.setCursor(9, 1);
    lcd.print("U=");
    lcd.print(consum, 5);
    number4 = energy;
    number5 = frequency;
    number6 = pf;



  
    
    if(thingspeak==30) {
        number1 = voltage;
        number2 = current;
        number3 = power;
        number4 = energy;
        number5 = frequency;
        number6 = pf;
        number7 = float(consum);

        ThingSpeak.setField(1, number1);
        ThingSpeak.setField(2, number2);
        ThingSpeak.setField(3, number3);
        ThingSpeak.setField(4, number4);
        ThingSpeak.setField(5, number5);
        ThingSpeak.setField(6, number6);
        ThingSpeak.setField(7, number7);

        ThingSpeak.setStatus(myStatus);

        int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

        if (x == 200) {
            Serial.println("Channel update successful.");
        }
        else {
            Serial.println("Problem updating channel. HTTP error code " + String(x));
        }
        thingspeak=0;
    }

    if (LED1status==HIGH)
    {
        digitalWrite(LED1pin, HIGH);
        delay(2000);
        digitalWrite(LED1pin, LOW);
    }
    else
    {
        digitalWrite(LED1pin, LOW);
    }

    testPin = digitalRead(ExternalPin);
    dis++;
    thingspeak++;
    Serial.println("*********************************");
    Serial.println();
    delay(800);
}


void handle_OnConnect() {
    LED1status = LOW;
    server.send(200, "text/html", SendHTML(voltage, current, power, frequency, pf, Units, LED1status));
}

void handle_led1on() {
    LED1status = HIGH;
    Serial.println("GPIO12 Status: ON");
    server.send(200, "text/html", SendHTML(voltage, current, power, frequency, pf, Units, true));
}

void handle_led1off() {
    LED1status = LOW;
    Serial.println("GPIO12 Status: OFF");
    server.send(200, "text/html", SendHTML(voltage, current, power, frequency, pf, Units, false));
}

void handle_NotFound() {
    server.send(404, "text/plain", "Not found");
}

String SendHTML(float voltage, float current, float power, float frequency, float pf, String Units, uint8_t led1stat) {
    String html = "<!DOCTYPE html>";
    html += "<html>";
    html += "<head>";
    html += "<title>power meter</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/5.7.2/css/all.min.css'>";
    html += "<link rel='stylesheet' type='text/css' href='styles.css'>";
    html += "<style>";
    html += "body { background-color: #fff; font-family: sans-serif; color: #333333; font: 14px Helvetica, sans-serif box-sizing: border-box;}";
    html += "#page { margin: 20px; background-color: #fff;}";
    html += ".container { height: inherit; padding-bottom: 20px;}";
    html += ".header { padding: 20px;}";
    html += ".header h1 { padding-bottom: 0.3em; color: #008080; font-size: 45px; font-weight: bold; font-family: Garmond, 'sans-serif'; text-align: center;}";
    html += "h2 { padding-bottom: 0.2em; border-bottom: 1px solid #eee; margin: 2px; text-align: center;}";
    html += "h4 { padding-bottom: 0.2em; margin: 2px; text-align: center;}";
    html += ".header h3 { font-weight: bold; font-family: Arial, 'sans-serif'; font-size: 17px; color: #b6b6b6; text-align: center;}";
    html += ".box-full { padding: 20px; border 1px solid #ddd; border-radius: 1em 1em 1em 1em; box-shadow: 1px 7px 7px 1px rgba(0,0,0,0.4); background: #fff; margin: 20px; width: 500px;}";
    html += "@media (max-width: 494px) { #page { width: inherit; margin: 5px auto; } #content { padding: 1px;} .box-full { margin: 8px 8px 12px 8px; padding: 10px; width: inherit;; float: none; } }";
    html += "@media (min-width: 494px) and (max-width: 980px) { #page { width: 465px; margin 0 auto; } .box-full { width: 380px; } }";
    html += "@media (min-width: 980px) { #page { width: 930px; margin: auto; } }";
    html += ".sensor { margin: 12px 0px; font-size: 2.5rem;}";
    html += ".sensor-labels { font-size: 1rem; vertical-align: middle; padding-bottom: 15px;}";
    html += ".units { font-size: 1.2rem;}";
    html += "hr { height: 1px; color: #eee; background-color: #eee; border: none;}";
    html += ".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 6px;}\n";
    html += ".button-on {background-color: #1abc9c;}\n";
    html += ".button-on:active {background-color: #16a085;}\n";
    html += ".button-off {background-color: #34495e;}\n";
    html += ".button-off:active {background-color: #2c3e50;}\n";
    html += "p1 {font-size: 14px;color: #888;margin-bottom: 15px;}\n";
    html += "</style>";

    // Code Start
    html += "<script>\n";
    html += "setInterval(loadDoc,1000);\n";
    html += "function loadDoc() {\n";
    html += "var xhttp = new XMLHttpRequest();\n";
    html += "xhttp.onreadystatechange = function() {\n";
    html += "if (this.readyState == 4 && this.status == 200) {\n";
    html += "document.body.innerHTML =this.responseText}\n";
    html += "};\n";
    html += "xhttp.open(\"GET\", \"/\", true);\n";
    html += "xhttp.send();\n";
    html += "}\n";
    html += "</script>\n";
    // Code END

    // Header Name Line
    html += "</head>";
    html += "<body>";
    html += "<div id='page'>";
    html += "<div class='header'>";
    html += "<h1>SMART ENERGY METER</h1>";
    html += "</div>";
    html += "<div id='content' align='center'>";
    html += "<div class='box-full' align='left'>";
    html += "<h2>POWER METER VALUE</h2>";
    html += "<div class='sensors-container'>";

    //For VOLTAGE
    html += "<div class='sensors'>";
    html += "<p class='sensor'>";
    html += "<i class='fas fa-plug' style='color:#0275d8'></i>";
    html += "<span class='sensor-labels'> VOLTAGE = </span>";
    html += (float)voltage;
    html += "<sup class='units'>Volt</sup>";
    html += "</p>";
    html += "<hr>";
    html += "</div>";

    //For Current
    html += "<div class='sensors'>";
    html += "<p class='sensor'>";
    html += "<i class='fab fa-medapps' style='color:#5bc0de'></i>";
    html += "<span class='sensor-labels'> CURRENT =  </span>";
    html += (float)current;
    html += "<sup class='units'>Amps</sup>";
    html += "</p>";
    html += "<hr>";

    //For Power
    html += "<p class='sensor'>";
    html += "<i class='fas fa-poo-storm' style='color:#cc3300'></i>";
    html += "<span class='sensor-labels'> POWER = </span>";
    html += (float)power;
    html += "<sup class='units'>w</sup>";
    html += "</p>";
    html += "<hr>";

    //For Frequency
    html += "<p class='sensor'>";
    html += "<i class='fas fa-burn' style='color:#f7347a'></i>";
    html += "<span class='sensor-labels'> FREQUENCY = </span>";
    html += (float)frequency;
    html += "<sup class='units'>Hz</sup>";
    html += "</p>";
    html += "<hr>";

    //For PF
    html += "<p class='sensor'>";
    html += "<i class='fas fa-tachometer-alt' style='color:#6a5acd'></i>";
    html += "<span class='sensor-labels'> PF = </span>";
    html += (float)pf;
    html += "<sup class='units'>Hz</sup>";
    html += "</p>";
    html += "<hr>";

    //For Units
    html += "<p class='sensor'>";
    html += "<i class='fas fa-charging-station' style='color:#3cb371'></i>";
    html += "<span class='sensor-labels'> UNITS = </span>";
    html += (String)Units;
    html += "<sup class='units'>KW-h</sup>";
    html += "</p>";
    html += "<hr>";

    if (led1stat)
    {
        html += "<h4>MCB SWITCH: OFF</h4>";
        html += "<a class=\"button button-off\" href=\"/led1off\">MCB-ON</a>\n";
    }
    else
    {
        html += "<h4>MCB SWITCH: ON</h4>";
        html += "<a class=\"button button-on\" href=\"/led1on\">MCB-OFF</a>\n";
    }

    html += "</div>";
    html += "</div>";
    html += "</div>";
    html += "</div>";
    html += "</div>";
    html += "</div>";
    html += "</body>";
    html += "</html>";
    return html;
}
