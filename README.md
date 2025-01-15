# Arenas++

`Arenas++` es una librería ligera en C++ diseñada para la asignación eficiente de memoria paginada en contextos de uso intensivo, donde los patrones de asignación y liberación son predecibles. Utiliza el modelo de **"Asignar una vez, liberar una vez"**, simplificando el manejo de memoria y reduciendo la complejidad asociada con múltiples llamadas a `new` y `delete` / `malloc` y `free`.


<img src="https://img.shields.io/badge/hecho_por-Ch'aska-253545?style=for-the-badge" alt="hecho_por_Chaska" height="25px"/> <img src="https://img.shields.io/badge/C%2B%2B_17-4549BF?style=for-the-badge&logo=c%2B%2B&logoColor=white" alt="C++" height="25px"/> <a href=https://www.raylib.com>

<img src="https://img.shields.io/badge/pruebas-pasando-darkgreen?style=for-the-badge" alt="pruebas" height="20px"/> <a href=https://github.com/hernanatn/github.com/hernanatn/futbol_fantasia/releases/latest><img src="https://img.shields.io/badge/Versión-0.0.1--alpha-orange?style=for-the-badge" alt="version" height="20px"/></a> <img src="https://img.shields.io/badge/Licencia-CC_BY--NC--ND_4.0-lightgrey?style=for-the-badge" alt="licencia" height="20px"/>

## Índice
1. [Introducción](#introducción)
2. [Referencia de la API](#referencia-de-la-api)
   - [Constructores](#constructores)
   - [Métodos](#métodos)
3. [Ejemplo de Uso Idiomático](#ejemplo-de-uso-idiomático)
4. [Detalles Técnicos](#detalles-técnicos)
5. [Estado del Proyecto](#estado-del-proyecto)
6. [Licencia](#licencia)


## Introducción

`Arena` provee un modelo de asignación de memoria paginada, eficiente para situaciones en las que se realizan múltiples asignaciones dentro de un mismo contexto (por ejemplo, durante el procesamiento de un lote de datos). Todas las páginas de memoria se liberan de una vez al finalizar el contexto, evitando el costo de liberar cada bloque de forma individual.

---

## Estado del Proyecto

Esta es la versión Alpha v0.0.1. La funcionalidad principal está implementada y probada con casos básicos y avanzados, pero la librería puede presentar errores y la API puede sufrir cambios sustanciales en el futuro.


## Referencia de la API

#### `Arena(size_t capacidad)`
Crea una nueva arena con una capacidad inicial especificada.

- **Parámetros:**
  - `capacidad`: Tamaño inicial de la arena en bytes.
- **Excepciones:**
  - Lanza `std::bad_alloc` si no se puede asignar memoria.

### Métodos

#### `template <typename T> T* alocar()`
Asigna memoria suficiente para un objeto de tipo `T`, respetando los requisitos de alineación de `T`.

- **Retorno:** Un puntero a memoria alineada para el tipo `T`.

#### `void* alocar(size_t tamaño, size_t alineado)`
Asigna un bloque de memoria de tamaño especificado con alineación personalizada.

- **Parámetros:**
  - `tamaño`: Cantidad de bytes a asignar.
  - `alineado`: Alineación requerida (por defecto, `alignof(std::max_align_t)`).
- **Retorno:** Un puntero a la memoria asignada.
- **Excepciones:** Lanza `std::bad_alloc` si no hay memoria suficiente.

#### `void liberar()`
Reinicia el marcador de uso de la arena, haciendo que toda la memoria previamente asignada sea reutilizable. Las páginas adicionales creadas durante el ciclo de vida también se reinician.

## Pruebas
La librería incluye una suite (aún en progreso) de pruebas implementada con Catch2, que verifica el comportamiento correcto.

Se puede correr el archivo provisto ( [/pruebas/correr_pruebas.exe](/pruebas/pruebas.exe) ) o [compilar las pruebas desde cero](/documentación/Pruebas.md)

---

## Ejemplo de Uso Idiomático

```cpp
#include <iostream>
#include "arenas.hpp"

struct Vector3 {
    float x, y, z;
};

int main() {
    // Crear una arena con capacidad inicial de 1024 bytes
    Arena arena(1024);

    // Asignar un entero
    int* entero = arena.alocar<int>();
    *entero = 42;
    std::cout << "Entero asignado: " << *entero << std::endl;

    // Asignar un objeto de tipo personalizado
    Vector3* vector = arena.alocar<Vector3>();
    vector->x = 1.0f;
    vector->y = 2.0f;
    vector->z = 3.0f;
    std::cout << "Vector asignado: (" << vector->x << ", " << vector->y << ", " << vector->z << ")\n";

    // Asignar un bloque de memoria alineado
    void* bloque = arena.alocar(256, 64);
    std::cout << "Bloque alineado asignado en: " << bloque << std::endl;

    // Reutilizar memoria liberando la arena
    arena.liberar();

    return 0;
} 
```


## Detalles Técnicos

1.  **Estrategia de Paginación:**
    - Si no hay suficiente espacio en la arena actual, se crea automáticamente una nueva página con el tamaño necesario.
    - Las páginas adicionales se gestionan mediante una lista enlazada privada, asegurando un modelo de propiedad claro y evitando fugas de memoria.
2. Eficiencia:
    - Optimizada para contextos donde el patrón de uso es asignar múltiples bloques y liberarlos todos al final.

3. Compatibilidad:
Requiere un compilador compatible con C++17 o superior.

## Licencia
Este proyecto está licenciado bajo CC BY-SA 4.0. Ver el archivo LICENSE para más detalles.
