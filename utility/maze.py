import sys
import random

class Maze:

    def __init__(self, width, height):
        #north, south, west, east = 0b1111 if it's like this it's a new room not processed
        self.width = width
        self.height = height
        self.rooms = [[0x0F] * self.width for _ in range(self.height)] #bidimensional list

        self.north = (0 , -1, 0b0111, 0b1011)  #move x, move y, what to turn off in my room, what to turn off in the next room
        self.south = (0 , 1, 0b1011, 0b0111)
        self.west = (-1 , 0, 0b1101, 0b1110)
        self.east = (1 , 0, 0b1110, 0b1101)

        self.directions = [self.north, self.south, self.west, self.east]

    def is_valid_room(self, x, y):
        if x < 0 or x >= self.width:
            return False
        if y < 0 or y >= self.height:
            return False
        return True

    def has_been_visited(self, x, y):
        return self.rooms[y][x] & 0xF != 0xF #aready visited

    def knock_down_walls(self, x = None, y = None):
        if x is None:
            x = random.randint(0, self.width - 1)
        if y is None:
            y = random.randint(0, self.height - 1)

        random.shuffle(self.directions)

        for delta_x, delta_y, current_bitmask, next_bitmask in self.directions:
            next_x, next_y = x + delta_x, y + delta_y
            if self.is_valid_room(next_x, next_y) and not self.has_been_visited(next_x, next_y):
                self.rooms[y][x] &= current_bitmask
                self.rooms[next_y][next_x] &= next_bitmask
                self.knock_down_walls(next_x, next_y)

maze = Maze(int(sys.argv[1]), int(sys.argv[2]))
maze.knock_down_walls()
for row in maze.rooms:
    print("{",",".join(map(str,row)),"}")