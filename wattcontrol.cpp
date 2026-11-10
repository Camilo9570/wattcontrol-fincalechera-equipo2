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

