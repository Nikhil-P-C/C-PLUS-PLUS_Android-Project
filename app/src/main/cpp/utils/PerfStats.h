//
// Frame / section timing for the debug overlay (DebugState).
//
// Engine::run() brackets every phase of the loop with PerfStats::Scope and calls beginFrame() / endWork();
// DebugState reads summarize() and recent() to draw the numbers and the frame-time graph.
//
// All times are milliseconds from SDL_GetPerformanceCounter. Single-threaded (the render thread), no locks.
//
//   frame   = real time between two loop iterations (includes the sleep / vsync wait): this is what the player feels
//   work    = time from the top of the loop to just before the frame-limiter sleep: how much of the budget you burn
//   events / update / render (world, overlays) / present / sleep = where that time goes (smoothed over ~10 frames)
//
// NOTE: render times are the CPU cost of submitting draw calls. With a GPU renderer the GPU's own time tends to show
// up in `present` (it blocks until the swap chain has a free image), so a big `present` = GPU bound / vsync wait.
//
#pragma once
#include <SDL3/SDL_timer.h>
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <vector>
#if defined(__linux__) || defined(__ANDROID__)
#include <unistd.h>
#endif

// plain counters other headers can bump without including the class (see Cull.h)
namespace PerfCounters {
    inline uint32_t cullTested   = 0;   // objects asked "are you on screen?" this frame
    inline uint32_t cullRejected = 0;   // ...and skipped because they were not
}

class PerfStats {
public:
    static constexpr int HISTORY = 600;          // ~10 s at 60 fps
    static constexpr float BUDGET_MS = 1000.0f / 60.0f;
    static constexpr float HITCH_MS  = 25.0f;    // a frame this long is a visible stutter

    enum Section { EVENTS = 0, UPDATE, RENDER_STATES, RENDER_OVERLAYS, PRESENT, SLEEP, SECTION_COUNT };

    struct Summary {
        int   samples = 0;
        float avgMs = 0, minMs = 0, maxMs = 0, p99Ms = 0;
        float fps = 0, lowFps = 0;               // lowFps = the "1% low": 1000 / p99
        int   hitches = 0;                       // frames over HITCH_MS in the history window
    };

    static PerfStats& get() { static PerfStats p; return p; }

    // top of Engine::run()'s loop
    void beginFrame() {
        const uint64_t now = SDL_GetPerformanceCounter();
        if (m_frameStart) {
            m_lastFrameMs = ticksToMs(now - m_frameStart);
            m_frameRing[m_head] = (float)m_lastFrameMs;
            m_head = (m_head + 1) % HISTORY;
            m_count = std::min(m_count + 1, HISTORY);
            for (int s = 0; s < SECTION_COUNT; ++s) {
                m_avg[s] += (m_cur[s] - m_avg[s]) * 0.1f;
                m_cur[s] = 0.0f;
            }
            m_avgWork += (m_work - m_avgWork) * 0.1f;
            m_cullTested = PerfCounters::cullTested;
            m_cullRejected = PerfCounters::cullRejected;
        }
        PerfCounters::cullTested = PerfCounters::cullRejected = 0;
        m_frameStart = now;
    }

    // just before the frame limiter's sleep
    void endWork() { m_work = (float)ticksToMs(SDL_GetPerformanceCounter() - m_frameStart); }

    struct Scope {
        Section s; uint64_t t0;
        explicit Scope(Section sec) : s(sec), t0(SDL_GetPerformanceCounter()) {}
        ~Scope() { PerfStats::get().m_cur[s] += (float)PerfStats::ticksToMs(SDL_GetPerformanceCounter() - t0); }
    };

    // ---- readers (DebugState) ----
    float lastFrameMs() const { return (float)m_lastFrameMs; }
    float workMs() const      { return m_avgWork; }
    float section(Section s) const { return m_avg[s]; }
    uint32_t cullTested() const   { return m_cullTested; }     // previous frame's totals
    uint32_t cullRejected() const { return m_cullRejected; }

    // the last `n` frame times, oldest first (for the graph). Returns how many were written.
    int recent(float* out, int n) const {
        n = std::min(n, m_count);
        for (int i = 0; i < n; ++i)
            out[i] = m_frameRing[(m_head - n + i + HISTORY * 2) % HISTORY];
        return n;
    }

    Summary summarize() const {
        Summary r;
        r.samples = m_count;
        if (m_count == 0) return r;
        std::vector<float> v(m_frameRing, m_frameRing + m_count);   // ring order does not matter for these stats
        double sum = 0.0;
        for (float f : v) { sum += f; if (f > HITCH_MS) ++r.hitches; }
        std::sort(v.begin(), v.end());
        r.avgMs = (float)(sum / v.size());
        r.minMs = v.front();
        r.maxMs = v.back();
        r.p99Ms = v[std::min((size_t)(v.size() * 0.99), v.size() - 1)];
        r.fps    = r.avgMs > 0.0f ? 1000.0f / r.avgMs : 0.0f;
        r.lowFps = r.p99Ms > 0.0f ? 1000.0f / r.p99Ms : 0.0f;
        return r;
    }

    // resident memory of this process in MB (0 where /proc is not available)
    static float residentMB() {
#if defined(__linux__) || defined(__ANDROID__)
        if (FILE* f = std::fopen("/proc/self/statm", "r")) {
            long size = 0, resident = 0;
            const int got = std::fscanf(f, "%ld %ld", &size, &resident);
            std::fclose(f);
            if (got == 2) return (float)((double)resident * (double)sysconf(_SC_PAGESIZE) / (1024.0 * 1024.0));
        }
#endif
        return 0.0f;
    }

    static double ticksToMs(uint64_t ticks) {
        static const double inv = 1000.0 / (double)SDL_GetPerformanceFrequency();
        return (double)ticks * inv;
    }

private:
    PerfStats() = default;
    uint64_t m_frameStart = 0;
    double   m_lastFrameMs = 0.0;
    float    m_work = 0.0f, m_avgWork = 0.0f;
    float    m_cur[SECTION_COUNT]{}, m_avg[SECTION_COUNT]{};
    float    m_frameRing[HISTORY]{};
    int      m_head = 0, m_count = 0;
    uint32_t m_cullTested = 0, m_cullRejected = 0;
};
