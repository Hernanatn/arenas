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
    
    Arena(Arena&&);
    Arena& operator=(Arena&&);

    Arena(size_t capacidad);
    ~Arena();
    
    /** 
     * @brief Asigna memoria para un objeto del tipo T y asegura el alineamiento.
     * @tparam T Tipo de objeto a asignar.
     * @return Puntero a la memoria asignada.
     */
    template<typename T> T* alocar();

    /** 
     * @brief Asigna memoria cruda del tamaño y alineamiento dados.
     * @param tamaño Tamaño de la memoria a asignar en bytes.
     * @param alineado Alineación de la memoria.
     * @return Puntero a la memoria asignada.
     */
    void* alocar(size_t tamaño, size_t alineado);
    
    /** 
     * @brief Resetea la arena, marca la memoria asignada como sobrescribible.
     */
    void liberar();

    private:
    /** 
     * @brief Crea una nueva arena (página), y transfiere el dominio de la memoria de la Arena actual. Resetea la arena actual.
     * @param capacidad Capacidad nueva de la  arena.
     */
    void nuevaPagina(size_t capacidad);

    /** 
     * @brief Alinea el tamaño al límite de alineamiento especificado.
     * @param n Tamaño a alinear.
     * @param alineado Límite de alineamiento.
     * @return Tamaño alineado.
     */
    static size_t alinearMax(size_t n, size_t alineado);

    /** 
     * @brief Comunica si la arena tiene espacio suficiente para alocar el tamaño solicitado.
     * @param tamaño Tamaño solicitado.
     */
    bool puedeAlocar(size_t tamaño) noexcept;
    
    /** 
     * capacidad - ocupado
     */
    size_t espacio() const noexcept;
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
Arena::Arena(Arena &&otro){
    this->data = otro.data;
    this->capacidad = otro.capacidad;
    this->ocupado = otro.ocupado;
    this->siguiente = otro.siguiente;
    otro.data = nullptr;
    otro.capacidad = 0;
    otro.ocupado = 0;
    otro.siguiente = nullptr;
}

Arena& Arena::operator=(Arena&& otro) {
    if (this != &otro) {
        this->~Arena(); 
        new (this) Arena(std::move(otro));
    }
    return *this;
}

Arena::~Arena(){
    if (siguiente != nullptr){
        delete siguiente;
    }
    if (data != nullptr){
        free(data);
    }
    data = nullptr;
    capacidad = 0;
    ocupado = 0;
};


void * Arena::alocar(size_t tamaño, size_t alineado = alignof(std::max_align_t)){
    size_t espacio = this->espacio();
    void *p = &this->data[this->ocupado];
    void *pa = std::align(alineado, tamaño, p,espacio);

    if (!pa || !puedeAlocar(tamaño)){
        if (siguiente != nullptr && siguiente->puedeAlocar(tamaño)){
            return siguiente->alocar(tamaño, alineado);
        } 
        nuevaPagina(std::max(capacidad*2,alinearMax(tamaño + alineado, alineado)));
        return alocar(tamaño, alineado); 
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

size_t Arena::espacio() const noexcept{
    return capacidad - ocupado;    
}

bool Arena::puedeAlocar(size_t tamaño) noexcept {
    return espacio() >= tamaño || (siguiente && siguiente->puedeAlocar(tamaño));
}

void Arena::nuevaPagina(size_t capacidad){
    Arena *vieja = new Arena(std::move(*this));

    this->data = static_cast<char*>(malloc(capacidad));
    if (!this->data) {
        delete vieja;
        throw std::bad_alloc();
    }
    this->ocupado = 0;
    this->capacidad = capacidad;

    this->siguiente = vieja;
}

size_t Arena::alinearMax(size_t n, size_t alineado){
    return  (n + alineado -1) & ~(alineado - 1);
}
#endif