#include <Ethernet.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(169, 254, 255, 254); //169.254.255.254
EthernetServer server(8080);

void setup() {
    Serial.begin(9600);
    while (!Serial) {;
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
                    handle_request(requestUrl, client);
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

String getCommandFromUrl(String url){
  int deviceIndex = url.indexOf('/', 1);
  int commandIndex = url.indexOf('/', deviceIndex + 1);
  return url.substring(0, commandIndex);
}

String getvalueFromUrl(String url){
  int deviceIndex = url.indexOf('/', 1);
  int commandIndex = url.indexOf('/', deviceIndex + 1);
  int valueIndex = url.indexOf('/', commandIndex + 1);
  return url.substring(commandIndex + 1, valueIndex);
}

void send_headers(EthernetClient client) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println("Access-Control-Allow-Origin: *");
    client.println();
}

void handle_request(String url, EthernetClient client) {
    send_headers(client);

    String command = getCommandFromUrl(url);
    client.println("command: " + command);
    if (command = "/led/on") {
      String value = getvalueFromUrl(url);
      led_on(value);
    } else if (command = "/led/off") {
      String value = getvalueFromUrl(url);
      led_off(value);

    }

    client.println("value: " + value);
    //client.print("{ \"device\": { \"extras\": [ { \"state\": 1 } ], \"id\": \"1\", \"type\": \"led\" }, \"command\": \"success\" }");

}

void led_on() {

}

void led_off() {

}
  
