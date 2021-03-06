# https://github.com/SnelleJelle/SuperDuperProtocol/wiki/Protocol-specifcations

import flask
# https://pypi.python.org/pypi/Flask-Cors
from flask_cors import CORS, cross_origin
from termcolor import colored

from ServerMock.Robot import Robot

robot = Robot.Robot()
print(robot.get_parts())

app = flask.Flask(__name__)
CORS(app)


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

@app.route("/led/toggle/<led_id>")
def led_toggle(led_id):
    return robot.led_toggle(led_id)


@app.route("/wheel/forward/<wheel_id>")
def wheel_forward(wheel_id):
    return robot.wheel_forward(wheel_id)


@app.route("/wheel/backward/<wheel_id>")
def wheel_backward(wheel_id):
    return robot.wheel_backward(wheel_id)


@app.route("/wheel/stop/<wheel_id>")
def wheel_stop(wheel_id):
    return robot.wheel_stop(wheel_id)


@app.route("/distance_sensor/read/<sensor_id>")
def distance_sensor_read(sensor_id):
    return robot.distance_sensor_read(sensor_id)


print(colored("http server starting", "green"))
app.run(host="localhost", debug=False, port=8080)
