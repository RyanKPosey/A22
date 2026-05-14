#ifndef SYSMEM_H
#define SYSMEM_H

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>     // for sysconf(_SC_PAGESIZE)

// =============================================================================
// Memory introspection on Linux.
//
// Two layers:
//   SysMem   -- system-wide stats from /proc/meminfo  (rarely useful for
//               algorithm comparison -- the box is too big for any single
//               sort to register on the system bars).
//   ProcMem  -- THIS process's stats from /proc/self/statm.  These are the
//               numbers that actually move when our program allocates.
//
// Plus shared rendering helpers:
//   human_bytes(b)      -- format raw bytes as "20.6G", "1.23T", etc.
//   draw_bar(...)       -- render one labeled bar  Mem [|||      20.6G/1.23T]
// =============================================================================

struct SysMem {
    uint64_t mem_total_kb;
    uint64_t mem_used_kb;
    uint64_t mem_buffers_kb;
    uint64_t mem_cached_kb;
    uint64_t swap_total_kb;
    uint64_t swap_used_kb;
};

inline SysMem read_sysmem() {
    SysMem s{};
    uint64_t available = 0, swap_free = 0;

    std::ifstream f("/proc/meminfo");
    std::string key, unit;
    uint64_t val;
    while (f >> key >> val >> unit) {
        if      (key == "MemTotal:")     s.mem_total_kb   = val;
        else if (key == "MemAvailable:") available        = val;
        else if (key == "Buffers:")      s.mem_buffers_kb = val;
        else if (key == "Cached:")       s.mem_cached_kb  = val;
        else if (key == "SwapTotal:")    s.swap_total_kb  = val;
        else if (key == "SwapFree:")     swap_free        = val;
    }
    s.mem_used_kb  = s.mem_total_kb  > available ? s.mem_total_kb  - available : 0;
    s.swap_used_kb = s.swap_total_kb > swap_free ? s.swap_total_kb - swap_free : 0;
    return s;
}

// -----------------------------------------------------------------------------
// Per-process memory (this process only).
//
// Read from /proc/self/statm, which is one cheap line of seven numbers:
//   size resident shared text lib data dt           (all in pages)
//
//   resident_bytes -- RSS, the bytes the kernel currently has in physical RAM
//                     for us.  Grows when allocations are touched; sticky on
//                     free (allocator pools usually retain freed pages).
//   data_bytes     -- size of the data + stack region.  Closely tracks heap
//                     growth via brk/mmap.  Also typically sticky on free.
//   size_bytes     -- total virtual address space we've mapped (includes
//                     shared libraries, mmap'd files, etc).
// -----------------------------------------------------------------------------
struct ProcMem {
    uint64_t resident_bytes;    // RSS   -- physical memory backing this process
    uint64_t data_bytes;        // VmData -- data segment + stack
    uint64_t size_bytes;        // VmSize -- total virtual memory
};

inline ProcMem read_procmem() {
    ProcMem p{};
    static const long page_size = sysconf(_SC_PAGESIZE);

    std::ifstream f("/proc/self/statm");
    if (!f) return p;

    long size_pg     = 0;
    long resident_pg = 0;
    long shared_pg   = 0;
    long text_pg     = 0;
    long lib_pg      = 0;
    long data_pg     = 0;
    long dt_pg       = 0;
    f >> size_pg >> resident_pg >> shared_pg >> text_pg >> lib_pg >> data_pg >> dt_pg;

    p.resident_bytes = static_cast<uint64_t>(resident_pg) * page_size;
    p.data_bytes     = static_cast<uint64_t>(data_pg)     * page_size;
    p.size_bytes     = static_cast<uint64_t>(size_pg)     * page_size;
    return p;
}

inline std::string human_bytes(uint64_t bytes) {
    static const char* units[] = {"B", "K", "M", "G", "T", "P"};
    int u = 0;
    double v = static_cast<double>(bytes);
    while (v >= 1024.0 && u < 5) { v /= 1024.0; ++u; }

    char buf[32];
    if      (v >= 100) std::snprintf(buf, sizeof(buf), "%.0f%s", v, units[u]);
    else if (v >= 10)  std::snprintf(buf, sizeof(buf), "%.1f%s", v, units[u]);
    else               std::snprintf(buf, sizeof(buf), "%.2f%s", v, units[u]);
    return std::string(buf);
}

// Render one bar on the current line (no trailing newline).
//   used_kb      -- numerator, in kilobytes
//   total_kb     -- denominator, in kilobytes
//   fill_color   -- ANSI sequence for the pipes (e.g. "\033[32m" green)
//   width        -- total characters between the brackets
inline void draw_bar(const char* label,
                     uint64_t used_kb, uint64_t total_kb,
                     const char* fill_color, int width = 60)
{
    double ratio = (total_kb == 0) ? 0.0
                                   : static_cast<double>(used_kb) / total_kb;
    if (ratio > 1.0) ratio = 1.0;
    int filled = static_cast<int>(ratio * width);

    std::string text = human_bytes(used_kb  * 1024) + "/" +
                       human_bytes(total_kb * 1024);
    int text_len = static_cast<int>(text.length());

    if (filled > width - text_len) filled = width - text_len;
    if (filled < 0) filled = 0;
    int gap = width - filled - text_len;
    if (gap < 0) gap = 0;

    std::cout << "\033[36;1m" << label << "\033[0m[";
    std::cout << fill_color;
    for (int i = 0; i < filled; ++i) std::cout << '|';
    std::cout << "\033[0m";
    for (int i = 0; i < gap;    ++i) std::cout << ' ';
    std::cout << "\033[36m" << text << "\033[0m";
    std::cout << ']';
}

#endif // SYSMEM_H
