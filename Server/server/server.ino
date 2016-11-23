#include <Ethernet.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(169, 254, 255, 254);
EthernetServer server(8080);

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ;
  }

  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}

void loop() {
    EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");

    boolean currentLineIsBlank = true;
    String requestUrl = String("");
    boolean finishedReadingUrl = false;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();

        if (c == '\n' && !finishedReadingUrl) {
            finishedReadingUrl = true;
            requestUrl = requestUrl.substring(4, requestUrl.length() - 10);
            Serial.println("Request for: " + requestUrl);
        }

        if (!finishedReadingUrl) {
            requestUrl += c;
        }

        if (c == '\n' && currentLineIsBlank) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println();

          client.print("derp");
          break;
        }
        if (c == '\n') {
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    delay(1);
    client.stop();
    Serial.println("client disconnected");
  }
}
