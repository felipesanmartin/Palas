#include <Ethernet2.h>
#include <Servo.h>

Servo myservo[4];
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(10, 42, 82, 190);

EthernetServer server(80);
String estado[2] = {"OFF", "ON"};
int dir[2] = {1, 1};
int movC[2] = {0, 0};
boolean movPala[2] = {false, false};
boolean sensor[2] = {false, false};
int s_pins[2] = {4, 1};

// Values to M0, M1, V0, V1
int values[] = {0, 0, 3, 3, 0};

void setup() {
  myservo[0].attach(2, 750, 2250);  // attaches the servo on pin 9 to the servo object
  myservo[0].write(1505);
  myservo[1].attach(3, 750, 2250);  // attaches the servo on pin 9 to the servo object
  myservo[1].write(1500);
  myservo[2].attach(5);  // attaches the servo on pin 9 to the servo object
  myservo[2].write(130);
  myservo[3].attach(6);  // attaches the servo on pin 9 to the servo object
  myservo[3].write(122);

  Ethernet.begin(mac, ip);
  server.begin();
}

void cargarPala(int id) {
  if (movC[id] < 1000)
    motor(id, dir[id], 1);
  else if (movC[id] == 1000){
    Serial.println("CARGA");
    motor(id, 0, 0);
    if (id == 1)
      myservo[id + 2].write(157 - 8);
    else
      myservo[id + 2].write(157);
  }
  else if (movC[id] < 6000){
    if (values[4] == 1){
      if (movC[id] % 200 == 0){
        if (id == 1)
          myservo[id + 2].write(152 - 8);
        else
          myservo[id + 2].write(152);
      }
      else{
      if (movC[id] % 100 == 0){
        if (id == 1)
          myservo[id + 2].write(157 - 8);
        else
          myservo[id + 2].write(157);
        }
      }
    }
  else if (movC[id] == 6000){
    Serial.println("ASD2");
    if (id == 1)
      myservo[id + 2].write(157 - 8);
    else
      myservo[id + 2].write(157);
  }
  }
  else if (movC[id] < 14000)
    motor(id, -dir[id], 1);
  else {
    Serial.println("ASD");
    movC[id] = 0;
    movPala[id] = false;
    dir[id] *= -1;
    Serial.println(id);
    Serial.println(movC[id]);
    Serial.println(dir[id]);
    return;
  }
  movC[id]++;
}

void descargarPala(int id) {
  if (movC[id] < 1800)
    motor(id, dir[id], 1);
  else if (movC[id] == 2000){
    Serial.println("DESCARGA");
    motor(id, 0, 0);
    if (id == 1)
      myservo[id + 2].write(109 - 8);
    else
      myservo[id + 2].write(109);
  }
  else if (movC[id] < 12000){
    if (movC[id] % 200 == 0){
      if (id == 1)
        myservo[id + 2].write(114 - 8);
      else
        myservo[id + 2].write(114);
    }
    else{
    if (movC[id] % 100 == 0){
      if (id == 1)
        myservo[id + 2].write(109 - 8);
      else
        myservo[id + 2].write(109);
      }
    }

  }
  else if (movC[id] == 12000){
    Serial.println("ASD2");
    if (id == 1)
      myservo[id + 2].write(130 - 8);
    else
      myservo[id + 2].write(130);
  }
  else if (movC[id] < 18000)
    motor(id, -dir[id], 1);
  else {
    movC[id] = 0;
    movPala[id] = false;

    dir[id] *= -1;
    Serial.println(id);
    Serial.println(movC[id]);
    Serial.println(dir[id]);
    return;
  }
  movC[id]++;
}

void motor(int id, int dir, int vel) {
  vel += 2;
  if (id == 1)
    myservo[id].write(1505 + 20*vel*dir);
  else
    myservo[id].write(1505 + 20*vel*dir);

}

void pala(int id) {
  if (values[id] == 0) {
    motor(id, 0, 0);
    return;
  }
  if (analogRead(s_pins[id]) > 512) {
    motor(id, 0, 0);
    movPala[id] = true;
    //sensor[id] = !sensor[id];
  }
//  Serial.println(dir[id]);
  if (movPala[id])
    if (dir[id] == 1){
      //Serial.println("Perez");
      cargarPala(id);
    }
    else
      descargarPala(id);
  else
    motor(id, dir[id], values[id + 2]);

}

void readResponse() {
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
            int par = cadena.substring(posi + 2, posi + 3).toInt();
            String value = cadena.substring(posi + 4, posi + 5);
            if (par > 3 && par < 6)
              movPala[par - 4] = true;
            else
              values[par] = value.toInt();
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

  client->print("</b><br /><button onClick=location.href='./?4=");
  client->print("1' >Levantar</button><br /><br />");

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

  client->print("</b><br /><button onClick=location.href='./?5=");
  client->print("1' >Levantar</button><br /><br />");

  client->print("<b>Velocidad: ");
  client->print(values[3]);
  client->print("</b><br />");
  client->print("<div class='slidecontainer'><input type='range' min='1' max='3' value='");
  client->print(values[3]);
  client->print("' class='slider' id='myRange' onChange='v(this.value, 3)'></div>");

  client->print("<b>Vibrar en la carga</b><br />");
  client->print("</b><br /><button onClick=location.href='./?6=");
  if (values[4])
    client->print("0' >OFF</button><br /><br />");
  else
    client->print("1' >ON</button><br /><br />");

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
  readResponse();

  pala(0);
  pala(1);
}
