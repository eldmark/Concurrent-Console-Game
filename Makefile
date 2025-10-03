# Makefile para Galaga con sistema de Jefe

CXX = g++
CXXFLAGS = -std=c++11 -Wall -pthread
LIBS = -lncurses -lpthread

# Archivos objeto
OBJS = Galaga.o Nave.o Enemigo.o Pantalla.o Boss.o

# Ejecutable
TARGET = galaga

# Regla principal
all: $(TARGET)

# Crear ejecutable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

# Compilar archivos individuales
Galaga.o: Galaga.cpp Nave.h Enemigo.h Pantalla.h Boss.h
	$(CXX) $(CXXFLAGS) -c Galaga.cpp

Nave.o: Nave.cpp Nave.h Pantalla.h
	$(CXX) $(CXXFLAGS) -c Nave.cpp

Enemigo.o: Enemigo.cpp Enemigo.h Pantalla.h
	$(CXX) $(CXXFLAGS) -c Enemigo.cpp

Pantalla.o: Pantalla.cpp Pantalla.h
	$(CXX) $(CXXFLAGS) -c Pantalla.cpp

Boss.o: Boss.cpp Boss.h Enemigo.h Pantalla.h
	$(CXX) $(CXXFLAGS) -c Boss.cpp

# Limpiar archivos compilados
clean:
	rm -f $(OBJS) $(TARGET)

# Recompilar todo
rebuild: clean all

# Ejecutar el juego
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean rebuild run