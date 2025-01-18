#include "arenas++.hpp"
#include <catch2/catch_all.hpp>

#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <iostream>

TEST_CASE("Prueba de creación y destrucción de una arena", "[Arena]") {
    size_t capacidad = 4096;
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

TEST_CASE("Prueba de la creación de nuevas páginas y transferencia de memoria", "[Arena]") {
    size_t capacidad = 128; // Capacidad pequeña para probar múltiples páginas
    Arena arena(capacidad);

    SECTION("Se debe crear una nueva página cuando no hay suficiente espacio") {
        // Asignamos más memoria de la que puede caber en la arena inicial
        void* memoria1 = arena.alocar(128, alignof(std::max_align_t));
        REQUIRE(memoria1 != nullptr);

        // Forzamos la creación de una nueva página al asignar más memoria
        void* memoria2 = arena.alocar(128, alignof(std::max_align_t));
        REQUIRE(memoria2 != nullptr);

        // Verificamos que se haya creado una nueva página
        REQUIRE(memoria1 != memoria2); // Las direcciones deben ser diferentes
    }

    SECTION("Las nuevas páginas deben manejarse correctamente sin perder datos previos") {
        void* memoria1 = arena.alocar(128, alignof(std::max_align_t));
        REQUIRE(memoria1 != nullptr);

        // Asignamos más memoria para forzar una nueva página
        void* memoria2 = arena.alocar(128, alignof(std::max_align_t));
        REQUIRE(memoria2 != nullptr);
  
        // Asignamos más memoria, pero una cantidad que no requiere 3ra pagina, reusa la segunda, que fue "limpiada".
        void* memoria3 = arena.alocar(128, alignof(std::max_align_t));
        REQUIRE(memoria3 != nullptr);

        // Ahora liberamos la memoria y verificamos si reutiliza correctamente
        arena.liberar();
        void* memoriaReusada1 = arena.alocar(128, alignof(std::max_align_t));
        void* memoriaReusada2 = arena.alocar(128, alignof(std::max_align_t));
        REQUIRE(memoriaReusada1 != nullptr);
        REQUIRE(memoriaReusada2 != nullptr);
        REQUIRE(memoriaReusada1 == memoria2); // Reutiliza la misma memoria
        REQUIRE(memoriaReusada2 == memoria3); // Reutiliza la misma memoria
    }
}

TEST_CASE("Prueba de fuga de memoria: Liberación de memoria después de nuevas páginas", "[Arena][Fugas]") {
    size_t capacidad = 128;
    Arena arena(capacidad);

    SECTION("No debe haber fuga de memoria después de crear nuevas páginas") {
        void* memoria1 = arena.alocar(128, alignof(std::max_align_t));
        void* memoria2 = arena.alocar(128, alignof(std::max_align_t));

        // Las direcciones deben ser diferentes, ya que una nueva página debe haberse creado
        REQUIRE(memoria1 != memoria2);

        // Ahora liberar la arena y verificar que no haya fuga
        arena.liberar();
        // Si llegamos aquí sin un error de memoria, no hay fuga de memoria
    }
}

TEST_CASE("Prueba de fuga de memoria: Destrucción de arena", "[Arena][Fugas]") {
    size_t capacidad = 1024;
    Arena arena(capacidad);

    SECTION("No debe haber fuga de memoria cuando la arena es destruida") {
        void* memoria1 = arena.alocar(128, alignof(std::max_align_t));
        void* memoria2 = arena.alocar(256, alignof(std::max_align_t));

        // Asignamos memoria, pero cuando la arena sea destruida, no debería haber fuga
        REQUIRE(memoria1 != nullptr);
        REQUIRE(memoria2 != nullptr);

        // La memoria debe ser liberada correctamente al destruir la arena
    }
}

TEST_CASE("Prueba de fuga de memoria: Múltiples liberaciones de arena", "[Arena][Fugas]") {
    size_t capacidad = 1024;
    Arena arena(capacidad);

    SECTION("No debe haber fuga de memoria después de múltiples liberaciones de la arena") {
        for (int i = 0; i < 100; ++i) {
            void* memoria = arena.alocar(128, alignof(std::max_align_t));
            REQUIRE(memoria != nullptr);
        }

        // Ahora liberar varias veces y verificar que no hay fuga
        arena.liberar();  // Primera liberación
        arena.liberar();  // Segunda liberación (sin que se pierda memoria)
    }
}

TEST_CASE("Prueba de fuga de memoria: Reasignación de memoria", "[Arena][Fugas]") {
    size_t capacidad = 128;
    Arena arena(capacidad);

    SECTION("No debe haber fuga de memoria cuando se reasigna una página") {
        void* memoria1 = arena.alocar(128, alignof(std::max_align_t));
        void* memoria2 = arena.alocar(256, alignof(std::max_align_t)); // Nueva página debe ser creada
        REQUIRE(memoria1 != memoria2);

        // Después de liberar, no debe haber fuga de memoria
        arena.liberar();

        void* nuevaMemoria = arena.alocar(128, alignof(std::max_align_t)); // Reasignación
        REQUIRE(nuevaMemoria != nullptr);
        REQUIRE(nuevaMemoria != memoria1); // Debe ser diferente de la anterior
    }
}

   class TipoConConstructorPorDefecto {
    public:
        int x;
        double y;
        TipoConConstructorPorDefecto() : x(0), y(0.0) {}
    };

    class TipoSinConstructorPorDefecto {
    public:
        int x;
        double y;
        TipoSinConstructorPorDefecto(int a, double b) : x(a), y(b) {}
    };

    inline bool operator==(const TipoConConstructorPorDefecto& a, const TipoSinConstructorPorDefecto& b) {
        return a.x == b.x && a.y == b.y;
    }

    inline bool operator!=(const TipoConConstructorPorDefecto& a, const TipoSinConstructorPorDefecto& b) {
        return !(a == b);
    }

    inline bool operator==(const TipoSinConstructorPorDefecto& a, const TipoConConstructorPorDefecto& b) {
        return a.x == b.x && a.y == b.y;
    }

    inline bool operator!=(const TipoSinConstructorPorDefecto& a, const TipoConConstructorPorDefecto& b) {
        return !(a == b);
    }

TEST_CASE("Prueba con tipos personalizados", "[Arena][TiposPersonalizados]") {
    size_t capacidad = 1024;
    Arena arena(capacidad);

    SECTION("Asignación de tipo con constructor por defecto") {
        auto* objeto = arena.alocar<TipoConConstructorPorDefecto>();
        REQUIRE(objeto != nullptr);
        REQUIRE(reinterpret_cast<uintptr_t>(objeto) % alignof(TipoConConstructorPorDefecto) == 0);

        // Validar valores iniciales
        REQUIRE(objeto->x == 0);
        REQUIRE(objeto->y == 0.0);
    }

    SECTION("Asignación de tipo sin constructor por defecto") {
        auto* objeto = arena.alocar<TipoSinConstructorPorDefecto>(42, 3.14);
        REQUIRE(objeto != nullptr);
        REQUIRE(reinterpret_cast<uintptr_t>(objeto) % alignof(TipoSinConstructorPorDefecto) == 0);

        // Validar valores inicializados
        REQUIRE(objeto->x == 42);
        REQUIRE(objeto->y == 3.14);
    }

    SECTION("Asignación múltiple de tipos personalizados") {
        auto* objeto1 = arena.alocar<TipoConConstructorPorDefecto>();
        auto* objeto2 = arena.alocar<TipoSinConstructorPorDefecto>(1, 2.71);
        REQUIRE(objeto1 != nullptr);
        REQUIRE(objeto2 != nullptr);
        REQUIRE(*objeto1 != *objeto2);

        // Validar valores
        REQUIRE(objeto1->x == 0);
        REQUIRE(objeto1->y == 0.0);
        REQUIRE(objeto2->x == 1);
        REQUIRE(objeto2->y == 2.71);
    }

    SECTION("Liberar memoria con tipos personalizados") {
        auto* objeto1 = arena.alocar<TipoConConstructorPorDefecto>();
        auto* objeto2 = arena.alocar<TipoSinConstructorPorDefecto>(99, 1.23);
        REQUIRE(objeto1 != nullptr);
        REQUIRE(objeto2 != nullptr);

        arena.liberar();

        auto* nuevoObjeto = arena.alocar<TipoConConstructorPorDefecto>();
        REQUIRE(nuevoObjeto != nullptr);
        REQUIRE(nuevoObjeto == objeto1); // Debe reutilizar la memoria
    }
}