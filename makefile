CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -pthread -Iinclude -MMD -MP
LDFLAGS = -pthread

FUENTES = src/main.cpp src/argumentos.cpp src/cfg.cpp src/cliente.cpp src/salaespera.cpp src/logs.cpp src/matchmaker.cpp src/mesa.cpp src/juez.cpp src/tablero.cpp src/fichas.cpp
OBJETOS = $(patsubst src/%.cpp,build/%.o,$(FUENTES))
DEPENDENCIAS = $(OBJETOS:.o=.d)

.PHONY: all compilar ejecutar clean

all: compilar

compilar: torneo

torneo: $(OBJETOS)
	$(CXX) $(OBJETOS) -o $@ $(LDFLAGS)

build/%.o: src/%.cpp | build
	$(CXX) $(CXXFLAGS) -c $< -o $@

build:
	mkdir -p build

ejecutar: compilar
	@if [ -z "$(N)" ] || [ -z "$(M)" ]; then \
		echo "Uso: make ejecutar N=10 M=2 [CONFIG=config.txt]"; \
		exit 1; \
	fi
	@if [ -n "$(CONFIG)" ]; then \
		./torneo "$(N)" "$(M)" "$(CONFIG)"; \
	else \
		./torneo "$(N)" "$(M)"; \
	fi

clean:
	rm -rf build torneo

-include $(DEPENDENCIAS)
