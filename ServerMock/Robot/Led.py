from ServerMock.Robot.Part import Part


class Led(Part):
    def __init__(self, part_id=-1, part_type=Part.type_led):
        super().__init__(part_id, part_type)

        self.__on = False

    def turn_on(self):
        self.__on = True

    def turn_off(self):
        self.__on = False

    def is_on(self):
        return self.__on == True

    def as_json(self):
        base = super().as_json()
        base["state"] = int(self.__on)
        return base
