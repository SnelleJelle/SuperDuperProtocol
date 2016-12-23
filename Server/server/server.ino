#include <Ethernet.h>
#include <LiquidCrystal_I2C.h>
#include <SD.h>
#include <NewPing.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(169, 254, 255, 254); // use: 169.254.255.254:8080
EthernetServer server(8080);
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

const int chipSelect = 4;

// pins
String deviceTypes[] = {"empty", "empty", "button", "distance_sensor_echo", "distance_sensor", "bridge_enable", "led", "led", "wheel", "wheel_reverse", "wheel", "wheel_reverse"}; //type of device with id 5 => deviceTypes[5] = deviceType;

boolean ledStates[] = {false, false, false, false, false, false, false}; // state of led with id 2 => ledStates[2] = true/false
String wheelPositions[] = {"", "", "", "", "", "", "", "", "left", "", "right", ""};
// end pins

void setup() {
    Serial.begin(9600);
    while (!Serial) {;
    }

    if (!SD.begin(chipSelect)) {
        Serial.println("Card failed, or not present");
        return;
    }

    Ethernet.begin(mac, ip);
    server.begin();
    Serial.print("starting server at: ");
    Serial.println(Ethernet.localIP());

    pinMode(0, INPUT);
    attachInterrupt(0, button_interrupt, RISING);

    lcd.begin(16,2);
    lcd.backlight();
}

