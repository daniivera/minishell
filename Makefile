# Compilador y banderas requeridas por la guía
CC = gcc
CFLAGS = -Wall -Wextra
TARGET = minishell
SRC = minishell.c

# Regla por defecto: compila el ejecutable
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

# Regla para limpiar el ejecutable
clean:
	rm -f $(TARGET)

.PHONY: all clean