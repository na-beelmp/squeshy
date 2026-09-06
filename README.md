<div align="center">

# ⚡ Squeeshy

### Next-Generation High-Performance Interactive Data Analytics & Visualization

[![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)](https://isocpp.org/)
[![Qt6](https://img.shields.io/badge/Qt-6.x-41CD52?style=for-the-badge&logo=qt&logoColor=white)](https://www.qt.io/)
[![OpenCL](https://img.shields.io/badge/OpenCL-GPU%2FCPU-green?style=for-the-badge&logo=khronos&logoColor=white)](https://www.khronos.org/opencl/)
[![CMake](https://img.shields.io/badge/CMake-3.20+-064F8C?style=for-the-badge&logo=cmake&logoColor=white)](https://cmake.org/)
[![License: CeCILL-C](https://img.shields.io/badge/License-CeCILL--C-blue?style=for-the-badge)](https://cecill.info/)
[![Platform](https://img.shields.io/badge/Platform-macOS%20%7C%20Linux-black?style=for-the-badge&logo=apple&logoColor=white)](#requirements)

<p align="center">
  <b>Squeeshy</b> is a blazingly fast visual data exploration and analytics suite engineered to query, correlate, and visualize millions of multi-dimensional data points in real time with hardware-accelerated rendering.
</p>

---

</div>

## 🌟 Key Features

- **⚡ Blazing Fast Hardware Acceleration**: GPU and multi-core CPU computing via **OpenCL**, **Intel TBB**, and vectorized SIMD processing.
- **📊 Interactive Visual Paradigms**:
  - **Full & Zoomed Parallel Coordinates**: Multi-dimensional correlation and trend discovery.
  - **Hit Count & Density Graphs**: Identify dense clusters and anomalies instantly.
  - **Scatter & Correlation Views**: Deep bivariate correlation across any variable.
  - **Time-Series & Series Hunters**: Dynamic temporal analysis and pattern discovery.
- **📁 Multi-Format Data Ingestion**:
  - **Log & Delimited Text**: CSV, TSV, custom regex-defined formats.
  - **Big Data Formats**: Apache Parquet with high-throughput columnar decoding.
  - **Network Captures**: Native PCAP & PCAPNG parsing via Wireshark / TShark dissection engines.
  - **Databases & Search Engines**: Direct SQL (PostgreSQL, MySQL) and Elasticsearch integration.
- **🎨 Multi-Layer Analysis & Filtering**:
  - Non-destructive layer stacking (isolate, combine, diff, and color-code subsets).
  - High-performance bitset-based filtering and instant search.
  - Python console & scriptable data pipelines for custom transformations.
- **✨ Modern & Clean UI**:
  - Polished dark and light themes with rounded controls, focus rings, and streamlined navigation cards.

---

## 🏗️ Architecture Overview

```
                        ┌─────────────────────────────────┐
                        │      Squeeshy (Qt6 GUI)         │
                        └────────────────┬────────────────┘
                                         │
                        ┌────────────────▼────────────────┐
                        │           libsquey              │
                        │ (Layers, Selections, View State)│
                        └────────┬───────────────┬────────┘
                                 │               │
            ┌────────────────────▼────┐    ┌─────▼───────────────────┐
            │    libpvparallelview    │    │       libpvkernel       │
            │ (OpenCL & QPainter GPU) │    │(Extractors, NRAW, Ingest)│
            └────────────┬────────────┘    └─────┬───────────────────┘
                         │                       │
                         └───────────┬───────────┘
                                     │
                        ┌────────────▼────────────────────┐
                        │            libpvcop             │
                        │(High-Performance Column Storage)│
                        └─────────────────────────────────┘
```

---

## 🚀 Getting Started

### Prerequisites

#### macOS (Apple Silicon / Intel)
```bash
brew install cmake ninja qt@6 boost tbb tcmalloc double-conversion pcre uchardet
```

#### Linux (Ubuntu / Debian)
```bash
sudo apt-get install -y build-essential cmake ninja-build qt6-base-dev \
    libboost-all-dev libtbb-dev libgoogle-perftools-dev libpcap-dev \
    libdouble-conversion-dev libpcre3-dev libuchardet-dev ocl-icd-opencl-dev
```

---

### 🔨 Building from Source

```bash
# 1. Clone the repository
git clone https://github.com/na-beelmp/squeshy.git
cd squeshy

# 2. Configure build with Ninja
cmake -S src -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

# 3. Compile
cmake --build build -j$(sysctl -n hw.ncpu 2>/dev/null || nproc)
```

---

### ▶️ Running Squeeshy

Simply launch the included runner script:

```bash
./run.sh
```

Or execute directly from the build directory:
```bash
./build/gui-qt/src/squey
```

---

## 💡 Quick Tips & Workflow

1. **Import Data**: Click **Import** on the start screen or drag-and-drop a `.csv`, `.parquet`, or `.pcap` file.
2. **Define Schema**: Use the built-in visual format builder to customize column types (strings, IP addresses, datetimes, numbers).
3. **Explore in Parallel Coordinates**: Drag along axis sliders to filter values dynamically.
4. **Create Layers**: Press `Ctrl+L` / `Cmd+L` to create color-coded layers and isolate specific patterns.
5. **Python Scripting**: Open the Python console (`Ctrl+P`) to run automated visual query routines.

---

## 🎨 Themes & Customization

Squeeshy comes with built-in dark and light themes. You can toggle between themes in **Preferences → Theme** or customize stylesheets located in:
- `src/gui-qt/src/resources/theme-dark.qss`
- `src/gui-qt/src/resources/theme-light.qss`

---

## 🤝 Contributing

Contributions, issues, and feature requests are welcome!
Feel free to open an issue or submit a Pull Request.

1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

---

## 📜 License

Distributed under the CeCILL-C License (LGPL compatible). See `LICENSE` and `src/COPYING` for more information.

<div align="center">
  <sub>Engineered with precision for high-performance visual data discovery.</sub>
</div>
