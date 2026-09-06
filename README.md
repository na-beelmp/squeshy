# Squeeshy

A high-performance interactive data visualization and visual analytics platform designed to explore large datasets in real time using CPU and GPU acceleration.

---

## Features

- **Multi-Format Ingestion**: Supports CSV, TSV, Apache Parquet, PCAP network captures, and SQL / Elasticsearch databases.
- **Hardware Acceleration**: GPU & Multi-Core CPU accelerated computation with OpenCL and SIMD.
- **Interactive Visualizations**: Parallel coordinates, scatter plots, correlation matrices, and dynamic histograms.
- **Advanced Filtering**: Real-time multi-layer search, grouping, and mapping filters.

---

## Quick Start

### 1. Build

```bash
# Configure
cmake -S src -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build -j$(sysctl -n hw.ncpu)
```

### 2. Run

```bash
./run.sh
```

Or run the binary directly:
```bash
./build/gui-qt/src/squey
```

---

## License

This project is licensed under the MIT License.

