<p align="center">
  <a href="https://github.com/mohammadraziei/libgsp">
    <img src="https://github.com/MohammadRaziei/libgsp/raw/master/docs/images/libgsp-logo-sq.svg" width="120px" alt="libGSP Logo">
  </a>
</p>

<h2 align="center">libGSP</h2>
<h4 align="center">A High-Performance Graph Signal Processing Library</h4>
<h5 align="center">Lightweight · Multi-language · Innovative Visualization</h5>

<p align="center">
  <a href="https://github.com/mohammadraziei/libgsp"><img src="https://img.shields.io/static/v1?label=mohammadraziei&message=libgsp&color=white&logo=github"></a>
  <a href="https://github.com/mohammadraziei/libgsp/forks"><img src="https://img.shields.io/github/forks/mohammadraziei/libgsp?style=social"></a>
  <a href="https://github.com/mohammadraziei/libgsp/issues"><img src="https://img.shields.io/github/issues/mohammadraziei/libgsp"></a>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/C++-17-blue" />
  <img src="https://img.shields.io/badge/Python-3.8%20|%203.9%20|%203.10%20|%203.11-blue" />
  <img src="https://img.shields.io/badge/Matlab-Support%20Planned-orange" />
  <img src="https://img.shields.io/badge/JavaScript-WebGL/three.js-green" />
  <img src="https://img.shields.io/badge/License-MIT-purple" />
</p>

---

## 🚀 Introduction

**libGSP** is a **lightweight, high-performance library** for **Graph Signal Processing (GSP)**, built on top of the [Eigen](https://eigen.tuxfamily.org/) linear algebra library.  
It is designed to handle both **sparse** and **dense** graphs efficiently, from small research problems to **large-scale graphs** with millions of nodes.

With a modern **C++17 core**, bindings for **Python**, planned support for **Matlab**, and **JavaScript (WebGL/three.js)** visualization, libGSP offers a truly **multi-language ecosystem**.

---

## ✨ Key Features

- ⚡ **Eigen-based** → extremely fast, lightweight, and memory-efficient.  
- 🎼 **Graph Fourier Transform (GFT)** → spectral analysis on graphs made easy.  
- 🔗 **Supports sparse & dense graphs** with optimized operators.  
- 📈 **Graph Learning** → tools for machine learning on graph signals.  
- 🏗️ **Large-scale graph ready** → optimized for real-world big graphs.  
- 🌍 **Multi-language** → C++ core, Python bindings, Matlab interface (planned), and JS/three.js visualization.  
- 🖼️ **Innovative visualization** → 2D (SVG) and 3D (WebGL) graph rendering, powered by [three.js](https://threejs.org/).  
- 📥 **Data import from Gephi** → easy integration with graph datasets and visual models.  

---

## 📺 Demo

<p align="center">
  <a href="https://github.com/mohammadraziei/libgsp">
    <img src="https://github.com/MohammadRaziei/libgsp/raw/master/docs/images/in-the-construction.png" width="400px" alt="Under Construction">
  </a>
</p>

👉 Watch our **graph visualization demo** (2D & 3D):  
[![Demo Video](https://img.shields.io/badge/Demo-Video-blue?logo=youtube)](https://github.com/mohammadraziei/libgsp)

---

## 🛠️ Build & Test

### Build C++ core
```bash
cmake -B build 
cmake --build build 
ctest --test-dir build 
ctest --test-dir build --extra-verbose
````

### Python bindings

```bash
pip install libgsp
```

(Matlab & JavaScript support are under development.)

---

## 🔮 Roadmap

* [x] C++17 core library (Eigen-based)
* [x] Graph Fourier Transform & spectral operations
* [ ] Python bindings (in progress)
* [ ] Matlab interface (planned)
* [ ] JavaScript/WebGL demos
* [ ] Import/export for more graph data formats
* [ ] Tutorials & educational notebooks

---

## 👷 Project Status

<p align="center">
  <img src="https://github.com/MohammadRaziei/libgsp/raw/master/docs/images/under-construction.png" width="300px" alt="Under Construction" />
</p>

libGSP is **actively under development**. Contributions, feedback, and feature requests are very welcome!

---

## 📚 License

This project is released under the [MIT License](LICENSE).
Free for academic, personal, or commercial use.

---

## 🙌 Acknowledgments

* Built on top of the powerful [Eigen](https://eigen.tuxfamily.org/) library.
* Visualization powered by [three.js](https://threejs.org/).
* Inspired by the growing **Graph Signal Processing** research community.

