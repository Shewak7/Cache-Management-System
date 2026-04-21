
#include <iostream>
#include <unordered_map>
#include <list>
#include <string>
#include <iomanip>
#include <sstream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <windows.h>
using namespace std;
struct CacheEntry {
    std::string key;
    std::string value;
};

// ─────────────────────────────────────────────────────────────
//  Secondary Storage  (simulates disk / slow backing store)
// ─────────────────────────────────────────────────────────────
class SecondaryStorage {
private:
    std::unordered_map<std::string, std::string> store;

public:
    void write(const std::string& key, const std::string& value) {
        store[key] = value;
    }

    std::string read(const std::string& key) const {
        auto it = store.find(key);
        return (it != store.end()) ? it->second : "";
    }

    bool exists(const std::string& key) const {
        return store.count(key) > 0;
    }

    void remove(const std::string& key) {
        store.erase(key);
    }

    void display() const {
        if (store.empty()) {
            std::cout << "  (empty)\n";
            return;
        }
        std::cout << "  +-----------------+---------------------+\n";
        std::cout << "  | Key             | Value               |\n";
        std::cout << "  +-----------------+---------------------+\n";
        for (auto& p : store) {
            std::cout << "  | " << std::setw(15) << std::left << p.first
                      << " | " << std::setw(19) << std::left << p.second
                      << " |\n";
        }
        std::cout << "  +-----------------+---------------------+\n";
    }

    int size() const { return (int)store.size(); }
    const std::unordered_map<std::string, std::string>& all() const { return store; }
};

// ─────────────────────────────────────────────────────────────
//  LRU Cache
// ─────────────────────────────────────────────────────────────
class LRUCache {
private:
    int capacity;
    std::list<CacheEntry> cacheList;   // front = MRU, back = LRU
    std::unordered_map<std::string, std::list<CacheEntry>::iterator> cacheMap;
    SecondaryStorage secondary;

    int hits   = 0;
    int misses = 0;
    int total  = 0;

    void moveToFront(std::list<CacheEntry>::iterator it) {
        cacheList.splice(cacheList.begin(), cacheList, it);
    }

    void evict() {
        auto& lru = cacheList.back();
        std::cout << "  [EVICT]  \"" << lru.key << "\" removed from cache → saved to secondary storage\n";
        secondary.write(lru.key, lru.value);
        cacheMap.erase(lru.key);
        cacheList.pop_back();
    }

    // Print a padded, truncated string inside table cell
    static std::string cell(const std::string& s, int width) {
        if ((int)s.size() > width) return s.substr(0, width - 2) + "..";
        return s + std::string(width - (int)s.size(), ' ');
    }

public:
    LRUCache(int cap) : capacity(cap) {}

    // ── PUT ──────────────────────────────────────────────────
    void put(const std::string& key, const std::string& value) {
        auto it = cacheMap.find(key);
        if (it != cacheMap.end()) {
            it->second->value = value;
            moveToFront(it->second);
            std::cout << "  [UPDATE] \"" << key << "\" updated to \"" << value << "\"\n";
        } else {
            if ((int)cacheList.size() >= capacity) evict();
            cacheList.push_front({key, value});
            cacheMap[key] = cacheList.begin();
            // if it was in secondary, remove it since it's now in cache
            if (secondary.exists(key)) secondary.remove(key);
            std::cout << "  [INSERT] \"" << key << "\" = \"" << value << "\" added to cache\n";
        }
    }

    // ── GET ──────────────────────────────────────────────────
    std::string get(const std::string& key) {
        total++;
        auto it = cacheMap.find(key);
        if (it != cacheMap.end()) {
            hits++;
            moveToFront(it->second);
            std::cout << "  [HIT]    \"" << key << "\" found in cache → \"" << it->second->value << "\"\n";
            return it->second->value;
        }

        misses++;
        // Check secondary storage
        std::string val = secondary.read(key);
        if (!val.empty()) {
            std::cout << "  [MISS]   \"" << key << "\" not in cache. Found in secondary storage → reloading.\n";
            secondary.remove(key);
            put(key, val);
            return val;
        }

        std::cout << "  [MISS]   \"" << key << "\" not found in cache or secondary storage.\n";
        return "";
    }

