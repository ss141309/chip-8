# This file is part of Chip-8 Emulator.

# Chip-8 Emulator is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or (at your option) any later
# version.

# Chip-8 Emulator is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

# You should have received a copy of the GNU General Public License along with
# Chip-8. If not, see <https://www.gnu.org/licenses/>.

CC = gcc
CFLAGS = -g3 -Wall -Wextra -Wpedantic -Wdouble-promotion -Wno-unused-parameter -Wno-unused-function -Wno-sign-conversion -fsanitize-trap -fsanitize=undefined
SRC = chip-8.c
DEPS = chip-8.h
TARGET = chip-8
LIBS = -lSDL2

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)
