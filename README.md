# 🗃️ Cache Management System

A terminal-based **LRU (Least Recently Used) Cache** simulator written in **C++**, featuring a two-tier memory architecture with primary cache and secondary storage, real-time performance tracking, and an interactive command-line interface.

---

## 📌 Table of Contents

- [Overview](#overview)
- [Features](#features)
- [How It Works](#how-it-works)
- [Data Structures](#data-structures)
- [Getting Started](#getting-started)
- [Usage](#usage)
- [Command Reference](#command-reference)
- [Sample Session](#sample-session)
- [Performance Metrics](#performance-metrics)
- [Project Structure](#project-structure)
- [Limitations](#limitations)
- [Future Enhancements](#future-enhancements)

---

## Overview

This project simulates a real-world caching system as used in CPUs, databases, and web servers. It implements the **LRU eviction policy** — when the cache is full, the entry that hasn't been used for the longest time is automatically removed to make space for a new one.

The system uses a **two-tier memory model**:

| Tier | Component | Speed | Capacity |
|------|-----------|-------|----------|
| L1 — Primary Cache | `LRUCache` (list + hashmap) | O(1) | User-defined |
| L2 — Secondary Storage | `SecondaryStorage` (hashmap) | O(1) | Unlimited |

When an entry is evicted from the primary cache, it is saved to secondary storage. If that entry is accessed again later, it is automatically promoted back into the cache.

---

## Features

- ✅ **O(1) GET, PUT, and DELETE** operations using `std::list` + `std::unordered_map`
- ✅ **Automatic LRU eviction** when cache is at capacity
- ✅ **Two-tier architecture** — evicted entries saved to secondary storage, not lost
- ✅ **Transparent promotion** — cache misses check secondary storage automatically
- ✅ **Live performance metrics** — hit rate, miss rate, total requests
- ✅ **Dynamic resize** — change cache capacity at runtime without data loss
- ✅ **Ranked cache display** — visualize MRU-to-LRU order in a formatted table
- ✅ **Interactive terminal interface** — full command loop with help menu

---

## How It Works

### LRU Policy

The LRU (Least Recently Used) policy ensures that the cache always retains the most recently accessed entries. When capacity is exceeded, the least recently used entry is evicted first.

```
Initial cache (capacity = 3):
  PUT A  →  [A]
  PUT B  →  [B, A]
  PUT C  →  [C, B, A]   ← full

  PUT D  →  EVICT A (LRU) → secondary
            INSERT D
            [D, C, B]

  GET B  →  HIT — move B to front
            [B, D, C]

  GET A  →  MISS — found in secondary → promote A
            EVICT C → secondary
            [A, B, D]
```

### Two-Tier Memory Flow

```
GET request
    │
    ├─► Found in primary cache?  ──YES──► Return value (HIT) ✓
    │
    └─► Not found (MISS)
            │
            ├─► Found in secondary storage?  ──YES──► Promote to cache → Return value
            │
            └─► Not found anywhere → Return empty (MISS) ✗
```

---

## Data Structures

The LRU algorithm is implemented using two containers working together:

### `std::list<CacheEntry>` — Doubly Linked List
- Front of list = **Most Recently Used (MRU)**
- Back of list = **Least Recently Used (LRU)**
- `splice()` repositions any node to front in **O(1)** — no allocation needed

### `std::unordered_map<string, list::iterator>` — Hash Map
- Maps each key to its exact position (iterator) in the list
- Enables **O(1) lookup** without traversing the list
- Iterators remain valid after `splice()` — no map updates needed on access

### `CacheEntry` Struct
```cpp
struct CacheEntry {
    std::string key;
    std::string value;
};
```

### Time Complexity

| Operation | Complexity | How |
|-----------|-----------|-----|
| `get(key)` | O(1) avg | Hash map lookup + list splice |
| `put(key, value)` | O(1) avg | Hash map insert + list push_front |
| `evict()` | O(1) | Direct access to `list.back()` + pop_back |
| `moveToFront()` | O(1) | `std::list::splice()` |

---

## Getting Started

### Prerequisites

- C++ compiler with C++11 or later (GCC, MSVC, Clang)
- Windows OS (current version uses `<windows.h>` — see [Limitations](#limitations))

### Build & Run

```bash
# Clone the repository
git clone https://github.com/your-username/cache-management-system.git
cd cache-management-system

# Compile
g++ -std=c++11 -o cache LRU.cpp

# Run
./cache
```

Or with MSVC on Windows:
```bash
cl /EHsc LRU.cpp
LRU.exe
```

---

## Usage

On startup, you are prompted to enter the cache capacity. After that, the interactive command loop begins:

```
  Enter cache capacity (number of slots): 3

  Cache initialized with capacity 3.

  Commands:
    put <key> <value>   Insert or update a key
    get <key>           Fetch a key
    del <key>           Delete a key
    resize <n>          Change cache capacity
    cache               Show primary cache table
    secondary           Show secondary storage
    stats               Show hit/miss metrics
    reset               Reset stats counters
    help                Show this menu
    exit                Quit

  >
```

After every `put`, `get`, `del`, or `resize` command, both the primary cache and secondary storage tables are automatically displayed.

---

## Command Reference

| Command | Syntax | Description |
|---------|--------|-------------|
| `put` | `put <key> <value>` | Insert or update a key-value pair |
| `get` | `get <key>` | Retrieve value; marks entry as most recently used |
| `del` | `del <key>` | Remove a key from cache or secondary storage |
| `resize` | `resize <n>` | Change capacity live; evicts LRU entries if needed |
| `cache` | `cache` | Display ranked cache (MRU → LRU) |
| `secondary` | `secondary` | Display all entries in secondary storage |
| `stats` | `stats` | Show hit rate, miss rate, cache usage |
| `reset` | `reset` | Reset all performance counters |
| `help` | `help` | Show command menu |
| `exit` | `exit` | Show final state + stats, then quit |

> Aliases: `delete` / `remove` for `del` · `setcap` / `capacity` for `resize` · `show` / `list` for `cache` · `store` / `storage` for `secondary` · `metrics` for `stats` · `quit` / `q` for `exit`

---

## Sample Session

```
  > put name Alice
  [INSERT] "name" = "Alice" added to cache

  > put age 25
  [INSERT] "age" = "25" added to cache

  > put city Chennai
  [INSERT] "city" = "Chennai" added to cache

  > put country India
  [EVICT]  "name" removed from cache → saved to secondary storage
  [INSERT] "country" = "India" added to cache

  +-------+--------------------+--------------------+
  | Rank  | Key                | Value              |
  +-------+--------------------+--------------------+
  | #1 MRU| country            | India              |
  | #2    | city               | Chennai            |
  | #3 LRU| age                | 25                 |
  +-------+--------------------+--------------------+
  Cache usage: 3 / 3

  > get name
  [MISS]   "name" not in cache. Found in secondary storage → reloading.
  [EVICT]  "age" removed from cache → saved to secondary storage
  [INSERT] "name" = "Alice" added to cache

  > stats
  +----------------------------------+
  |       Performance Metrics        |
  +----------------------------------+
  | Total requests :              1  |
  | Cache hits     :              0  |
  | Cache misses   :              1  |
  | Hit rate       :           0.0%  |
  | Cache usage    :      3 /   3   |
  +----------------------------------+
```

---

## Performance Metrics

The system tracks three counters on every `GET` request:

- **Total requests** — total number of `get` calls made
- **Cache hits** — requests served directly from primary cache
- **Cache misses** — requests not found in primary cache

```
Hit Rate (%) = (hits / total) × 100
```

> Note: A miss that results in a secondary-storage promotion is still counted as a miss — the counter reflects primary cache performance only.

Use `reset` to clear counters and measure performance over a fresh workload.

---

## Project Structure

```
cache-management-system/
│
├── LRU.cpp          # Full source — all classes and main loop in one file
└── README.md        # This file
```

### Class Overview

```
LRU.cpp
├── struct CacheEntry          - Key-value node stored in the list
├── class SecondaryStorage     - Simulated disk/backing store
│     └── unordered_map        - Unlimited key-value store
└── class LRUCache             - Main cache with LRU logic
      ├── list<CacheEntry>     - Recency-ordered doubly linked list
      ├── unordered_map        - Key → list iterator mapping
      ├── SecondaryStorage     - Embedded secondary tier
      ├── put()                - Insert / update
      ├── get()                - Retrieve with cache miss handling
      ├── remove()             - Delete from either tier
      ├── evict()              - LRU eviction to secondary
      ├── moveToFront()        - Mark entry as MRU via splice
      ├── setCapacity()        - Live resize with auto-eviction
      └── displayStats()       - Hit/miss performance report
```

---

## Limitations

- **Windows only** — includes `<windows.h>`; remove this header for cross-platform compatibility (it is unused functionally)
- **No thread safety** — concurrent access will cause undefined behaviour; needs `std::shared_mutex` for multi-threaded use
- **String types only** — keys and values are `std::string`; a templated version would support arbitrary types
- **In-memory secondary storage** — all data is lost on exit; no file-based persistence
- **No TTL** — cache entries never expire automatically; no time-based eviction

---

## Future Enhancements

- [ ] Cross-platform build with CMake (remove Windows dependency)
- [ ] Thread-safe cache with reader-writer locking (`std::shared_mutex`)
- [ ] Template parameters `<KeyType, ValueType>` for generic support
- [ ] File-backed secondary storage for session persistence
- [ ] TTL (Time-to-Live) with timestamp-based expiry
- [ ] LFU (Least Frequently Used) as an alternative eviction policy
- [ ] Unit test suite with Google Test

---

## Concepts Demonstrated

This project is a practical demonstration of the following computer science topics:

- **Cache replacement policies** (LRU)
- **Doubly linked list** with O(1) splice
- **Hash map + linked list combination** for O(1) cache operations
- **Two-tier memory hierarchy** (L1 cache + L2 backing store)
- **Amortized O(1) operations** via iterator stability
- **Interactive CLI design** in C++

---

## License

This project is open source and available under the [MIT License](LICENSE).

---

> Built with C++ · Data Structures · System Design