    // ── DELETE ───────────────────────────────────────────────
    bool remove(const std::string& key) {
        auto it = cacheMap.find(key);
        if (it != cacheMap.end()) {
            cacheList.erase(it->second);
            cacheMap.erase(it);
            std::cout << "  [DELETE] \"" << key << "\" removed from cache.\n";
            return true;
        }
        if (secondary.exists(key)) {
            secondary.remove(key);
            std::cout << "  [DELETE] \"" << key << "\" removed from secondary storage.\n";
            return true;
        }
        std::cout << "  [DELETE] \"" << key << "\" not found anywhere.\n";
        return false;
    }

    // ── Resize capacity live ──────────────────────────────────
    void setCapacity(int newCap) {
        if (newCap < 1) { std::cout << "  [ERROR]  Capacity must be >= 1\n"; return; }
        capacity = newCap;
        while ((int)cacheList.size() > capacity) evict();
        std::cout << "  [CONFIG] Cache capacity set to " << capacity << "\n";
    }

    // ── Ranked table ─────────────────────────────────────────
    void displayCache() const {
        std::cout << "\n  +-------+--------------------+--------------------+\n";
        std::cout << "  | Rank  | Key                | Value              |\n";
        std::cout << "  +-------+--------------------+--------------------+\n";
        if (cacheList.empty()) {
            std::cout << "  | (cache is empty)                               |\n";
        } else {
            int rank = 1;
            int total_entries = (int)cacheList.size();
            for (auto& e : cacheList) {
                std::string rankStr = "#" + std::to_string(rank);
                if (rank == 1)            rankStr += " MRU";
                else if (rank == total_entries) rankStr += " LRU";
                std::cout << "  | " << std::setw(5) << std::left << rankStr
                          << " | " << cell(e.key, 18)
                          << " | " << cell(e.value, 18) << " |\n";
                rank++;
            }
        }
        std::cout << "  +-------+--------------------+--------------------+\n";
        std::cout << "  Cache usage: " << cacheList.size() << " / " << capacity
                  << "   (Rank 1 = Most Recently Used | Last = LRU)\n\n";
    }

    // ── Secondary storage table ───────────────────────────────
    void displaySecondary() const {
        std::cout << "\n  Secondary Storage (" << secondary.size() << " entries):\n";
        secondary.display();
        std::cout << "\n";
    }

    // ── Stats ────────────────────────────────────────────────
    void displayStats() const {
        double rate = (total > 0) ? (100.0 * hits / total) : 0.0;
        std::cout << "\n  +----------------------------------+\n";
        std::cout << "  |       Performance Metrics        |\n";
        std::cout << "  +----------------------------------+\n";
        std::cout << "  | Total requests : " << std::setw(14) << total    << " |\n";
        std::cout << "  | Cache hits     : " << std::setw(14) << hits     << " |\n";
        std::cout << "  | Cache misses   : " << std::setw(14) << misses   << " |\n";
        std::cout << "  | Hit rate       : " << std::setw(12) << std::fixed
                  << std::setprecision(1) << rate << "%" << " |\n";
        std::cout << "  | Cache usage    : " << std::setw(8) << cacheList.size()
                  << " / " << std::setw(3) << capacity << " |\n";
        std::cout << "  +----------------------------------+\n\n";
    }

    void resetStats() { hits = misses = total = 0; }
    int size() const  { return (int)cacheList.size(); }
};

// ─────────────────────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────────────────────
void printHelp() {
    std::cout << "\n  Commands:\n"
              << "    put <key> <value>   Insert or update a key\n"
              << "    get <key>           Fetch a key\n"
              << "    del <key>           Delete a key\n"
              << "    resize <n>          Change cache capacity\n"
              << "    cache               Show primary cache table\n"
              << "    secondary           Show secondary storage\n"
              << "    stats               Show hit/miss metrics\n"
              << "    reset               Reset stats counters\n"
              << "    help                Show this menu\n"
              << "    exit                Quit\n\n";
}

