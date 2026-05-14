#ifndef LIVE_MONITOR_H
#define LIVE_MONITOR_H

#include <atomic>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include "sysmem.h"

// =============================================================================
// LiveMonitor -- three algorithm-relevant heap bars updated in real time.
//
//   Live   -- bytes currently outstanding.  Dances during merge, flat for
//             in-place sorts.  Final frame shows 0 (everything freed) -- the
//             Peak bar is the one that retains the visible "what happened."
//   Peak   -- high-water mark since reset.  Sticky upward.
//   Total  -- cumulative bytes ever allocated since reset.  Always grows.
//             For merge sort this hits Theta(n log n) bytes -- much more
//             than Peak, because each recursion allocates fresh L and R.
//
// All three numbers come from heap_track (operator new/delete in main.cpp).
// The OS-level metrics from /proc/self/status[m] are gone -- they were
// misleading: thread stacks, malloc pools, and sticky RSS make them look
// alarming for the FIRST sort and dead for the rest.  Heap tracking is the
// only honest per-algorithm view.
//
// The sampler thread sets heap_track::tracking_enabled = false on entry, so
// its own ifstream / string formatting allocations don't get counted as
// part of the algorithm under test.
//
// Constructor:
//   LiveMonitor(int width, int hz, uint64_t input_bytes)
//
// Live and Peak scale to input_bytes (full bar = "auxiliary equals input").
// Total scales to input_bytes * TOTAL_SCALE_FACTOR; for very large n the
// bar saturates at 100%, which itself is a true and useful signal.
// =============================================================================

namespace heap_track {
    extern std::atomic<size_t> live;
    extern std::atomic<size_t> peak;
    extern std::atomic<size_t> total;
    extern thread_local bool tracking_enabled;
}

class LiveMonitor {
public:
    static constexpr int TOTAL_SCALE_FACTOR = 16;

    explicit LiveMonitor(int width = 64, int hz = 30,
                         uint64_t input_bytes = 0)
        : width_(width),
          interval_(std::chrono::milliseconds(1000 / (hz > 0 ? hz : 1))),
          input_bytes_(input_bytes) {}

    ~LiveMonitor() { if (running_) stop(); }

    LiveMonitor(const LiveMonitor&)            = delete;
    LiveMonitor& operator=(const LiveMonitor&) = delete;

    void start() {
        if (running_.load()) return;
        std::cout << "\n\n\n" << std::flush;          // reserve 3 lines
        running_.store(true, std::memory_order_release);
        thread_ = std::thread([this] {
            // First thing: opt this thread out of heap_track.
            heap_track::tracking_enabled = false;
            sample_loop();
        });
    }

    void stop() {
        if (!running_.load()) return;
        running_.store(false, std::memory_order_release);
        if (thread_.joinable()) thread_.join();
        std::cout << "\033[3A";
        draw_frame(/*final=*/true);
    }

    void erase() {
        std::cout << "\033[3A";
        for (int i = 0; i < 3; ++i) std::cout << "\033[2K\n";
        std::cout << "\033[3A" << std::flush;
    }

private:
    uint64_t live_peak_scale_kb() const {
        if (input_bytes_ == 0) return 1;
        uint64_t s = input_bytes_ / 1024;
        return s == 0 ? 1 : s;
    }
    uint64_t total_scale_kb() const {
        return live_peak_scale_kb() * TOTAL_SCALE_FACTOR;
    }

    void draw_frame(bool /*final_frame*/ = false) {
        const uint64_t live_b  = heap_track::live.load(std::memory_order_relaxed);
        const uint64_t peak_b  = heap_track::peak.load(std::memory_order_relaxed);
        const uint64_t total_b = heap_track::total.load(std::memory_order_relaxed);

        const uint64_t lp_scale = live_peak_scale_kb();
        const uint64_t tot_scale = total_scale_kb();

        // Live -- bright yellow.  Dances with allocate/free.
        std::cout << "\033[2K";
        draw_bar("Live", live_b / 1024, lp_scale, "\033[33;1m", width_);
        std::cout << '\n';

        // Peak -- yellow.  Sticky upward.
        std::cout << "\033[2K";
        draw_bar("Peak", peak_b / 1024, lp_scale, "\033[33m", width_);
        std::cout << '\n';

        // Total -- magenta.  Cumulative, monotonically increasing.
        std::cout << "\033[2K";
        draw_bar("Tot ", total_b / 1024, tot_scale, "\033[35m", width_);
        std::cout << '\n';

        std::cout.flush();
    }

    void sample_loop() {
        while (running_.load(std::memory_order_acquire)) {
            std::cout << "\033[3A";
            draw_frame(/*final=*/false);
            std::this_thread::sleep_for(interval_);
        }
    }

    std::thread thread_;
    std::atomic<bool> running_{false};
    int width_;
    std::chrono::milliseconds interval_;
    uint64_t input_bytes_;
};

#endif // LIVE_MONITOR_H
