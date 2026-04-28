#ifndef CPU_USAGE_H
#define CPU_USAGE_H

#include <cstdint>
#include <string>

/// @brief Estructura que representa los contadores de tiempo de CPU
/// obtenidos desde la primera línea de /proc/stat.
/// Cada campo representa tiempo acumulado en distintos estados.
struct ContadoresCPU {
  uint64_t usuario{};     ///< Tiempo en modo usuario
  uint64_t nice{};        ///< Tiempo en modo usuario con prioridad ajustada
  uint64_t sistema{};     ///< Tiempo en modo kernel (sistema)
  uint64_t ocioso{};      ///< Tiempo en estado idle (sin uso)
  uint64_t espera_io{};   ///< Tiempo esperando operaciones de entrada/salida (I/O)
  uint64_t irq{};         ///< Tiempo atendiendo interrupciones de hardware
  uint64_t softirq{};     ///< Tiempo atendiendo interrupciones de software
  uint64_t robado{};      ///< Tiempo "robado" por otras máquinas virtuales
};

/// @brief Lee y parsea la primera línea del archivo /proc/stat.
/// @param ruta Ruta al archivo (por defecto "/proc/stat").
/// @return Estructura ContadoresCPU con los valores leídos.
/// @note En caso de error, retorna todos los valores en cero.
ContadoresCPU LeerEstadisticasCPU(const std::string& ruta = "/proc/stat");

/// @brief Calcula la suma total de ticks de CPU.
/// @param c Estructura con contadores de CPU.
/// @return Total de ticks acumulados.
uint64_t CalcularTicksTotales(const ContadoresCPU& c);

/// @brief Calcula los ticks correspondientes al tiempo ocioso.
/// @param c Estructura con contadores de CPU.
/// @return Ticks ociosos (idle + iowait).
uint64_t CalcularTicksOciosos(const ContadoresCPU& c);

/// @brief Calcula el uso de CPU en porcentaje (0.0 - 100.0).
/// Realiza dos lecturas de /proc/stat con un intervalo de 1 segundo.
/// @return Porcentaje de uso de CPU.
/// @note Si ocurre un error o los datos no son válidos, retorna 0.0.
double ObtenerUsoCPU();

#endif // CPU_USAGE_H