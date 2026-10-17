# C++ Arena Engine (Data-Oriented Design Experiment)

A custom-built C++ auto-battler engine focused on handling large-scale entity simulations (10,000 vs 10,000 units). 

This project started as a simple sandbox to test out basic OOP and FSM (Finite State Machines) in C++. However, it quickly escalated into an experiment comparing standard **Object-Oriented Programming (OOP)** against **Data-Oriented Design (DOD) / Entity Component System (ECS)** architecture used in modern AAA engines like Unreal's Mass Entity or Unity DOTS.

*Fun fact: I used AI as an architectural mentor to bounce ideas off of, but I wrote the logic, debugged the pointers, and fought the C++ compiler myself. It was a great learning experience.*

## The Journey & Optimization

The biggest takeaway from this project was seeing firsthand how unoptimized C++ can kill performance, and how simply restructuring data in RAM can make the engine 1000x faster.

### Phase 1: The OOP Approach (Array of Structs)
Initially, I built the engine using classic OOP. Every unit was an `ArenaUnit` object holding its HP, State, and a pointer to its base stats (loaded dynamically from a CSV file). 
*   **Result:** It worked perfectly for small numbers, but as soon as I spawned 20,000 units, the CPU choked.
*   **Why?** Iterating over massive arrays of objects caused severe Cache Misses. Furthermore, units were using an O(N^2) brute-force method to find targets, checking every single entity on the board every frame.

### Phase 2: The DOD Refactor (Struct of Arrays)
I tore down the OOP structure and rewrote the engine using Data-Oriented Design.
1.  **Memory Layout:** Units are no longer objects. They are just indices. Their data is split into massive contiguous arrays (`vector<float> hps`, `vector<State> states`, etc.) inside a `Registry`.
2.  **Systems over Objects:** Instead of units updating themselves, I created "Combines" (Systems) like `BattleSystem` and `MovementSystem` that iterate strictly over the required data arrays.
3.  **Garbage Collection (Swap & Pop):** Instead of leaving `DEAD` units in memory to slow down loops, I implemented a `CleanupDeadUnits` system that uses the *Swap & Pop* method to instantly remove dead entities without memory leaks or iterator invalidation.

### Phase 3: Spatial Partitioning (Grid)
To fix the O(N^2) distance calculation nightmare, I implemented a simple 2D Grid system. The arena (1000x1000) is divided into 50x50 cells. Units now only check their own cell and the 8 surrounding cells for enemies, completely bypassing millions of unnecessary mathematical checks.

## Features
*   **Data-Driven:** Unit stats (HP, Speed, Damage, Crit Chance, Dodge, Range) are parsed from `units.csv`.
*   **FSM AI:** Simple but robust Finite State Machine (`IDLE`, `ATTACK`, `DEAD`, `VICTORY`).
*   **RNG Mechanics:** Combat includes Critical Hits and Dodges using the standard AAA `<random>` library (Mersenne Twister), completely avoiding the outdated `rand()`.
*   **Profiling:** Built-in `<chrono>` timers to measure the execution time of a single frame (turn).

## Future Plans?
Right now, this was purely a backend architectural exercise to understand how memory and cache optimization works in game development. It served its purpose perfectly. In the future, I might hook this backend up to a simple graphical interface (like Raylib or OpenGL) just to watch the 20,000 pixels actually beat each other up.
