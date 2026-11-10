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


class Medidor {
private:
    float kWh;
    float costo;


public:
    Medidor() : kWh(0.0f), costo(0.0f) {}
    void reiniciar() { kWh = 0.0f; costo = 0.0f; }
    void registrar(float potencia, float precio) {
        kWh += potencia;
        costo += potencia * precio;
    }
    float getKWh() const { return kWh; }
    float getCosto() const { return costo; }
};


class Mercado {
public:
    float precioHora(int hora) const {
        int minimo = 0;
        int maximo = 0;
        if (hora <= 6) {
            minimo = 398;
            maximo = 640;
        } else if (hora <= 14) {
            minimo = 456;
            maximo = 970;
        } else if (hora <= 17) {
            minimo = 596;
            maximo = 1040;
        } else if (hora <= 20) {
            minimo = 705;
            maximo = 1612;
        } else {
            minimo = 565;
            maximo = 970;
        }
        return static_cast<float>(minimo + rand() % (maximo - minimo + 1));
    }
};


class Negocio {
private:
    string nombre;
    Medidor medidor;                 // Composicion: nace y muere con el negocio.
    Carga* cargas[MAX_CARGAS];       // Agregacion: las cargas se crean afuera.
    int cantidadCargas;
    Mercado* mercado;                // Asociacion: el negocio conoce al mercado.
    float costoSinGestor;
    float costoConGestor;
    int horasSimuladas;
    int fallas;
    

public:
    Negocio(string nombreNegocio, Mercado* mercadoNegocio)
        : nombre(nombreNegocio), cantidadCargas(0), mercado(mercadoNegocio),
          costoSinGestor(0.0f), costoConGestor(0.0f), horasSimuladas(0), fallas(0) {}


    void agregarCarga(Carga* carga) {
        if (cantidadCargas < MAX_CARGAS) {
            cargas[cantidadCargas++] = carga;
        }
    }


    void mostrar() const {
        cout << "\nNegocio: " << nombre << "\nCargas instaladas:\n";
        for (int i = 0; i < cantidadCargas; ++i) {
            cout << "  - " << cargas[i]->getNombre() << " ("
                 << cargas[i]->getPotencia() << " kW)\n";
        }
    }


    void evaluar(int hora, float precio, float nivel) const {
        cout << "\nLectura: hora " << hora << ", precio " << precio
             << " COP/kWh, embalse " << nivel << " %\n";
        for (int i = 0; i < cantidadCargas; ++i) {
            bool encender = cargas[i]->debeEncender(hora, precio, nivel);
            cout << "  " << setw(24) << left << cargas[i]->getNombre() << right
                 << (encender ? "ENCENDER" : "APLAZAR/APAGAR") << "\n";
        }
    }