void loop() {
    EthernetClient client = server.available();
    if (client) {

        boolean currentLineIsBlank = true;
        String requestUrl = String("");
        boolean finishedReadingUrl = false;
        while (client.connected()) {
            if (client.available()) {
                char c = client.read();

                if (c == '\n' && !finishedReadingUrl) {
                    finishedReadingUrl = true;
                    requestUrl = requestUrl.substring(4, requestUrl.length() - 10);
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
    }
}

void button_interrupt() {
  stop_all_wheels();
}

void showOnLcd(String text) {
  lcd.setCursor(0,0);
  lcd.print(text);
}

String getCommandFromUrl(String url){
  int deviceIndex = url.indexOf('/', 1);
  int commandIndex = url.indexOf('/', deviceIndex + 1);
  return url.substring(0, commandIndex);
}

int getDeviceIdFromUrl(String url){
  int deviceIndex = url.indexOf('/', 1);
  int commandIndex = url.indexOf('/', deviceIndex + 1);
  int valueIndex = url.indexOf('/', commandIndex + 1);
  String deviceId = url.substring(commandIndex + 1, valueIndex);
  return deviceId.toInt();
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
    int deviceId = getDeviceIdFromUrl(url);
    Serial.println("command: " + command + ", deviceId: " + deviceId);
    showOnLcd(command);

    if (url == "/list") {
       serve_list(client);
    } else if (url == "/gui") {
       serve_gui(client); //bad way
       // serve_gui_seperated(client); //good way
    } else if (url == "/gui/css") {
       serve_gui_css(client);
    } else if (url == "/gui/js") {
       serve_gui_js(client);
    } else if (command == "/led/off") {
      led_off(client, deviceId);
    } else if (command == "/led/on") {
      led_on(client, deviceId);
    } else if (command == "/led/toggle") {
      led_toggle(client, deviceId);
    } else if (command == "/wheel/forward") {
      wheel_forward(client, deviceId);
    } else if (command == "/wheel/backward") {
      wheel_backward(client, deviceId);
    } else if (command == "/wheel/stop") {
      wheel_stop(client, deviceId);
    } else if (command == "/distance_sensor/read")  {
      read_distance(client, deviceId);
    } else {
      Serial.println("Unrecognised url: " + url);
      client.println("{ \"command\": \"error\" }");
    }
}

void serve_list(EthernetClient client) {
  Serial.println("Serving device list overview");
  client.println("{ \"parts\":[ { \"id\":\"6\", \"extras\":[ { \"state\":0 } ], \"type\":\"led\" }, { \"id\":\"7\", \"extras\":[ { \"state\":0 } ], \"type\":\"led\" }, { \"id\":\"8\", \"extras\":[ { \"direction\":\"stop\" }, { \"position\":\"left\" } ], \"type\":\"wheel\" }, { \"id\":\"10\", \"extras\":[ { \"direction\":\"stop\" }, { \"position\":\"right\" } ], \"type\":\"wheel\" }, { \"id\":\"4\", \"extras\":[ { \"distance\":0 } ], \"type\":\"distance_sensor\" } ] }");
}

void led_on(EthernetClient client, int led_id) {
  if (isLed(led_id)) {
      Serial.println("Turning on led: " + String(led_id));
      ledStates[led_id] = true;
      client.println("{ \"device\": { \"id\": \"" + String(led_id) + "\", \"type\": \"led\", \"extras\": [ { \"state\": 1 } ] }, \"command\": \"success\" }");
  } else {
    Serial.println("No led found with id: " + String(led_id));
  }
}

void led_off(EthernetClient client, int led_id) {
  if (isLed(led_id)) {
      Serial.println("Turning off led: " + String(led_id));
      ledStates[led_id] = false;
      client.println("{ \"device\": { \"id\": \"" + String(led_id) + "\", \"type\": \"led\", \"extras\": [ { \"state\": 0 } ] }, \"command\": \"success\" }");
  } else {
      Serial.println("No led found with id: " + String(led_id));
  }
}

void led_toggle(EthernetClient client, int led_id) {
  if (isLed(led_id)) {
      Serial.println("Toggling led: " + String(led_id));
      boolean ledState = ledStates[led_id];
      ledState = !ledState;
      ledStates[led_id] = ledState;
      String ledStateString = ledState == true ? "1" : "0";
      client.println("{ \"device\": { \"id\": \"" + String(led_id) + "\", \"type\": \"led\", \"extras\": [ { \"state\": " + ledStateString + " } ] }, \"command\": \"success\" }");
  } else {
      Serial.println("No led found with id: " + String(led_id));
  }
}

void stop_all_wheels() {
  digitalWrite(8, LOW);
  digitalWrite(9, LOW);
  digitalWrite(10, LOW);
  digitalWrite(11, LOW);
}

void wheel_forward(EthernetClient client, int wheel_id) {
  if (isWheel(wheel_id)) {
       stop_all_wheels();
       Serial.println("Turning wheel forward: " + String(wheel_id));
       digitalWrite(wheel_id, HIGH);
       String direction = "forward";
       String wheelPosition = wheelPositions[wheel_id];
       client.println("{ \"device\": { \"id\": \"" + String(wheel_id) + "\", \"type\": \"wheel\", \"extras\": [ { \"direction\": \"" + direction + "\" }, { \"position\": \"" + wheelPosition + "\" } ] }, \"command\": \"success\" }");
  } else {
       Serial.println("No wheel found with id: " + String(wheel_id));
  }
}

void wheel_backward(EthernetClient client, int wheel_id) {
  if (isWheel(wheel_id)) {
       stop_all_wheels();
       int reverse_wheel_id = wheel_id + 1;
       digitalWrite(reverse_wheel_id, HIGH);
       Serial.println("Turning wheel backward: " + String(wheel_id));
       String direction = "backward";
       String wheelPosition = wheelPositions[wheel_id];
       client.println("{ \"device\": { \"id\": \"" + String(wheel_id) + "\", \"type\": \"wheel\", \"extras\": [ { \"direction\": \"" + direction + "\" }, { \"position\": \"" + wheelPosition + "\" } ] }, \"command\": \"success\" }");
  } else {
       Serial.println("No wheel found with id: " + String(wheel_id));
  }
}

void wheel_stop(EthernetClient client, int wheel_id) {
  if (isWheel(wheel_id)) {
       Serial.println("Stopping wheel: " + String(wheel_id));
       digitalWrite(wheel_id, LOW);
       int reverse_wheel_id = wheel_id + 1;
       digitalWrite(reverse_wheel_id, LOW);
       String direction = "stop";
       String wheelPosition = wheelPositions[wheel_id];
       client.println("{ \"device\": { \"id\": \"" + String(wheel_id) + "\", \"type\": \"wheel\", \"extras\": [ { \"direction\": \"" + direction + "\" }, { \"position\": \"" + wheelPosition + "\" } ] }, \"command\": \"success\" }");
  } else {
       Serial.println("No wheel found with id: " + String(wheel_id));
  }
}

void read_distance(EthernetClient client, int sensor_id) {
  if (isDistanceSensor(sensor_id)) {
      Serial.println("Reading distance from sensor: " + String(sensor_id));
      int trigger = sensor_id;
      int echo = sensor_id - 1;

      pinMode(trigger, OUTPUT);
      digitalWrite(trigger, LOW);
      delayMicroseconds(2);
      digitalWrite(trigger, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigger, LOW);
      pinMode(echo, INPUT);
      int duration = pulseIn(echo, HIGH);

      int distance = duration / 29 / 2;
      client.println("{ \"device\": { \"id\": \"" + String(sensor_id) + "\", \"type\": \"distance_sensor\", \"extras\": [ { \"distance\": " + String(distance) + " } ] }, \"command\": \"success\" }");
  } else {
      Serial.println("No sensor found with id: " + String(sensor_id));
  }
}

boolean isLed(int led_id) {
  return deviceTypes[led_id] == "led";
}

boolean isWheel(int wheel_id) {
  return deviceTypes[wheel_id] == "wheel";
}

boolean isDistanceSensor(int sensor_id) {
  return deviceTypes[sensor_id] == "distance_sensor";
}

// when possible use serve_gui_seperated(client); instead
void serve_gui(EthernetClient client) {
  client.println("<!DOCTYPE html>\r\n<html lang=\"en\">\r\n<head>\r\n    <meta charset=\"UTF-8\">\r\n\r\n    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\r\n\r\n    <!--jQuery-->\r\n    <script src=\"https://code.jquery.com/jquery-3.1.1.min.js\" integrity=\"sha256-hVVnYaiADRTO2PzUGmuLJr8BLUSjGIZsDYGmIJLv2b8=\" crossorigin=\"anonymous\"></script>\r\n\r\n    <!-- bootstrap -->\r\n    <link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css\" integrity=\"sha384-BVYiiSIFeK1dGmJRAkycuHAHRg32OmUcww7on3RYdg4Va+PmSTsz/K68vbdEjh4u\" crossorigin=\"anonymous\">\r\n    <script src=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js\" integrity=\"sha384-Tc5IQib027qvyjSMfHjOMaLkfuWVxZxUPnCJA7l2mCWNIpG9mGCD8wGNIcPD7Txa\" crossorigin=\"anonymous\"></script>\r\n    <!-- bootstrap theme -->\r\n    <link rel=\"stylesheet\" href=\"https://bootswatch.com/slate/bootstrap.min.css\">\r\n    <!--bootstrap slider-->\r\n    <link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/bootstrap-slider/9.5.1/css/bootstrap-slider.min.css\">\r\n    <script src=\"https://cdnjs.cloudflare.com/ajax/libs/bootstrap-slider/9.5.1/bootstrap-slider.min.js\"></script>\r\n\r\n    <style>\r\n\r\n        html {\r\n            min-width: 450px;\r\n        }\r\n\r\n        button {\r\n            background-color: rgb(91, 192, 222);\r\n            border: none;\r\n            color: white;\r\n            padding: 15px 32px;\r\n            text-align: center;\r\n            text-decoration: none;\r\n            display: inline-block;\r\n            font-size: 16px;\r\n            margin: 4px 2px;\r\n            cursor: pointer;\r\n        }\r\n\r\n        .badge {\r\n            background-color: rgb(91, 192, 222);\r\n        }\r\n\r\n        button:hover {\r\n            background-color: #147CCC;\r\n        }\r\n\r\n        .panel-body button.button-up {\r\n            display: block;\r\n            margin: 0 0 3px 90px;\r\n        }\r\n\r\n        .panel-body button.button-stop {\r\n            background-color: #FF6140;\r\n            margin: 4px 2px 3px 90px;\r\n            width: 80px;\r\n            padding: 15px 0;\r\n        }\r\n\r\n        button.button-stop:hover {\r\n            background-color: #CC1A14;\r\n        }\r\n\r\n        .driving-controls{\r\n            width: 300px;\r\n            margin: 0 auto;\r\n        }\r\n\r\n        .panel-info {\r\n            margin: 5px;\r\n        }\r\n\r\n        .led {\r\n            padding: 5px;\r\n            display: inline;\r\n        }\r\n\r\n        .distance_value {\r\n            background-color: #FF6140;\r\n            border-radius: 6px;\r\n            padding: 5px;\r\n            margin-left: 10px;\r\n        }\r\n\r\n        .sensor-controls > a {\r\n            padding-left: 4px;\r\n            margin: 5px;\r\n            width: 125px;\r\n            border-radius: 0px;\r\n        }\r\n\r\n        #btn_manual {\r\n            float: left;\r\n        }\r\n\r\n        #panel-driving-settings,\r\n        #panel-sensor-controls,\r\n        #panel-led-controls {\r\n            display: none;\r\n        }\r\n\r\n        #distance_setting_slider {\r\n            margin: 25px 10px 10px;\r\n        }\r\n\r\n        #distance_setting_slider .slider-handle{\r\n            background: #FF6140;\r\n        }\r\n\r\n        #distance_setting_slider .slider-selection {\r\n            background: rgb(91, 192, 222);\r\n        }\r\n\r\n        .driving-settings > span {\r\n            padding-top: 25px;\r\n            display: block;\r\n            float: left;\r\n        }\r\n\r\n    </style>\r\n\r\n    <title>IOT Jelle Criel</title>\r\n</head>\r\n<body class=\"modal-open\">\r\n\r\n<div class=\"container col-md-12\">\r\n\r\n    <div class=\"modal fade\" id=\"connect_modal\" role=\"dialog\">\r\n        <div class=\"modal-dialog modal-lg\">\r\n            <div class=\"modal-content\">\r\n                <form>\r\n                    <div class=\"modal-body\">\r\n                        <div class=\"input-group\">\r\n                            <span class=\"input-group-addon\">Address</span>\r\n                            <input id=\"connect_address\" type=\"text\" class=\"form-control\" name=\"msg\" placeholder=\"Connect to address\" required=\"\">\r\n                        </div>\r\n                    </div>\r\n                    <div class=\"modal-footer\">\r\n                        <button type=\"submit\" class=\"btn btn-default\" id=\"btn_manual\" data-dismiss=\"modal\" onclick='connect(\"manual\");'>Connect with manual control</button>\r\n                        <button type=\"submit\" class=\"btn btn-default\" id=\"btn_automatic\" data-dismiss=\"modal\" onclick='connect(\"automatic\");'>Connect with automatic control</button>\r\n                    </div>\r\n                </form>\r\n            </div>\r\n        </div>\r\n    </div>\r\n\r\n    <div class=\"jumbotron\">\r\n        <h1><img src=\"https://upload.wikimedia.org/wikipedia/commons/thumb/8/87/Arduino_Logo.svg/1280px-Arduino_Logo.svg.png\" height=\"100\"/> IOT Jelle Criel</h1>\r\n        <p>Robot graphical control panel</p>\r\n    </div>\r\n\r\n    <div class=\"row col-md-3\">\r\n        <div class=\"panel panel-info\">\r\n            <div class=\"panel-heading\">\r\n                Connected devices\r\n            </div>\r\n            <div class=\"panel-body\">\r\n                <div class=\"connected-devices\">\r\n                    <ul class=\"list-group\">\r\n                    </ul>\r\n                </div>\r\n            </div>\r\n        </div>\r\n    </div>\r\n\r\n    <div class=\"row col-md-3\">\r\n        <div class=\"panel panel-info\">\r\n            <div class=\"panel-heading\">\r\n                Driving controls\r\n            </div>\r\n            <div class=\"panel-body\">\r\n                <div class=\"driving-controls\">\r\n\r\n                    <button type=\"button\" class=\"button-up\">\r\n                        <span class=\"glyphicon glyphicon-arrow-up\"></span>\r\n                    </button>\r\n\r\n                    <button type=\"button\" class=\"button-left\">\r\n                        <span class=\"glyphicon glyphicon-arrow-left\"></span>\r\n                    </button>\r\n\r\n                    <button type=\"button\" class=\"button-down\">\r\n                        <span class=\"glyphicon glyphicon-arrow-down\"></span>\r\n                    </button>\r\n\r\n                    <button type=\"button\" class=\"button-right\">\r\n                        <span class=\"glyphicon glyphicon-arrow-right\"></span>\r\n                    </button>\r\n\r\n                    <button type=\"button\" class=\"button-stop\">\r\n                        STOP\r\n                    </button>\r\n\r\n                </div>\r\n            </div>\r\n        </div>\r\n    </div>\r\n\r\n    <div class=\"row col-md-3\" id=\"panel-led-controls\">\r\n        <div class=\"panel panel-info\">\r\n            <div class=\"panel-heading\">\r\n                LED controls\r\n            </div>\r\n            <div class=\"panel-body\">\r\n                <div class=\"led-controls\">\r\n                </div>\r\n            </div>\r\n        </div>\r\n    </div>\r\n\r\n    <div class=\"row col-md-3\" id=\"panel-sensor-controls\">\r\n        <div class=\"panel panel-info\">\r\n            <div class=\"panel-heading\">\r\n                Distance sensor controls\r\n            </div>\r\n            <div class=\"panel-body\">\r\n                <div class=\"sensor-controls\">\r\n                </div>\r\n            </div>\r\n        </div>\r\n    </div>\r\n\r\n    <div class=\"row col-md-3\" id=\"panel-driving-settings\">\r\n        <div class=\"panel panel-info\">\r\n            <div class=\"panel-heading\">\r\n                Driving settings\r\n            </div>\r\n            <div class=\"panel-body\">\r\n                <div class=\"driving-settings\">\r\n                    <div id=\"invis-padding\"></div>\r\n                    <span>Automatic break at: </span>\r\n                    <input id=\"distance_setting\" data-slider-id='distance_setting_slider' type=\"text\" data-slider-min=\"0\" data-slider-max=\"100\" data-slider-step=\"10\" data-slider-value=\"10\"/>\r\n                </div>\r\n            </div>\r\n        </div>\r\n    </div>\r\n\r\n</div>\r\n\r\n<!--preload images-->\r\n<img src=\"http://www.downloadclipart.net/large/18483-led-red-off-design.png\" style=\"display: none\">\r\n<img src=\"http://www.clker.com/cliparts/v/C/V/N/w/e/led-red-control-hi.png\" style=\"display: none\">\r\n\r\n<script>\r\nlet host = \"\";\r\nlet connect_mode = \"\";\r\n\r\nlet mode_automatic = \"automatic\";\r\nlet mode_manual = \"manual\";\r\n\r\nlet led_src_on = \"http://www.clker.com/cliparts/v/C/V/N/w/e/led-red-control-hi.png\";\r\nlet led_src_off = \"http://www.clker.com/cliparts/q/2/m/P/e/I/red-led-off-small-md.png\";\r\n\r\nlet connected_devices = [];\r\nlet left_wheels = [];\r\nlet right_wheels = [];\r\nlet leds = [];\r\nlet distance_sensors = [];\r\n\r\nlet driving = false;\r\nlet break_distance = 10;\r\n\r\n$(document).ready(function() {\r\n    $(\"#connect_modal\").modal();\r\n    $(\"#connect_address\").val(window.location.host); //window.location.host\r\n\r\n});\r\n\r\nfunction connect(mode) {\r\n    connect_mode = mode;\r\n    host = window.location.protocol + \"//\" +  $(\"#connect_address\").val();\r\n    populate_connected_devices();\r\n\r\n    assign_driving_events(connect_mode);\r\n}\r\n\r\nfunction load_leds() {\r\n\r\n    var $led_controls = $(\".led-controls\");\r\n\r\n    $led_controls.html(\"\");\r\n    leds.forEach(function (led) {\r\n        var led_state = led.extras.find(e => e).state;\r\n        var led_img_src = led_state == 1 ? led_src_on : led_src_off;\r\n        var led_html = `<div class=\"led\"><span class=\"badge\">${led.id}</span><img id=\"led_${led.id}\" width=\"75\" src=\"${led_img_src}\" onclick=\"led_onclick(this.id)\"></div>`;\r\n        $led_controls.append(led_html);\r\n    });\r\n\r\n    $(\"#panel-led-controls\").show();\r\n}\r\n\r\nfunction load_sensors() {\r\n    var $sensor_controls = $(\".sensor-controls\");\r\n\r\n    $sensor_controls.html(\"\");\r\n    distance_sensors.forEach(function (sensor) {\r\n        var distance = sensor.extras.find(e => e).distance;\r\n        var sensor_html = `<a id=\"sensor_${sensor.id}\" class=\"btn btn-info btn-lg\" onclick=\"refresh_sensor(this.id);\"><span class=\"badge\">${sensor.id}</span> <span class=\"distance_value\">${distance}</span> <span class=\"glyphicon glyphicon-refresh\"></span></a>`;\r\n        $sensor_controls.append(sensor_html);\r\n    })\r\n\r\n    $(\"#panel-sensor-controls\").show();\r\n}\r\n\r\nfunction load_driving_settings() {\r\n    $(\"#panel-driving-settings\").show();\r\n\r\n    var slider = $(\"#distance_setting\").slider({\r\n        tooltip: 'always',\r\n        formatter: function(value) {\r\n            return value + ' cm';\r\n        }\r\n    });\r\n\r\n    slider.on(\"slide\", function(slideEvt) {\r\n        break_distance = slideEvt.value;\r\n    });\r\n}\r\n\r\nfunction populate_connected_devices() {\r\n    $('.connected-devices > .list-group').html(\"\");\r\n\r\n    $.ajax({\r\n        url: host + \"/list\",\r\n        success: function (result) {\r\n            connected_devices = JSON.parse(result).parts;\r\n            connected_devices.forEach(function (part) {\r\n                add_part_to_collection(part);\r\n                add_part_to_overview(part);\r\n            });\r\n\r\n            if (connect_mode == mode_manual) {\r\n                load_leds();\r\n                load_sensors();\r\n            } else if (connect_mode = mode_automatic) {\r\n                load_driving_settings();\r\n            }\r\n        },\r\n        error : function (error) {\r\n            console.log(error);\r\n        }\r\n    });\r\n}\r\n\r\nfunction assign_driving_events(mode) {\r\n\r\n\r\n        var btn_left = $(\".button-left\");\r\n        btn_left.mousedown(button_press_left);\r\n        btn_left.mouseup(button_leave_left);\r\n\r\n        var btn_right = $(\".button-right\");\r\n        btn_right.mousedown(button_press_right);\r\n        btn_right.mouseup(button_leave_right);\r\n\r\n        var btn_up = $(\".button-up\");\r\n        btn_up.mousedown(button_press_up);\r\n        btn_up.mouseup(button_leave_up);\r\n\r\n        var btn_down = $(\".button-down\");\r\n        btn_down.mousedown(button_press_down);\r\n        btn_down.mouseup(button_leave_down);\r\n\r\n        $(\".button-stop\").mousedown(button_press_stop);\r\n    if (mode == mode_manual) {\r\n        //\r\n    } else if (mode == mode_automatic) {\r\n        btn_left.mousedown(function () {\r\n            sendCommand(\"/led/on/1\")\r\n        });\r\n        btn_left.mouseup(function () {\r\n            sendCommand(\"/led/off/1\")\r\n        });\r\n\r\n        btn_right.mousedown(function () {\r\n            sendCommand(\"/led/on/2\")\r\n        });\r\n        btn_right.mouseup(function () {\r\n            sendCommand(\"/led/off/2\")\r\n        });\r\n\r\n        btn_up.mousedown(function () {\r\n            sendCommand(\"/led/on/1\")\r\n            sendCommand(\"/led/on/2\")\r\n        });\r\n        btn_up.mouseup(function () {\r\n            sendCommand(\"/led/off/1\")\r\n            sendCommand(\"/led/off/2\")\r\n        });\r\n\r\n        var upDownLeftRight = $(\".driving-controls > button:not(.button-stop)\");\r\n        upDownLeftRight.mousedown(function () {\r\n            enable_distance_check();\r\n        });\r\n        upDownLeftRight.mouseup(function () {\r\n            disable_distance_check();\r\n        });\r\n    }\r\n}\r\n\r\nfunction enable_distance_check() {\r\n    driving = true;\r\n    setTimeout(function () {\r\n        distance_check();\r\n        if (driving) {\r\n            enable_distance_check();\r\n        }\r\n    },333);\r\n}\r\n\r\nfunction disable_distance_check() {\r\n    driving = false;\r\n}\r\n\r\nfunction distance_check() {\r\n\r\n    var sensor = sendCommand(\"/distance_sensor/read/5\");\r\n    var distance = sensor.extras.find(e => e.distance).distance;\r\n    on_distance_polled(distance);\r\n}\r\n\r\nfunction on_distance_polled(distance) {\r\n    if (distance <= break_distance) {\r\n        button_press_stop();\r\n        disable_distance_check();\r\n    }\r\n}\r\n\r\nfunction add_part_to_overview(part) {\r\n    var li = `<li class=\"list-group-item\" id=\"device_${part.id}\"><span class=\"label label-info\">${part.id}</span> ${part.type}`;\r\n\r\n    if (part.extras) {\r\n        part.extras.forEach(function (extra) {\r\n            var key = Object.keys(extra)[0];\r\n            var value = extra[key];\r\n            li += `<span class=\"badge\">${key}: ${value}</span>`;\r\n        });\r\n    }\r\n    li += '</li>';\r\n    $('.connected-devices > .list-group').append(li);\r\n}\r\n\r\nfunction add_part_to_collection(part) {\r\n\r\n    if (part.type == \"wheel\") {\r\n        var position = part.extras.find(e => e.position).position;\r\n        if (position == \"left\") {\r\n            left_wheels.push(part);\r\n        } else if (position == \"right\") {\r\n            right_wheels.push(part);\r\n        }\r\n    } else if (part.type == \"led\") {\r\n        leds.push(part);\r\n    } else if (part.type == \"distance_sensor\") {\r\n        distance_sensors.push(part);\r\n    }\r\n}\r\n\r\nfunction sendCommand(command) {\r\n    var request = host + command;\r\n    //console.log(\"requesting: \" + request);\r\n\r\n    var device;\r\n    $.ajax({\r\n        url: request,\r\n        dataType: \"json\",\r\n        async: false,\r\n        success: function (result) {\r\n            device = result.device;\r\n            if (result.command == \"success\") {\r\n                update_overview(result.device);\r\n            }\r\n        },\r\n        error : function (error) {\r\n            console.log(\"error: \" + error);\r\n        }\r\n    });\r\n\r\n    return device;\r\n}\r\n\r\nfunction update_overview(newpart) {\r\n    var device_id = \"#device_\" + newpart.id;\r\n    $(device_id + ' > .badge').remove();\r\n\r\n    var badge_ids = [];\r\n    newpart.extras.forEach(function (extra) {\r\n        var badgeHtml = \"\";\r\n        var key = Object.keys(extra)[0];\r\n        var value = extra[key];\r\n        var random_badge_id = \"random_badge_id_\" + Math.random().toString(36).substr(2, 10);\r\n        badgeHtml += `<span class=\"badge\" id=\"${random_badge_id}\">${key}: ${value}</span>`;\r\n        $(device_id).append(badgeHtml);\r\n\r\n        badge_ids.push(random_badge_id);\r\n\r\n        var badge = $('#' + random_badge_id);\r\n        badge.fadeOut(200).fadeIn(200)\r\n    });\r\n}\r\n\r\nfunction button_press_up() {\r\n    left_wheels.forEach(function (wheel) {\r\n        sendCommand(\"/wheel/forward/\" + wheel.id);\r\n    });\r\n    right_wheels.forEach(function (wheel) {\r\n        sendCommand(\"/wheel/forward/\" + wheel.id);\r\n    })\r\n}\r\nfunction button_leave_up() {\r\n    left_wheels.forEach(function (wheel) {\r\n        sendCommand(\"/wheel/stop/\" + wheel.id);\r\n    });\r\n    right_wheels.forEach(function (wheel) {\r\n        sendCommand(\"/wheel/stop/\" + wheel.id);\r\n    })\r\n}\r\n\r\nfunction button_press_down() {\r\n    left_wheels.forEach(function (wheel) {\r\n        sendCommand(\"/wheel/backward/\" + wheel.id);\r\n    });\r\n    right_wheels.forEach(function (wheel) {\r\n        sendCommand(\"/wheel/backward/\" + wheel.id);\r\n    })\r\n}\r\nfunction button_leave_down() {\r\n    left_wheels.forEach(function (wheel) {\r\n        sendCommand(\"/wheel/stop/\" + wheel.id);\r\n    });\r\n    right_wheels.forEach(function (wheel) {\r\n        sendCommand(\"/wheel/stop/\" + wheel.id);\r\n    })\r\n}\r\n\r\nfunction button_press_left() {\r\n    left_wheels.forEach(function (wheel) {\r\n        sendCommand(\"/wheel/forward/\" + wheel.id);\r\n    });\r\n}\r\nfunction button_leave_left() {\r\n    left_wheels.forEach(function (wheel) {\r\n        sendCommand(\"/wheel/stop/\" + wheel.id);\r\n    });\r\n}\r\n\r\nfunction button_press_right() {\r\n    right_wheels.forEach(function (wheel) {\r\n        sendCommand(\"/wheel/forward/\" + wheel.id);\r\n    })\r\n}\r\nfunction button_leave_right() {\r\n    right_wheels.forEach(function (wheel) {\r\n        sendCommand(\"/wheel/stop/\" + wheel.id);\r\n    })\r\n}\r\n\r\nfunction button_press_stop() {\r\n    left_wheels.forEach(function (wheel) {\r\n        sendCommand(\"/wheel/stop/\" + wheel.id);\r\n    });\r\n    right_wheels.forEach(function (wheel) {\r\n        sendCommand(\"/wheel/stop/\" + wheel.id);\r\n    })\r\n}\r\n\r\nfunction led_onclick(clicked_id) {\r\n    var id = clicked_id.split(\"_\").pop();\r\n\r\n    var led = sendCommand(\"/led/toggle/\" + id);\r\n\r\n    var led_state = led.extras.find(e => e).state;\r\n    var led_img_src = led_state == 1 ? led_src_on : led_src_off ;\r\n    $(\"#led_\" + led.id).attr(\"src\", led_img_src);\r\n}\r\n\r\nfunction refresh_sensor(clicked_id) {\r\n    var id = clicked_id.split(\"_\").pop();\r\n\r\n    var sensor = sendCommand(\"/distance_sensor/read/\" + id);\r\n\r\n    var distance = sensor.extras.find(e => e).distance;\r\n\r\n    $(\"#sensor_\" + id + \" > .distance_value\").html(distance);\r\n}\r\n\r\n</script>\r\n</body>\r\n</html>");
  Serial.println("Serving gui");
}

// requires files to be put on fs
// sd card in FAT fs
// filenames is 8.3 format
void serve_gui_seperated(EthernetClient client) {
  serve_file(client, "gui.html");
  Serial.println("Serving gui");
}

void serve_gui_css(EthernetClient client) {
  serve_file(client, "style.css");
  Serial.println("Serving gui css");
}

void serve_gui_js(EthernetClient client) {
  serve_file(client, "script.js");
  Serial.println("Serving gui js");
}

void serve_file(EthernetClient client, String fileName) {
  char filename[fileName.length()+1];
  fileName.toCharArray(filename, sizeof(filename));
  File dataFile = SD.open(filename);
  if (dataFile) {
    while (dataFile.available()) {
      client.print(dataFile.read());
    }
    dataFile.close();
  }
  else {
    client.print("Error reading file: " + fileName);
  }
}
