class Part:
    type_led = "led"
    type_wheel = "wheel"
    type_distance_sensor = "distance_sensor"
    type_unset = "ERROR: unset"

    def __init__(self, part_id=-1, part_type=type_unset):
        self.__part_id = part_id
        self.__part_type = part_type

    def get_type(self):
        return self.__part_type

    def get_id(self) -> int:
        return self.__part_id

    def as_json(self):
        return {self.__part_id: self.__part_type}

    def __str__(self):
        return self.as_json()
