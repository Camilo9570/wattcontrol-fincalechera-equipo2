/*
 * WattControl - Gestor de energia para Finca Lechera ("El Buen Pasto")
 * Evidencia de Aprendizaje 2 (EA2 / S35)
 * Asignatura: Programacion Orientada a Objetos - IU Digital

  * - Herencia, Polimorfismo, Composicion, Agregacion y Asociacion.
 * - Regla 6: Control de Potencia Contratada (8.0 kW).
 * - CargaAplazable con horas minimas (5 h) y alerta "produccion incompleta".
 * - Falla de lectura (continue): opera unicamente CargaCritica.
 * - Emergencia por racionamiento (break): precio > 1500 COP && embalse < 20%.
 * - Calculo dinamico desacoplado del costo sin gestor.
 */


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
const float POTENCIA_CONTRATADA = 8.0f;
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
    virtual bool esCritica() const = 0;
    virtual int getHorasMinimas() const { return 0; }

    void encender() {
        encendida = true;
        ++horasUso;
    }


    void apagar() { encendida = false; }
    void reiniciarUso() { horasUso = 0; encendida = false; }

    bool isEncendida() const { return encendida; }

    float costoHora(float precio) const { return encendida ? potenciaKW * precio : 0.0f; }
    string getNombre() const { return nombre; }
    float getPotencia() const { return potenciaKW; }
    int getHorasUso() const { return horasUso; }
};


class CargaCritica : public Carga {
public:

    CargaCritica(string nombreCarga, float potencia) : Carga(nombreCarga, potencia) {}

    bool esCritica() const override { return true; }

    bool debeEncender(int hora, float precio, float nivel) override {
        (void)hora;
        (void)precio;
        (void)nivel;
        return true;
    }
};


class CargaAplazable : public Carga {
private:
    int horasMinimas;


public:
    CargaAplazable(string nombreCarga, float potencia, int hMin = 5) 
        : Carga(nombreCarga, potencia), horasMinimas(hMin) {}

    bool esCritica() const override { return false; }
    int getHorasMinimas() const override { return horasMinimas; }


    bool debeEncender(int hora, float precio, float nivel) override {
        bool horaPico = (hora >= PICO_INICIO && hora <= PICO_FIN);
        bool embalseBajo = (nivel < NIVEL_CRITICO);
        bool precioAlto = (precio >= PRECIO_BARATO);

        if (embalseBajo || horaPico || precioAlto) {
            return false; // Aplazar / Apagar
        }
        return true; // Encender
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
    Medidor medidor;
    Carga* cargas[MAX_CARGAS];
    int cantidadCargas;
    Mercado* mercado;
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

        float potenciaPretendida = 0.0f;
        for (int i = 0; i < cantidadCargas; ++i) {
            const bool encender = cargas[i]->debeEncender(hora, precio, nivel);
            if (encender) {
                potenciaPretendida += cargas[i]->getPotencia();
            }
            cout << "  " << setw(24) << left << cargas[i]->getNombre() << right
                 << (encender ? "ENCENDER" : "APLAZAR/APAGAR") << "\n";
        }

        if (potenciaPretendida > POTENCIA_CONTRATADA) {
            cout << "  [ALERTA POTENCIA] Pretendida (" << potenciaPretendida
                 << " kW) supera Contratada (" << POTENCIA_CONTRATADA << " kW).\n";
        }
    }

    void simularDia() {
        medidor.reiniciar();
        costoSinGestor = 0.0f;
        costoConGestor = 0.0f;
        horasSimuladas = 0;
        fallas = 0;
        for (int i = 0; i < cantidadCargas; ++i) {
            cargas[i]->reiniciarUso();
        }

        cout << "\n--- Simulacion de 24 horas ---\n";
        cout << setw(5) << "Hora" << setw(9) << "Precio" << setw(10)
             << "Gestor" << setw(14) << "Costo hora" << "\n";

        for (int hora = 0; hora < HORAS_DIA; ++hora) {
            const float precio = mercado->precioHora(hora);
            const float nivelSimulado = 15.0f + static_cast<float>(rand() % 70);
            float potenciaSinGestor = 0.0f;

            for (int i = 0; i < cantidadCargas; ++i) {
                potenciaSinGestor += cargas[i]->getPotencia();
            }
            costoSinGestor += potenciaSinGestor * precio;

            if (precio > PRECIO_ESCASEZ && nivelSimulado < NIVEL_CRITICO) {
                cout << setw(5) << hora << setw(9) << fixed << setprecision(0) << precio
                     << " !!! [BREAK - RACIONAMIENTO EXTREMO] !!!\n";
                break;
            }

            ++horasSimuladas;


            float potenciaPretendida = 0.0f;
            for (int i = 0; i < cantidadCargas; ++i) {
                if (cargas[i]->isEncendida()) {
                    potenciaPretendida += cargas[i]->getPotencia();
                }
            }

            if (rand() % FALLA_CADA == 0) {
                ++fallas;
                cout << setw(5) << hora << setw(9) << "--"
                     << "  [FALLA LECTURA - CONTINUE] Solo opera Carga Critica\n";
                for (int i = 0; i < cantidadCargas; ++i) {
                    if (cargas[i]->esCritica()) {
                        cargas[i]->encender();
                        medidor.registrar(cargas[i]->getPotencia(), precio);
                        costoConGestor += cargas[i]->costoHora(precio);
                    } else {
                        cargas[i]->apagar();
                    }
                }
                continue;
            }

            if (potenciaPretendida > POTENCIA_CONTRATADA) {
                int idxMayor = -1;
                float mayorPot = -1.0f;
                for (int i = 0; i < cantidadCargas; ++i) {
                    if (!cargas[i]->esCritica() && cargas[i]->isEncendida() && cargas[i]->getPotencia() > mayorPot) {
                        mayorPot = cargas[i]->getPotencia();
                        idxMayor = i;
                    }
                }
                if (idxMayor != -1) {
                    cargas[idxMayor]->apagar();
                }
            }

            float costoHora = 0.0f;
            for (int i = 0; i < cantidadCargas; ++i) {
                if (cargas[i]->isEncendida()) {
                    costoHora += cargas[i]->costoHora(precio);
                    medidor.registrar(cargas[i]->getPotencia(), precio);
                }
            }

            costoConGestor += costoHora;
            cout << setw(5) << hora << setw(9) << fixed << setprecision(0) << precio << setw(12) << "activo"
                 << setw(14) << fixed << setprecision(0) << costoHora << "\n";
        }
    }

