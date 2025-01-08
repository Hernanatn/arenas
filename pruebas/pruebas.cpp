#include "arenas++.hpp"
#include <catch2/catch_all.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Prueba de creación y destrucción de una arena", "[Arena]") {
    size_t capacidad = 1024;
    Arena arena(capacidad);

    SECTION("La arena debe inicializarse correctamente") {
        REQUIRE_NOTHROW(Arena(capacidad));
    }
}

TEST_CASE("Prueba de asignación básica en la arena", "[Arena]") {
    size_t capacidad = 1024;
    Arena arena(capacidad);

    SECTION("Asignación de enteros debe respetar alineación") {
        int* entero = arena.alocar<int>();
        REQUIRE(entero != nullptr);
        REQUIRE(reinterpret_cast<uintptr_t>(entero) % alignof(int) == 0);
    }

    SECTION("Asignación de estructuras debe respetar tamaño y alineación") {
        struct Estructura {
            double x;
            int y;
        };
        Estructura* estructura = arena.alocar<Estructura>();
        REQUIRE(estructura != nullptr);
        REQUIRE(reinterpret_cast<uintptr_t>(estructura) % alignof(Estructura) == 0);
    }
}

TEST_CASE("Prueba de múltiples asignaciones", "[Arena]") {
    size_t capacidad = 128; // Capacidad pequeña para forzar múltiples páginas
    Arena arena(capacidad);

    SECTION("La arena debe manejar múltiples asignaciones dentro de una sola página") {
        int* entero1 = arena.alocar<int>();
        int* entero2 = arena.alocar<int>();
        REQUIRE(entero1 != nullptr);
        REQUIRE(entero2 != nullptr);
        REQUIRE(entero1 != entero2);
    }

    SECTION("Asignaciones grandes deben crear nuevas páginas") {
        void* memoriaGrande = arena.alocar(256, alignof(std::max_align_t));
        REQUIRE(memoriaGrande != nullptr);
    }
}

TEST_CASE("Prueba de liberar memoria en la arena", "[Arena]") {
    size_t capacidad = 1024;
    Arena arena(capacidad);

    SECTION("Liberar debe reiniciar la arena a estado inicial") {
        void* memoria1 = arena.alocar(128, alignof(std::max_align_t));
        REQUIRE(memoria1 != nullptr);

        arena.liberar();

        void* memoria2 = arena.alocar(128, alignof(std::max_align_t));
        REQUIRE(memoria2 != nullptr);
        REQUIRE(memoria2 == memoria1); // Debe reutilizar la misma memoria
    }

    SECTION("Liberar debe reiniciar todas las páginas") {
        void* memoria1 = arena.alocar(512, alignof(std::max_align_t));
        void* memoria2 = arena.alocar(512, alignof(std::max_align_t)); // Nueva página
        REQUIRE(memoria1 != nullptr);
        REQUIRE(memoria2 != nullptr);

        arena.liberar();

        void* nuevaMemoria = arena.alocar(1024, alignof(std::max_align_t));
        REQUIRE(nuevaMemoria != nullptr);
        REQUIRE(nuevaMemoria == memoria1); // Debe reutilizar desde la primera página
    }
}

