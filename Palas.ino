#include <Ethernet2.h>
#include <Servo.h>

Servo myservo[4];
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 0, 16);

EthernetServer server(80);
String estado[2] = {"OFF", "ON"};
int dir[2] = {1, 1};
int movC[2] = {0, 0};
boolean movPala[2] = {false, false};

// Values to M0, M1, V0, V1
int values[] = {0, 0, 3, 3};

void setup() {
  myservo[0].attach(2, 750, 2250);  // attaches the servo on pin 9 to the servo object
  myservo[0].write(1500);
  myservo[1].attach(3, 750, 2250);  // attaches the servo on pin 9 to the servo object
  myservo[1].write(1500);
  myservo[2].attach(5);  // attaches the servo on pin 9 to the servo object
  myservo[2].write(90);
  myservo[3].attach(6);  // attaches the servo on pin 9 to the servo object
  myservo[3].write(90);

  Ethernet.begin(mac, ip);
  server.begin();
}

void moverPala(int id) {
  if (movC[id] < 100)
    motor(id + 2, dir[id], 1);
  else if (movC[id] == 100)
    myservo[id + 2].write(180);
    dir[id] *= -1;
  else if (movC[id] < 200)
    motor(id + 2, dir[id], 1);
  else {
    movC[id] = 0;
    movPala[id] = false;
    return;
  }
  movC[id]++;
}

void motor(int id, int dir, int vel) {
  vel += 2;
  if (dir == 0)
    myservo[id].write(1500);
  else
    myservo[id].write(1500 + 20*vel*dir);
}

void createResponse(EthernetClient *client) {
  String M0 = estado[values[0]];
  String M1 = estado[values[1]];
  //\nRefresh: 2\n
  client->print("HTTP/1.1 200 OK\nContent-Type: text/html\nConnection: close\n\n<!DOCTYPE HTML>\n<html><title>Palas</title>\n<head></head><body><h1 align='center'>PALAS </h1><div style='text-align:center;'>");
  client->print("<b>Pala Izquierda</b><br /><button onClick=location.href='./?0=");
  if (values[0])
    client->print("0' >OFF</button><br /><br />");
  else
    client->print("1' >ON</button><br /><br />");
  client->print("<b>Velocidad: ");
  client->print(values[2]);
  client->print("</b><br />");
  client->print("<div class='slidecontainer'><input type='range' min='1' max='3' value='");
  client->print(values[2]);
  client->print("' class='slider' id='myRange' onChange='v(this.value, 2)'></div>");
  client->print("<div style='text-align:center;'>");
  client->print("<br /><br />");
  client->print("<b>Pala Derecha</b><br />");
  client->print("</b><br /><button onClick=location.href='./?1=");
  if (values[1])
    client->print("0' >OFF</button><br /><br />");
  else
    client->print("1' >ON</button><br /><br />");
  client->print("<b>Velocidad: ");
  client->print(values[3]);
  client->print("</b><br />");
  client->print("<div class='slidecontainer'><input type='range' min='1' max='3' value='");
  client->print(values[3]);
  client->print("' class='slider' id='myRange' onChange='v(this.value, 3)'></div>");
  client->print("</b></body>");
  // int sensorReading = analogRead(1);
  // client->print("analog input 1 is ");
  // client->print(sensorReading);
  // client->print("<br />");
  // sensorReading = analogRead(4);
  // client->print("analog input 4 is ");
  // client->print(sensorReading);
  client->print("<script> function v(n, i){ location.href='./?'+i+'=' + n;} </script>");
  client->print("</html>");

}
void loop() {
  EthernetClient client = server.available();
  if (client) {
    boolean currentLineIsBlank = true;
    String cadena="";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        cadena.concat(c);
        if (c == '\n' && currentLineIsBlank) {
          int posi=cadena.indexOf("/?");
          if (posi > 0) {
            String par = cadena.substring(posi + 2, posi + 3);
            String value = cadena.substring(posi + 4, posi + 5);
            values[par.toInt()] = value.toInt();
          }
          createResponse(&client);
          break;
        }
        if (c == '\n') {
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    delay(100);
    client.stop();
  }
  if (analogRead(1) < 512 && movPala[0] == false) {
    motor(0, 0, 0);
    movPala[0] = true;
  }

  if (movPala[0])
    moverPala(0);
  else
    motor(0, values[0]*dir[0], values[2]);

  if (analogRead(4) < 512 && movPala[1] == false) {
    motor(1, 0, 0);
    movPala[1] = true;
  }

  if (movPala[1])
    moverPala(1);
  else
    motor(1, values[1]*dir[1], values[3]);

}
