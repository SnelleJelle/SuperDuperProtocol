from random import randint

from ServerMock.Robot.Part import Part


class DistanceSensor(Part):
    def __init__(self, part_id=-1, part_type=Part.type_distance_sensor):
        super().__init__(part_id, part_type)

    def get_distance_in_cm(self):
        return randint(0, 100)
