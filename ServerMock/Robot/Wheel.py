from ServerMock.Robot.Part import Part


class Wheel(Part):

    direction_forward = "forward"
    direction_backward = "backward"
    direction_stop = "stop"

    def __init__(self, part_id=-1, part_type=Part.type_wheel):
        super().__init__(part_id, part_type)

        self.__direction = Wheel.direction_stop

    def drive_forward(self):
        self.__direction = Wheel.direction_forward

    def drive_backward(self):
        self.__direction = Wheel.direction_backward

    def stop(self):
        self.__direction = Wheel.direction_stop

    def get_direction(self):
        return self.__direction

    def as_json(self):
        base = super().as_json(end=",\n\t")
        return base + "{0}: {1} }}\n\n".format("direction", self.__direction)