    void resumen() const {
        cout << "\n--- Resumen de " << nombre << " ---\n";
        cout << "Horas simuladas: " << horasSimuladas << " de " << HORAS_DIA
             << " | fallas: " << fallas << "\n";

        bool produccionIncompleta = false;
        for (int i = 0; i < cantidadCargas; ++i) {
            cout << "  " << setw(24) << left << cargas[i]->getNombre() << right
                 << cargas[i]->getHorasUso() << " h, "
                 << cargas[i]->getHorasUso() * cargas[i]->getPotencia() << " kWh";

            if (!cargas[i]->esCritica()) {
                const int horasMinimas = cargas[i]->getHorasMinimas();
                cout << " (Minimo: " << horasMinimas << " h)";
                const bool incumplida = cargas[i]->getHorasUso() < horasMinimas;
                if (incumplida) {
                    cout << " -> [INCUMPLIDO]";
                }
                produccionIncompleta = produccionIncompleta || incumplida;
            }
            cout << "\n";
        }

        cout << fixed << setprecision(0);
        cout << "Consumo con gestor: $" << costoConGestor << " COP\n";
        cout << "Consumo sin gestor: $" << costoSinGestor << " COP\n";

        const float ahorro = (costoSinGestor > 0.0f) ? (costoSinGestor - costoConGestor) : 0.0f;
        if (costoSinGestor > 0.0f) {
            cout << "Ahorro en pesos:    $" << ahorro << " COP (";
            cout << setprecision(1) << (ahorro / costoSinGestor) * 100.0f << " % )\n";
        }

        cout << setprecision(2);
        cout << "Medidor: " << medidor.getKWh() << " kWh, $" << medidor.getCosto() << " COP\n";

        const bool produccionCompleta = !produccionIncompleta;
        if (!produccionIncompleta) {
            cout << "\n [OK] PRODUCCION COMPLETA: Se cumplieron las horas minimas de operacion.\n";
        } else {
            cout << "\n [ALERTA] produccion incompleta: Cargas aplazables no alcanzaron las horas minimas.\n";
        }
    } // fin resumen()
}; // fin class Negocio



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


    int opcion;
    int hora = 6;
    float precio = 500.0f;
    float nivel = 50.0f;
    bool haySimulacion = false;
    do {
        cout << "\n===== WattControl: Finca lechera =====\n"
             << "1. Ingresar lectura manual (hora, precio, embalse)\n"
             << "2. Evaluar decision de cada carga\n"
             << "3. Simular un dia completo (24 h)\n"
             << "4. Ver resumen de la ultima simulacion\n"
             << "0. Salir\n";
        opcion = leerEntero("Opcion: ", 0, 4);
        switch (opcion) {
        case 1:
            hora = leerEntero("Hora (0 a 23): ", 0, 23);
            precio = leerPrecio();
            nivel = static_cast<float>(leerEntero("Embalse o reserva (0 a 100 %): ", 0, 100));
            cout << "Lectura guardada.\n";
            break;
        case 2:
            finca.evaluar(hora, precio, nivel);
            break;
         case 3:
            finca.simularDia();
            haySimulacion = true;
            break;
        case 4:
            if (haySimulacion) {
                finca.resumen();
            } else {
                cout << "Primero ejecute la opcion 3.\n";
            }
            break;
        case 0:
            cout << "Hasta luego.\n";
            break;
        default:
            cout << "Opcion invalida.\n";
        }
    } while (opcion != 0);
    return 0;
}
