#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace pulso::core {

/**
 * @brief Representa una metrica individual del sistema con sus metadatos
 *
 * Una metrica encapsula un valor de medicion puntual, junto con el nombre
 * que lo identifica, la unidad en que se expresa y el momento en que fue tomado
 */
struct Metrica {
    /** @brief Nombre identificador de la metrica. Ej: "cpu.usage", "ram.used" */
    std::string nombre;

    /** @brief Valor numerico de la medicion */
    double valor;

    /** @brief Unidad en que se expresa el valor. Ej: "porcentaje", "bytes" */
    std::string unidad;

    /** @brief Momento en que se tomo la medición, expresado como Unix timestamp en segundos. */
    std::int64_t timestamp;
};

/**
 * @brief Representa un conjunto de métricas capturadas en un instante determinado.
 *
 * Un Snapshot agrupa todas las Metrica recolectadas durante un mismo ciclo de muestreo,
 * permitiendo tener una vision completa del estado del sistema en un momento puntual.
 */
struct Snapshot {
    /** @brief Momento en que se realizo el muestreo, expresado como Unix timestamp en segundos. */
    std::int64_t timestamp;

    /** @brief Coleccion de métricas individuales capturadas en este snapshot. */
    std::vector<Metrica> metricas;
};
/**
 * @brief Retorna una copia del snapshot con solo las metricas cuyo nombre
 *        hace match con un patron glob simple.
 *
 * Soporta '*' (cero o mas caracteres) y '?' (exactamente un caracter),
 * al estilo de los patrones glob de Unix:
 *   "cpu.*"       -> todas las metricas que empiezan por "cpu."
 *   "network.rx"  -> exactamente la metrica "network.rx"
 *   "*"           -> todas (sin filtro efectivo)
 *
 * Si @p patron esta vacio, retorna el snapshot sin modificar.
 * El snapshot original nunca se modifica: esta funcion es de presentacion.
 *
 * @param snapshot Snapshot original con todas las metricas.
 * @param patron   Patron glob simple para filtrar por nombre de metrica.
 * @return Nuevo Snapshot con el mismo timestamp pero solo las metricas
 *         cuyo nombre hace match con @p patron.
 */
inline Snapshot filtrarSnapshot(const Snapshot& snapshot,
                                const std::string& patron)
{
    if (patron.empty()) {
        return snapshot;
    }

    // Matching glob iterativo: evita recursion para patrones degenerados.
    auto globMatch = [](const std::string& texto,
                        const std::string& pat) -> bool {
        const char* t  = texto.c_str();
        const char* p  = pat.c_str();
        const char* st = nullptr; // posicion del ultimo '*' en pat
        const char* ss = nullptr; // posicion en texto cuando se vio ese '*'

        while (*t != '\0') {
            if (*p == '*') {
                st = p++;
                ss = t;
            } else if (*p == '?' || *p == *t) {
                ++p; ++t;
            } else if (st != nullptr) {
                p = st + 1;
                t = ++ss;
            } else {
                return false;
            }
        }
        while (*p == '*') { ++p; }
        return *p == '\0';
    };

    Snapshot resultado;
    resultado.timestamp = snapshot.timestamp;
    for (const auto& m : snapshot.metricas) {
        if (globMatch(m.nombre, patron)) {
            resultado.metricas.push_back(m);
        }
    }
    return resultado;
}

} // namespace pulso::core