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
    

    void simularDia() {
        medidor.reiniciar();
        costoSinGestor = 0.0f;
        costoConGestor = 0.0f;
        horasSimuladas = 0;
        fallas = 0;
        cout << "\n--- Simulacion de 24 horas ---\n";
        cout << setw(5) << "Hora" << setw(9) << "Precio" << setw(10)
             << "Gestor" << setw(14) << "Costo hora" << "\n";


        for (int hora = 0; hora < HORAS_DIA; ++hora) {
            float precio = mercado->precioHora(hora);
            float potenciaSinGestor = POTENCIA_ORDENADORA + POTENCIA_REFRIGERADOR +
                                      POTENCIA_BOMBA_RIEGO;
            costoSinGestor += potenciaSinGestor * precio;


            if (precio > PRECIO_ESCASEZ) {
                cout << setw(5) << hora << setw(9) << precio
                     << " PRECIO DE ESCASEZ: break\n";
                break;
            }


            ++horasSimuladas;
            if (rand() % FALLA_CADA == 0) {
                for (int i = 0; i < cantidadCargas; ++i) {
                    cargas[i]->apagar();
                }
                ++fallas;
                cout << setw(5) << hora << setw(9) << "--"
                     << " falla de lectura: continue\n";
                continue;
            }


            float costoHora = 0.0f;
            for (int i = 0; i < cantidadCargas; ++i) {
                if (cargas[i]->debeEncender(hora, precio, NIVEL_OBJETIVO)) {
                    cargas[i]->encender();
                    costoHora += cargas[i]->costoHora(precio);
                    medidor.registrar(cargas[i]->getPotencia(), precio);
                } else {
                    cargas[i]->apagar();
                }
            }
            costoConGestor += costoHora;
            cout << setw(5) << hora << setw(9) << precio << setw(10) << "activo"
                 << setw(14) << fixed << setprecision(0) << costoHora << "\n";
        }
    }


    void resumen() const {
        cout << "\n--- Resumen de " << nombre << " ---\n";
        cout << "Horas simuladas: " << horasSimuladas << " de " << HORAS_DIA
             << " | fallas: " << fallas << "\n";
        for (int i = 0; i < cantidadCargas; ++i) {
            cout << "  " << setw(24) << left << cargas[i]->getNombre() << right
                 << cargas[i]->getHorasUso() << " h, "
                 << cargas[i]->getHorasUso() * cargas[i]->getPotencia() << " kWh\n";
        }
        cout << fixed << setprecision(0)
             << "Consumo con gestor: $" << costoConGestor << "\n"
             << "Consumo sin gestor: $" << costoSinGestor << "\n";
        if (costoSinGestor > 0.0f) {
            float ahorro = costoSinGestor - costoConGestor;
            cout << "Ahorro: $" << ahorro << " ("
                 << setprecision(1) << ahorro / costoSinGestor * 100.0f << " %)\n";
        }
        cout << setprecision(2) << "Medidor: " << medidor.getKWh()
             << " kWh, $" << medidor.getCosto() << "\n";
    }
};


int leerEntero(const string& mensaje, int minimo, int maximo) {
    int valor;
    do {
        cout << mensaje;
        cin >> valor;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(10000, '\n');
            valor = minimo - 1;
        }
        if (valor < minimo || valor > maximo) {
            cout << "Valor invalido. Rango permitido: " << minimo << " a " << maximo << ".\n";
        }
    } while (valor < minimo || valor > maximo);
    return valor;
}


float leerPrecio() {
    float precio;
    do {
        cout << "Precio de energia (COP/kWh, mayor que 0): ";
        cin >> precio;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(10000, '\n');
            precio = 0.0f;
        }
        if (precio <= 0.0f) cout << "El precio debe ser mayor que 0.\n";
    } while (precio <= 0.0f);
    return precio;
}


int main() {
    srand(SEMILLA);
    Mercado mercado;
    CargaCritica ordenadora("Ordenadora electrica", POTENCIA_ORDENADORA);
    CargaCritica refrigerador("Refrigerador de leche", POTENCIA_REFRIGERADOR);
    CargaAplazable riego("Bomba de riego", POTENCIA_BOMBA_RIEGO);
    Negocio finca("Finca lechera El Buen Pasto", &mercado);
    finca.agregarCarga(&ordenadora);
    finca.agregarCarga(&refrigerador);
    finca.agregarCarga(&riego);
    finca.mostrar();

