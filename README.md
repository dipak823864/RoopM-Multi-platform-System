# 🚀 RoopM Multi-platform System
> **An Ultra-Lightweight, Blazing-Fast, Modular Web Browser & UI Runtime Engine**

[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Android-blue.svg)](#platform-support)
[![Language](https://img.shields.io/badge/Language-C%2B%2B%20%2F%20C%20%2F%20JavaScript-orange.svg)](#technology-stack)
[![Rendering](https://img.shields.io/badge/Renderer-Blend2D%20%2B%20AsmJit-green.svg)](https://blend2d.com)
[![Layout](https://img.shields.io/badge/Layout-Meta%20Yoga%20(Flexbox)-purple.svg)](https://yogalayout.dev/)
[![JavaScript](https://img.shields.io/badge/JS%20Engine-QuickJS%20(ES2020)-yellow.svg)](https://bellard.org/quickjs/)

---

## 📌 Overview

**RoopM** is an embeddable, modular, high-performance web browser and multi-platform rendering engine. Unlike monolithic engines such as Chromium or WebKit that require gigabytes of dependencies and hundreds of megabytes of RAM, RoopM is engineered from the ground up for **extreme speed, minimal footprint, and direct low-level hardware control**.

It combines battle-tested industry-standard native engines (Blend2D, Lexbor, Yoga, and QuickJS) into a cohesive, high-performance platform runtime suitable for desktop, mobile (Android), and embedded systems.

---

## 🏗 System Architecture & Technology Stack

RoopM orchestrates a unified pipeline connecting HTML parsing, CSS style resolution, Flexbox layout computing, JavaScript execution, and 2D hardware-accelerated rasterization:

```
                  ┌───────────────────────────────┐
                  │    HTML5 / CSS3 Input / JS    │
                  └───────────────┬───────────────┘
                                  │
                   ┌──────────────┴──────────────┐
                   ▼                             ▼
        ┌────────────────────┐         ┌────────────────────┐
        │       Lexbor       │         │      QuickJS       │
        │ HTML/CSS Parser &  │         │  Lightweight ES2020│
        │   DOM Tree Engine  │         │  JavaScript Engine │
        └──────────┬─────────┘         └─────────┬──────────┘
                   │                             │
                   └──────────────┬──────────────┘
                                  │ (DOM & Events)
                                  ▼
                     ┌────────────────────────┐
                     │       Meta Yoga        │
                     │ Flexbox / Layout Engine│
                     └────────────┬───────────┘
                                  │ (Computed Geometry & Boxes)
                                  ▼
                     ┌────────────────────────┐
                     │    Blend2D + AsmJit    │
                     │ High-Performance JIT   │
                     │ 2D Vector Rasterizer   │
                     └────────────┬───────────┘
                                  │ (Direct Pixel Buffer)
                                  ▼
                     ┌────────────────────────┐
                     │ Platform Abstraction   │
                     │ (Win32 / Android NDK)  │
                     └────────────────────────┘
```

### 🧩 Core Components
* **2D Graphics & Rendering Pipeline ([Blend2D](https://blend2d.com/) + [AsmJit](https://asmjit.com/)):** High-speed software/JIT 2D vector rasterization engine that generates machine code dynamically for ultra-smooth anti-aliasing, linear/radial gradients, and path clipping.
* **HTML & CSS Engine ([Lexbor](https://lexbor.com/)):** High-performance, spec-compliant C-based HTML5 tokenizer, tree constructor, and CSS selector resolver.
* **Layout Engine ([Meta Yoga](https://yogalayout.dev/)):** Efficient Flexbox engine responsible for box models, flex wrapping, nested alignments, and viewport recalculations.
* **Scripting Runtime ([QuickJS](https://bellard.org/quickjs/)):** Complete ES2020 JavaScript engine created by Fabrice Bellard, delivering instant initialization and near-zero memory consumption.
* **Platform Layer:** Cross-platform abstraction separating OS-specific windowing and surface management (`Win32` for Windows, `NDK/JNI` for Android).

---

## ✨ Key Features

- [x] **Lightning Fast Startup:** Starts and renders within milliseconds without heavy runtime bloat.
- [x] **Spec-Compliant HTML5 Parsing:** Full DOM tree lifecycle with element traversal and attribute management.
- [x] **Modern CSS & Flexbox Support:** Resolves nested Flexbox hierarchies, borders, margins, padding, and text-flow accurately.
- [x] **JIT 2D Vector Rendering:** Anti-aliased geometry, images, canvas primitives, and typography rendered with sub-pixel precision.
- [x] **Embedded ECMAScript Runtime:** Complete JavaScript bindings to DOM events and manipulation routines.
- [x] **Multi-Platform Support:** Ready-to-build configurations for Windows (`Win32`) and Android (`android_build`).
- [x] **Built-in Diagnostic & Pixel Testing Suites:** Pixel-diff verification tools, alignment testers, and wrap auditors.

---

## 📂 Repository Structure

```
RoopM-Multi-platform-System/
├── android_build/         # Android NDK, CMake, and mobile build toolchains
├── app/                   # Browser host application and window shells
├── build/                 # Compiled binaries, static libs, and intermediate objects
├── core/                  # Core browser engine (DOM bridge, render tree, event loops)
├── docs/                  # Technical documentation and internal specs
├── platform/              # OS abstraction layers (Win32, Android display surfaces)
├── tests_src/             # Diagnostic, pixel regression, and layout unit tests
├── tools/                 # Build tooling, asset compilers, and audit utilities
├── vendor/                # Native dependencies:
│   ├── asmjit/            # Machine-code JIT generation library
│   ├── blend2d/           # 2D Vector rendering engine
│   ├── lexbor/            # HTML5/CSS3 parser and DOM implementation
│   ├── quickjs/           # Embedded JavaScript interpreter
│   └── yoga/              # Meta's Flexbox layout system
├── workspace/             # Demo application and interactive test pages
│   ├── index.html         # Main workspace entry point
│   ├── fleet.html         # Fleet dashboard demo
│   ├── telemetry.html     # High-frequency telemetry visualization
│   └── style.css          # Core CSS styling test suite
├── Features.md            # Detailed feature matrix and milestone roadmap
├── run_web.py             # Local workspace web server runner
└── build_*.bat / run_*.bat# Automated build and test scripts
```

---

## 🛠 Getting Started (Windows)

### Prerequisites
* **Windows 10 / 11** (x86_64)
* **MSVC C++ Build Tools** (Visual Studio 2019/2022 or Clang-cl)
* **Python 3.8+** (Optional, for local server & diagnostic runner)

### 1. Build and Run the Host Browser
To compile the core engine and launch the workspace application:
```cmd
build_and_run.bat
```

### 2. Verify Layout and Diagnostic Checks
RoopM comes packed with comprehensive diagnostic suites to ensure layout correctness:
* **Layout & Alignments:** Run `build_diag_align.bat`
* **Pixel Accuracy Tests:** Run `run_pixel_tests.bat`
* **Full Master Suite:** Run `run_master_tests.bat`
* **Workspace Audit:** Run `build_workspace_audit.bat`

### 3. Run the Local Test Server
To preview the sample workspace telemetry pages over HTTP:
```cmd
python run_web.py
```

---

## 📱 Android Support

The `android_build/` directory includes the Native Development Kit (NDK) setup and JNI bridges required to run the RoopM engine on Android devices as an embedded native surface.

---

## 🧪 Workspace Demo: Telemetry & Fleet Suite

The `workspace/` directory showcases RoopM's layout and rendering capabilities under real-world conditions:
* **Interactive UI:** Dynamic telemetry dashboards, metrics inspection, and fluid animations.
* **Component Testing:** Flexbox cards, typography scaling, image compositing, and DOM event responsiveness.

---

## 🤝 Contributing

Contributions are welcome! If you're interested in helping develop:
1. **Fork** the Repository.
2. **Create** a feature branch (`git checkout -b feature/awesome-feature`).
3. **Commit** your changes (`git commit -m "Add new layout feature"`).
4. **Push** to your branch (`git push origin feature/awesome-feature`).
5. **Open** a Pull Request.

---

## 📄 License & Acknowledgments

This project is licensed under the terms of the project repository.  
Special thanks to the open-source projects powering the RoopM engine:
* [Blend2D](https://blend2d.com) & [AsmJit](https://asmjit.com) (Petr Kobalíček and contributors)
* [Lexbor](https://lexbor.com) (Alexander Borisov and contributors)
* [Yoga](https://yogalayout.dev) (Meta Platforms, Inc.)
* [QuickJS](https://bellard.org/quickjs) (Fabrice Bellard and Charlie Gordon)