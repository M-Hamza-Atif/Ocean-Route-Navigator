# Ocean-Route-Navigator
C++ Maritime Logistics Engine with SFML Visualization.

# ⚓ Ocean Route Navigator - Maritime Logistics Engine

**Ocean Route Navigator** is a high-performance maritime route optimization system built in **C++17** and visualized with **SFML**.
Unlike standard implementations, this project was built with a strict **"No STL" (Standard Template Library)** constraint, relying entirely on custom-built Data Structures (Vectors, Lists, Queues, Heaps) to manage complex logic.

## ⚡ Key Features

* **Global Route Visualization:** Interactive SFML-based world map with dynamic ghost-lines and active path highlighting.
* **Advanced Algorithms:**
    * **Dijkstra:** Optimized for both *Cost ($)* and *Time (Duration)*.
    * **A\* Search:** Heuristic-based pathfinding for rapid queries.
    * **Multi-Leg Routing:** "Builder Mode" to construct complex itineraries (A -> B -> C) with automatic time-continuity checks.
* **Simulation Mechanics:**
    * **Time-Dependent Logic:** Calculates wait times based on specific departure schedules (prevents "time travel").
    * **Docking Simulation:** Visualizes queue depth per shipping company at every port.
    * **Subgraph Filtering:** Company Viewer filters the map to show specific fleets (Maersk, MSC, etc.).
* **Modern Dashboard UI:**
    * **Interactive Control Panel:** Clean, professional side panel  featuring user-friendly settings.
    * **Live Analytics & Itinerary:** Bottom panel to display real-time path statistics or visualize itinerary.
    * **Adaptive Map Legend:** Context-aware overlay that dynamically updates based on the active mode.

## 🧮 Custom Data Structures

To demonstrate core computer science concepts, all containers were built from scratch:

| Feature | Custom Structure | Purpose |
| :--- | :--- | :--- |
| **Graph Topology** | `Adjacency List` | Stores the global port network and routes. |
| **Pathfinding** | `MinHeap` | Binary Heap for Dijkstra/A* priority queue operations. |
| **Core Storage** | `SimpleVector` | Dynamic array used throughout for resizing/storage. |
| **Itinerary** | `SimpleList` | Linked List for storing booked multi-leg journeys. |
| **Logistics** | `SimpleQueue` | Manages docking queues for individual companies. |


## 📂 Project Structure

```text
Ocean-Route-Navigator/
├── assets/               # Fonts and Icons
│   ├── arial.ttf
│   ├── icon.jpg
│   └── world_map.png
├── data/                 # Simulation Datasets
│   ├── PortCharges.txt
│   └── Routes.txt
├── include/              # Header Files
│   ├── Algorithms.h      # Dijkstra, A*, Multi-leg Logic
│   ├── DataStructs.h     # Port, Route, Ship Models
│   ├── Visualizer.h      # SFML Render Engine & UI State Machine
│   ├── UIComponents.h    # Custom UI Widgets (Toggles, Inputs)
│   ├── Graph.h           # Directed Graph Implementation
│   ├── FileParser.h      # Data Loading Logic
│   ├── MinHeap.h
│   ├── SimpleList.h
│   ├── SimpleQueue.h
│   └── SimpleVector.h
├── src/                  # Source Code
│   └── main.cpp          # Application Entry Point
├── LICENSE               # MIT License
└── README.md             # Project Documentation
```

## ⚙️ How to Build & Run

**Requirements:**
* C++17 (or later)
* Compiler (MinGW, Clang, MSVC)
* SFML 2.6.X Library

**Build Command (Terminal):**
```bash
g++ -std=c++17 src/*.cpp -I include -lsfml-graphics -lsfml-window -lsfml-system -o OceanRoute
```

**Run:**
```bash
./OceanRoute
```

## ✍️ Author
Muhammad Hamza Atif - BS Software Engineering, FAST NUCES Islamabad
