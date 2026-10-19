// Una segunda unidad de traducción para las pruebas: incluye arenas++.hpp otra vez. Si una definición del encabezado
// no fuera `inline`, el enlace fallaría por definición múltiple.

#include "arenas++.hpp"

using namespace memoria;

int asignarEnOtraUnidad(Arena &arena, int valor) {
    int *p = arena.alocar<int>(valor);
    return *p;
}
