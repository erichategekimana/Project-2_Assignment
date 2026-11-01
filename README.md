# Project 2 Assignments - Programming in C

This repository contains three projects of Project 2_Assignment. Each project demonstrates core computer science and C programming paradigms, including dynamic memory allocation, pointer arithmetic, custom data structures (`struct` and `union`), and function pointer dispatch tables/callbacks.

---

## 📁 Repository Structure

```text
Project-2_Assignment/
├── README.md
├── project_1/
│   ├── academic_engine.c                       # Project 1: Academic Performance Engine
│   └── a.out                                   # Compiled executable
├── project_2/
│   ├── dynamic_text_and_linguistic_analyzer.c  # Project 2: Text & Linguistic Analyzer
│   └── a.out                                   # Compiled executable
└── project_3/
    ├── IoT_event_monitoring_and_callback_engine.c # Project 3: IoT Monitoring & Callback Engine
    └── a.out                                   # Compiled executable
```

---

## Projects Overview

### Project 1: Academic Performance Engine
- **File:** `project_1/academic_engine.c`
- **Description:** A student record management and academic performance analysis system. It dynamically manages student records (ID, name, course, and grades) with resizing arrays, performs statistical evaluations using pointer arithmetic, and provides sorting and academic risk analysis.
- **Key Concepts:** Dynamic memory (`malloc`/`realloc`/`free`), pointer arithmetic, function pointer dispatch table for menu handling.

### Project 2: Dynamic Text and Linguistic Analyzer
- **File:** `project_2/dynamic_text_and_linguistic_analyzer.c`
- **Description:** A text mining engine and linguistic analysis tool that tokenizes dynamic text input (or a built-in corpus from SICP Chapter 1), computes word frequencies, identifies longest/most frequent terms, and calculates readability metrics.
- **Key Concepts:** Dynamic string buffer growth, custom `struct wordStat` frequency tables, sorting algorithms, modular function pointer execution.

### Project 3: IoT Event Monitoring and Callback Engine
- **File:** `project_3/IoT_event_monitoring_and_callback_engine.c`
- **Description:** An event-driven IoT sensor monitoring system supporting diverse sensor types (Temperature, Pressure, Battery Voltage). It evaluates real-time telemetry against operating limits and sensor drift thresholds, triggering automated event callbacks.
- **Key Concepts:** Memory-efficient `union` structures for sensor readings, threshold alerting, callback architectures via function pointers, math computations.

---

## Prerequisites

Ensure you have a C compiler installed (such as `gcc` or `clang`):

```bash
# Check GCC version
gcc --version
```

---

## How to Compile and Run

You can compile and run each project individually by navigating into its directory or directly from the repository root.

### Method 1: Navigate to the Project Folder (Recommended)

#### Project 1: Academic Engine
```bash
cd project_1

# Compile to default a.out
gcc academic_engine.c

# Run the executable
./a.out
```

#### Project 2: Dynamic Text Analyzer
```bash
cd project_2

# Compile to default a.out
gcc dynamic_text_and_linguistic_analyzer.c

# Run the executable
./a.out
```

#### Project 3: IoT Monitoring Engine
> *Note: Project 3 uses `<math.h>`, so the `-lm` flag is recommended when compiling.*

```bash
cd project_3

# Compile to default a.out with math library link
gcc IoT_event_monitoring_and_callback_engine.c -lm

# Run the executable
./a.out
```

---

### Method 2: Compile with Custom Output Binary Names

If you prefer naming your executable instead of using the default `a.out`, use the `-o` flag:

```bash
# Project 1
gcc -Wall -Wextra project_1/academic_engine.c -o project_1/academic_engine
./project_1/academic_engine

# Project 2
gcc -Wall -Wextra project_2/dynamic_text_and_linguistic_analyzer.c -o project_2/text_analyzer
./project_2/text_analyzer

# Project 3
gcc -Wall -Wextra project_3/IoT_event_monitoring_and_callback_engine.c -lm -o project_3/iot_engine
./project_3/iot_engine
```

---

## Recommended Compiler Flags

For strict standards compliance and clean compilation, you can include warning flags:
```bash
gcc -Wall -Wextra -std=c11 <filename>.c -o <output_binary>
```
