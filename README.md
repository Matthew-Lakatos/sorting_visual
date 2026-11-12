# Sorting Central — SFML Edition

A C++23 modular sorting visualiser using SFML for animated bar-chart playback.

---

## Dependencies

| Dependency | Version | Install |
|------------|---------|---------|
| C++ compiler | Clang 17+ / GCC 14+ / MSVC 19.36+ | — |
| CMake | 3.28+ | — |
| SFML | 2.6+ | See below |

### Installing SFML

**Linux (Debian/Ubuntu)**
```bash
sudo apt install libsfml-dev
```

**macOS**
```bash
brew install sfml
```

**Windows (vcpkg)**
```powershell
vcpkg install sfml
```

---

## Building

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/sorting_central
```

---

## SFML Window Controls

| Key | Action |
|-----|--------|
| `Space` | Play / Pause |
| `→` / `←` | Step forward / backward one frame |
| `↑` / `↓` | Increase / decrease playback speed |
| `R` | Restart from the first frame |
| `Esc` | Close the window |

---

## Colour Legend

| Colour | Meaning |
|--------|---------|
| Steel-blue | Default / unsorted element |
| Amber | Two elements being compared |
| Red | Two elements being swapped |
| Green | Element in its final sorted position |
| Magenta | Pivot (Quick sort) |
| Cyan | Element being placed into a radix bucket |

---

## Supported Algorithms

| Key | Algorithm | Stable? |
|-----|-----------|---------|
| `a` | Bubble sort | Yes |
| `b` | Merge sort (unstable) | No |
| `bs` | Merge sort (stable) | Yes |
| `c` | Insertion sort | Yes |
| `d` | Quick sort | No |
| `e` | Heap sort | No |
| `f` | Bogo sort | — |
| `g` | LSD Radix sort | Yes (integers only) |
| `h` | MSD Radix sort | No (integers + strings) |
