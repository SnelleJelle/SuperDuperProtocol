from ServerMock.Robot.Part import Part


class Wheel(Part):

    direction_forward = "forward"
    direction_backward = "backward"
    direction_stop = "stop"

    position_left = "left"
    position_right = "right"

    def __init__(self, part_id=-1, part_type=Part.type_wheel, position=None):
        super().__init__(part_id, part_type)

        self.__direction = Wheel.direction_stop
        self.__position = position;

    def drive_forward(self):
        self.__direction = Wheel.direction_forward

    def drive_backward(self):
        self.__direction = Wheel.direction_backward

    def stop(self):
        self.__direction = Wheel.direction_stop

    def get_direction(self):
        return self.__direction

    def as_json(self):
        base = super().as_json()
        base["extras"] = [
            {"direction":  self.__direction},
            {"position": self.__position}
        ]
        return base