void divider() {
    std::cout << " +---------------------------------------+\n";
}

// ─────────────────────────────────────────────────────────────
//  Main — interactive terminal loop
// ─────────────────────────────────────────────────────────────
int main() {
    std::cout << "\n  +-----------------+---------------------+\n";
    std::cout << "  || LRU Cache Manager -- Interactive Mode ||\n";
    std::cout << "    +-----------------+---------------------+\n\n";

    int cap = 0;
    while (cap < 1) {
        std::cout << "  Enter cache capacity (number of slots): ";
        std::string input;
        std::getline(std::cin, input);
        try { cap = std::stoi(input); }
        catch (...) { cap = 0; }
        if (cap < 1) std::cout << "  Please enter a number >= 1.\n";
    }

    LRUCache cache(cap);
    std::cout << "\n  Cache initialized with capacity " << cap << ".\n";
    printHelp();

    std::string line;
    while (true) {
        std::cout << "  > ";
        if (!std::getline(std::cin, line)) break;

        // trim
        while (!line.empty() && (line.front() == ' ' || line.front() == '\t')) line.erase(line.begin());
        while (!line.empty() && (line.back()  == ' ' || line.back()  == '\t')) line.pop_back();
        if (line.empty()) continue;

        std::istringstream ss(line);
        std::string cmd;
        ss >> cmd;
        // lowercase command
        std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

        divider();

        if (cmd == "put") {
            std::string key, value, rest;
            ss >> key;
            std::getline(ss, rest);
            // trim leading space from rest
            while (!rest.empty() && rest.front() == ' ') rest.erase(rest.begin());
            value = rest;
            if (key.empty() || value.empty()) {
                std::cout << "  Usage: put <key> <value>\n";
            } else {
                cache.put(key, value);
                cache.displayCache();
                cache.displaySecondary();
            }

        } else if (cmd == "get") {
            std::string key; ss >> key;
            if (key.empty()) { std::cout << "  Usage: get <key>\n"; }
            else {
                std::string val = cache.get(key);
                if (val.empty()) std::cout << "  Result: not found.\n";
                else             std::cout << "  Result: \"" << val << "\"\n";
                cache.displayCache();
                cache.displaySecondary();
            }

        } else if (cmd == "del" || cmd == "delete" || cmd == "remove") {
            std::string key; ss >> key;
            if (key.empty()) { std::cout << "  Usage: del <key>\n"; }
            else {
                cache.remove(key);
                cache.displayCache();
                cache.displaySecondary();
            }

        } else if (cmd == "resize" || cmd == "setcap" || cmd == "capacity") {
            int n = 0;
            std::string ns; ss >> ns;
            try { n = std::stoi(ns); } catch (...) { n = 0; }
            if (n < 1) { std::cout << "  Usage: resize <number >= 1>\n"; }
            else {
                cache.setCapacity(n);
                cache.displayCache();
                cache.displaySecondary();
            }

        } else if (cmd == "cache" || cmd == "show" || cmd == "list") {
            cache.displayCache();

        } else if (cmd == "secondary" || cmd == "store" || cmd == "storage") {
            cache.displaySecondary();

        } else if (cmd == "stats" || cmd == "metrics") {
            cache.displayStats();

        } else if (cmd == "reset") {
            cache.resetStats();
            std::cout << "  Stats reset.\n";

        } else if (cmd == "help" || cmd == "?") {
            printHelp();

        } else if (cmd == "exit" || cmd == "quit" || cmd == "q") {
            std::cout << "  Final state:\n";
            cache.displayCache();
            cache.displaySecondary();
            cache.displayStats();
            std::cout << "  Goodbye!\n\n";
            break;

        } else {
            std::cout << "  Unknown command: \"" << cmd << "\". Type 'help' for commands.\n";
        }
    }

    return 0;
}