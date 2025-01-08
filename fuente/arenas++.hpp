#ifndef ARENAS_H
#define ARENAS_H
#include <cassert>
#include <cstddef>
#include <cstdlib>

struct Arena{
    private:
    size_t capacidad;
    size_t ocupado;
    char *data;
    Arena *siguiente;

    public:
    Arena(const Arena&) = delete;
    Arena& operator=(const Arena&) = delete;


    Arena(size_t capacidad);
    ~Arena();

    template<typename T> T* alocar();
    void* alocar(size_t tamaño, size_t alineado);
    void liberar();

    private:
    void nuevaPagina(size_t capacidad);
    static size_t alinearMax(size_t n, size_t alineado);
};

template<typename T> T* Arena::alocar(){
    size_t tamaño = sizeof(T);
    size_t alineado = alignof(T);

    void * pa = alocar(tamaño,alineado);
    return static_cast<T*>(pa);
};
#endif

#ifndef ARENAS_IMPL
#define ARENAS_IMPL
#include <memory>
#include <new>

Arena::Arena(size_t capacidad) : capacidad(capacidad), ocupado(0), siguiente(nullptr) {
    data = static_cast<char*>(malloc(this->capacidad));
    if (!data) {
        throw std::bad_alloc();
    }
}

Arena::~Arena(){
    if (siguiente != nullptr) {
        delete siguiente;
        siguiente = nullptr;
    }
    free(data);
    data = nullptr;
    capacidad = 0;
    ocupado = 0;
};


void * Arena::alocar(size_t tamaño, size_t alineado = alignof(std::max_align_t)){
    
    size_t espacio = capacidad - ocupado;
    void *p = &this->data[this->ocupado];
    void *pa = std::align(alineado, tamaño, p,espacio);

    if (!pa || espacio < tamaño){
        if (siguiente == nullptr){
            nuevaPagina(std::max(capacidad,alinearMax(tamaño + alineado, alineado)));
        }
        return siguiente->alocar(tamaño, alineado);
    }

    this->ocupado += (static_cast<char*>(pa) - (this->data + this->ocupado)) + tamaño;
    return pa;

}



void Arena::liberar(){
    if (siguiente != nullptr){
        siguiente->liberar();
    }
    this->ocupado = 0;
};


void Arena::nuevaPagina(size_t capacidad){
    if (siguiente == nullptr){
        siguiente = new Arena(capacidad);
    } 
}

size_t Arena::alinearMax(size_t n, size_t alineado){

    return  (n + alineado -1) & ~(alineado - 1);
}
#endif