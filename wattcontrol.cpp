// WattControl - Gestor de energia para una finca lechera
// Evidencia de aprendizaje 2: herencia, polimorfismo y relaciones entre clases.
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>

using namespace std;

#define HORAS_DIA 24
#define MAX_CARGAS 5
#define SEMILLA 2026

const float POTENCIA_ORDENADORA = 2.2f;
const float POTENCIA_REFRIGERADOR = 3.0f;
const float POTENCIA_BOMBA_RIEGO = 4.5f;
const float PRECIO_BARATO = 600.0f;
const float PRECIO_ESCASEZ = 1500.0f;
const float NIVEL_CRITICO = 20.0f;
const float NIVEL_OBJETIVO = 80.0f;
const int PICO_INICIO = 18;
const int PICO_FIN = 20;
const int FALLA_CADA = 20;


class Carga {
protected:
    string nombre;
    float potenciaKW;
    bool encendida;
    int horasUso;


public:
    Carga(string nombreCarga, float potencia)
        : nombre(nombreCarga), potenciaKW(potencia), encendida(false), horasUso(0) {}
    virtual ~Carga() = default;
    virtual bool debeEncender(int hora, float precio, float nivel) = 0;


    void encender() {
        encendida = true;
        ++horasUso;
    }


    void apagar() { encendida = false; }
    float costoHora(float precio) const { return encendida ? potenciaKW * precio : 0.0f; }
    string getNombre() const { return nombre; }
    float getPotencia() const { return potenciaKW; }
    int getHorasUso() const { return horasUso; }
};


class CargaCritica : public Carga {
public:
    CargaCritica(string nombreCarga, float potencia) : Carga(nombreCarga, potencia) {}


    bool debeEncender(int hora, float precio, float nivel) override {
        (void)hora;
        (void)precio;
        (void)nivel;
        return true;
    }
};


class CargaAplazable : public Carga {
public:
    CargaAplazable(string nombreCarga, float potencia) : Carga(nombreCarga, potencia) {}


    bool debeEncender(int hora, float precio, float nivel) override {
        bool horaPico = hora >= PICO_INICIO && hora <= PICO_FIN;
        return nivel < NIVEL_CRITICO ||
               (!horaPico && precio < PRECIO_BARATO && nivel < NIVEL_OBJETIVO) ||
               (precio >= PRECIO_BARATO && precio < PRECIO_ESCASEZ && nivel < NIVEL_CRITICO);
    }
};