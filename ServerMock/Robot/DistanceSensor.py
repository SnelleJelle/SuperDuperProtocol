from random import randint

from ServerMock.Robot.Part import Part


class DistanceSensor(Part):
    def __init__(self, part_id=-1, part_type=Part.type_distance_sensor):
        super().__init__(part_id, part_type)

    def get_distance_in_cm(self) -> int:
        return randint(0, 100)

    def as_json(self):
        base = super().as_json()
        base["extras"] = [{"distance": self.get_distance_in_cm()}]
        return base
