from DistanceSensor import DistanceSensor
from Led import Led
from Part import Part
from Wheel import Wheel


class Robot:
    message_ok = "ok"
    message_error = "error"

    def __init__(self):
        self.__left_led = Led(1)
        self.__right_led = Led(2)

        self.__left_wheel = Wheel(3)
        self.__right_wheel = Wheel(4)

        self.__distance_sensor = DistanceSensor(5)

        self.__parts = {self.__left_led, self.__right_led, self.__left_wheel, self.__right_wheel,
                        self.__distance_sensor}

        self.__connected_client = "no client connected yet :("

    def get_parts(self) -> str:

        parts_string = "{ parts: [\n"

        for part in self.__parts:
            parts_string += part.as_json()

        parts_string += "]}"

        return parts_string

    def do_with_part(self, id, type, action):
        id = int(id)
        for part in self.__parts:
            if part.get_type() == type and part.get_id() == id:
                action(part)
                return Robot.message_ok
        return Robot.message_error

    def led_on(self, led_id: int):
        return self.do_with_part(led_id, Part.type_led, lambda part: part.turn_on())

    def led_off(self, led_id):
        return self.do_with_part(led_id, Part.type_led, lambda part: part.turn_off())

    def wheel_forward(self, wheel_id):
        return self.do_with_part(wheel_id, Part.type_wheel, lambda part: part.drive_forward())

    def wheel_backward(self, wheel_id):
        return self.do_with_part(wheel_id, Part.type_wheel, lambda part: part.drive_backward())

    def wheel_stop(self, wheel_id):
        return self.do_with_part(wheel_id, Part.type_wheel, lambda part: part.stop())

    def wheel_fullstop(self):
        no_wheels_found = True
        for part in self.__parts:
            if part.get_type() == Part.type_wheel:
                part.stop()
                no_wheels_found = False

        if no_wheels_found:
            return Robot.message_error
        else:
            return Robot.message_ok

    def distance_sensor_read(self, sensor_id):
        sensor_id = int(sensor_id)
        for part in self.__parts:
            if part.get_type() == Part.type_distance_sensor and part.get_id() == sensor_id:
                return str(part.get_distance_in_cm())
        return Robot.message_error
