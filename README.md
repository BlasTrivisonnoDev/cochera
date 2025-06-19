# 🅿️ Administrador de Cochera en C

Este proyecto es una aplicación de consola escrita en C que permite gestionar una cochera (estacionamiento) de hasta 50 plazas, diferenciando por tipo de vehículo. Incluye funcionalidades para ingresar, retirar, consultar, modificar tarifas, y llevar una recaudación histórica.

---

## 📦 Funcionalidades

- 🚗 Ingresar vehículo (patente, tipo y hora)
- 🚪 Retirar vehículo (calcula tarifa y libera plaza)
- 📋 Ver estado actual de todas las plazas
- 🔍 Buscar vehículo por patente
- 💵 Modificar tarifas por tipo de vehículo:
  - Auto
  - Moto
  - Camión
- 💰 Ver recaudación total acumulada
- 💾 Guardado automático del estado y configuración

---

## 🛠️ Tecnologías

- Lenguaje: **C99**
- Entrada/Salida: Consola
- Archivos:
  - `cochera_state.bin` → guarda el estado de las plazas y la recaudación
  - `cochera_cfg.txt` → guarda las tarifas actuales por tipo

---

## 💸 Tarifas por defecto

| Tipo    | Primera hora | Hora extra |
|---------|---------------|-------------|
| Auto    | $500          | $300        |
| Moto    | $300          | $150        |
| Camión  | $800          | $500        |

> ⚙️ Podés modificar estas tarifas desde el menú o editando `cochera_cfg.txt`.

---

## ▶️ Compilación y ejecución

```bash
gcc -std=c99 -Wall -Wextra -o cochera cochera.c
./cochera
