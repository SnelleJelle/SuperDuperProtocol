# SuperDuperProtocol

An iot assignment to create our own protocol ...based on HTTP for ease sake.

## Table of contents
<!-- TOC depthFrom:1 depthTo:6 withLinks:1 updateOnSave:1 orderedList:0 -->

- [SuperDuperProtocol](#superduperprotocol)
	- [Table of contents](#table-of-contents)
	- [HTTP](#http)
	- [Actors & practices](#actors-practices)
	- [Robot Command specification](#robot-command-specification)
		- [List connected devices](#list-connected-devices)
		- [Led controls](#led-controls)
		- [Wheel (driving) controls](#wheel-driving-controls)
		- [Distance sensor controls](#distance-sensor-controls)
		- [Connection controls](#connection-controls)

<!-- /TOC -->

## HTTP

This protocol uses HTTP which in turn uses TCP/IP for connection handling and reliability. HTTP is a reliable and common protocol, it has client and server implementations in every programming language. I use HTTP because it is easy to work with and I'm a lazy person. _Let's not reinvent the wheel..._

## Actors & practices

**Server:** Intel Galileo iot device with HTTP-server.

**Client:** Laptop with HTTP-client.

**Connection:** Both devices are connected to the same LAN with a network cable.

**Protocol** uses HTTP body and URL to specify commands and arguments.

**Server development** based on the Arduino ide samples for HTTP-server.

## Robot Command specification

The client device (laptop with HTTP-client) sends HTTP-requests to the server (Intel Galileo).<br>
Requests are of the format: `http://host:port/command/argument(s)` and are specified below.<br>
The server replies with {} default or a JSON-object when more info is required.

Possible values for `command` and `arguments` are specified below, grouped per target component.

### List connected devices
Command | Arguments | Description                     | Reply body
------- | --------- | ------------------------------- | --------------------------------------------------------------------------------------
list    |           | Lists the connected devices by name and their associated ID's  | The device info is returned in JSON format.

### Led controls
Command | Arguments | Description                     | Reply body
------- | --------- | ------------------------------- | --------------------------------------------------------------------------------------
led     | /on/{id}  | Turns the led with id {id} on.  | "ok" when action is executed successfully. "error" when led with id {id} is not found.
led     | /off/{id} | Turns the led with id {id} off. | "ok" when action is executed successfully. "error" when led with id {id} is not found.

### Wheel (driving) controls

Command | Arguments | Description                       | Reply body
------- | --------- | --------------------------------- | ----------------------------------------------------------------------------------------
wheel   | /forward/{id}  | Turns the wheel with id {id} forwards.  | "ok" when action is executed successfully. "error" when wheel with id {id} is not found.
wheel   | /backward/{id}  | Turns the wheel with id {id} backwards.  | "ok" when action is executed successfully. "error" when wheel with id {id} is not found.
wheel   | /stop/{id} | Turns the wheel with id {id} off. | "ok" when action is executed successfully. "error" when wheel with id {id} is not found.
wheel   | /fullstop | Stops all wheels | "ok" when action is executed successfully. "error" when no wheels are found.

### Distance sensor controls

Command        | Arguments | Description                       | Reply body
-------------- | --------- | --------------------------------- | ----------------
distance_sensor | /read/{id}  |Reads the distance from distance sensor with id {id}  | {distance} when action is executed successfully. "error" when distance sensor with id {id} is not found.

### Connection controls

Command           | Arguments | Description | Reply body
----------------- | --------- | ----------- | ----------
connect           | /to/{ipv4} | Initialises a connection to the device with the given address {ipv4} in IPv4 notation | "connected" when sucessfully connected, nothing when connection failed.
