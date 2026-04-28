#include "cpu_usage.h"
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>

/// @brief Lee y parsea los contadores de CPU desde /proc/stat.
/// @param ruta Ruta al archivo de estadísticas.
/// @return ContadoresCPU con los valores leídos o en cero si falla.
ContadoresCPU LeerEstadisticasCPU(const std::string& ruta) {
  // Inicializar contadores en cero
  ContadoresCPU contadores{};

  try {
    // Abrir archivo
    std::ifstream archivo(ruta);

    // Activar excepciones si ocurre error en la lectura
    archivo.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    // Leer la primera línea (contadores globales de CPU)
    std::string linea;
    std::getline(archivo, linea);

    // Crear flujo para parsear la línea
    std::istringstream flujo(linea);
    std::string etiqueta;

    // Leer etiqueta inicial (debe ser "cpu")
    flujo >> etiqueta;

    // Validar que se trata de la línea global de CPU
    if (etiqueta != "cpu") {
      return contadores;
    }

    // Extraer los valores en orden
    flujo >> contadores.usuario
          >> contadores.nice
          >> contadores.sistema
          >> contadores.ocioso
          >> contadores.espera_io
          >> contadores.irq
          >> contadores.softirq
          >> contadores.robado;

  } catch (...) {
    // En caso de error, devolver estructura en cero
    return ContadoresCPU{};
  }

  return contadores;
}

/// @brief Calcula la suma total de ticks de CPU.
/// @param c Contadores de CPU.
/// @return Total de ticks acumulados.
uint64_t CalcularTicksTotales(const ContadoresCPU& c) {
  return c.usuario + c.nice + c.sistema + c.ocioso +
         c.espera_io + c.irq + c.softirq + c.robado;
}

/// @brief Calcula los ticks correspondientes al tiempo ocioso.
/// @param c Contadores de CPU.
/// @return Ticks ociosos (idle + iowait).
uint64_t CalcularTicksOciosos(const ContadoresCPU& c) {
  return c.ocioso + c.espera_io;
}

/// @brief Calcula el uso de CPU en porcentaje.
/// Se basa en dos lecturas de /proc/stat separadas por 1 segundo.
/// @return Valor entre 0.0 y 100.0 que representa el uso de CPU.
double ObtenerUsoCPU() {
  // Primera lectura
  ContadoresCPU lectura1 = LeerEstadisticasCPU();
  uint64_t total1 = CalcularTicksTotales(lectura1);
  uint64_t ocioso1 = CalcularTicksOciosos(lectura1);

  // Esperar 1 segundo (requerido para calcular diferencia)
  std::this_thread::sleep_for(std::chrono::seconds(1));

  // Segunda lectura
  ContadoresCPU lectura2 = LeerEstadisticasCPU();
  uint64_t total2 = CalcularTicksTotales(lectura2);
  uint64_t ocioso2 = CalcularTicksOciosos(lectura2);

  // Validar posibles inconsistencias (reinicio o lecturas inválidas)
  if (total2 <= total1 || ocioso2 < ocioso1) {
    return 0.0;
  }

  // Calcular diferencias entre lecturas
  uint64_t delta_total = total2 - total1;
  uint64_t delta_ocioso = ocioso2 - ocioso1;

  // Evitar división por cero
  if (delta_total == 0) {
    return 0.0;
  }

  // Aplicar fórmula de uso de CPU
  double uso = (double)(delta_total - delta_ocioso) / delta_total * 100.0;

  // Limitar el resultado al rango 0 - 100
  if (uso < 0.0) uso = 0.0;
  if (uso > 100.0) uso = 100.0;

  return uso;
}