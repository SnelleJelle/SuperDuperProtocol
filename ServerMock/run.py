# https://github.com/SnelleJelle/SuperDuperProtocol/wiki/Protocol-specifcations

import flask
from termcolor import colored

from ServerMock.Robot import Robot

robot = Robot.Robot()
print(robot.get_parts())

app = flask.Flask(__name__)


@app.route("/")
def index():
    return "IT WORKS"


@app.route("/list")
def list():
    return robot.get_parts()


@app.route("/led/on/<led_id>")
def led_on(led_id):
    return robot.led_on(led_id)


@app.route("/led/off/<led_id>")
def led_off(led_id):
    return robot.led_off(led_id)


@app.route("/wheel/forward/<wheel_id>")
def wheel_forward(wheel_id):
    return robot.wheel_forward(wheel_id)


@app.route("/wheel/backward/<wheel_id>")
def wheel_backward(wheel_id):
    return robot.wheel_backward(wheel_id)


@app.route("/wheel/stop/<wheel_id>")
def wheel_stop(wheel_id):
    return robot.wheel_stop(wheel_id)


@app.route("/wheel/fullstop")
def wheel_fullstop():
    return robot.wheel_fullstop()


@app.route("/distance_sensor/read/<sensor_id>")
def distance_sensor_read(sensor_id):
    return robot.distance_sensor_read(sensor_id)


print(colored("http server starting", "green"))
app.run(host="localhost", debug=False, port=8080)
