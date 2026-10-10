//
// Created by LENOVO on 11-10-2026.
//

//
// HearingFx - cinematic "damage audio" DSP for the final mix (runs on the audio thread, header-only).
//
//   muffle    0..1 : steep low-pass (world goes dull / out of focus), a little quieter, plus a small dark room
//                    reverb that grows with it. NO pitch or speed change (varispeed sounds like a tape/arcade effect).
//   heartbeat 0..1 : lub-dub built from a pitch-dropping body + a soft low-passed noise transient, soft-clipped
//   boom           : one-shot sub impact (same construction), triggered with playBoom()
//
// Game thread calls setMuffle / setHeartbeat / playBoom (lock-free); the audio thread calls process().
//

#pragma once
#include <SDL3/SDL.h>
#include <atomic>
#include <vector>
#include <cmath>
#include <cstdint>
#include <algorithm>

#ifndef HFX_RV_SEND
#define HFX_RV_SEND 0.035f          // reverb send level (reverb loudness); tuned offline
#endif

class HearingFx {
public:
    void setMuffle(float v)    { m_muffleTarget.store(clamp01(v), std::memory_order_relaxed); }
    void setHeartbeat(float v) { m_heartTarget.store(clamp01(v), std::memory_order_relaxed); }
    void playBoom(float s)     { m_boomTrigger.store(clamp01(s), std::memory_order_relaxed); }

    // pcm = interleaved float32, `samples` = total float count (NOT frames)
    void process(float* pcm, int samples, int channels, int freq) {
        if (channels < 1 || freq < 8000 || samples <= 0) return;
        const float muffleT = m_muffleTarget.load(std::memory_order_relaxed);
        const float heartT  = m_heartTarget.load(std::memory_order_relaxed);
        const float boomTrg = m_boomTrigger.exchange(0.0f, std::memory_order_relaxed);
        if (boomTrg > 0.0f) { m_boomAmp = boomTrg; m_boomTime = 0.0f; m_boomActive = true; }

        const bool idle = muffleT < 0.0005f && m_muffleCur < 0.0005f &&
                          heartT  < 0.0005f && m_heartCur  < 0.0005f && !m_boomActive;
        if (idle) {
            if (m_dirty) reset();
            return;
        }
        m_dirty = true;
        ensureReverb(freq);
        if (heartT > 0.0005f && !m_heartWasOn) m_hbTime = 0.0f;       // turning on: begin right on beat 1
        m_heartWasOn = heartT > 0.0005f || m_heartCur > 0.0005f;

        const int   ch      = SDL_min(channels, 8);
        const int   frames  = samples / channels;
        const float fs      = (float)freq;
        const float dt      = 1.0f / fs;
        const float smooth  = 1.0f - std::exp(-dt / 0.020f);          // ~20 ms parameter smoothing
        const float PERIOD  = 0.9f;                                   // heartbeat cycle (~67 bpm)
        const float TWO_PI  = 6.2831853f;

        for (int f = 0; f < frames; f++) {
            m_muffleCur += (muffleT - m_muffleCur) * smooth;
            m_heartCur  += (heartT  - m_heartCur)  * smooth;

            if (--m_coefCountdown <= 0) {
                m_coefCountdown = 32;
                // 20 kHz (clear) -> 700 Hz (heavily muffled), logarithmic
                float fc = 20000.0f * std::pow(700.0f / 20000.0f, m_muffleCur);
                if (fc > 0.45f * fs) fc = 0.45f * fs;
                m_lpCoef = 1.0f - std::exp(-TWO_PI * fc / fs);
            }
            const float wet  = SDL_min(1.0f, m_muffleCur);               // 1.0 = pure low-pass (no phase-smeared dry leak)
            const float duck = 1.0f - 0.28f * m_muffleCur;

            // ---- synthesised thumps (heartbeat + boom) --------------------------------------------------
            float thump = 0.0f;
            if (m_heartCur > 0.0005f) {
                m_hbTime += dt;
                if (m_hbTime >= PERIOD) m_hbTime -= PERIOD;
                thump += heartBeat(m_hbTime, 48.0f, 18.0f, 0.60f) * m_heartCur;            // lub
                thump += heartBeat(m_hbTime - 0.20f, 44.0f, 14.0f, 0.42f) * m_heartCur;    // dub
            }
            if (m_boomActive) {
                const float t = m_boomTime;
                const float ph = TWO_PI * (38.0f * t + 82.0f * 0.12f * (1.0f - std::exp(-t / 0.12f)));
                const float env = (1.0f - std::exp(-t / 0.003f)) * std::exp(-t / 0.30f);
                float b = (std::sin(ph) + 0.3f * std::sin(2.0f * ph)) * env * 0.75f;
                b += m_nzBoom * std::exp(-t / 0.05f) * 0.55f;
                m_nzBoom += 0.06f * (noise() - m_nzBoom);                                    // dark noise "thud"
                thump += b * m_boomAmp;
                m_boomTime += dt;
                if (m_boomTime > 1.6f) m_boomActive = false;
            }
            thump = std::tanh(thump * 1.4f) / 1.4f;                                         // soft clip: warm, not square

            // ---- muffle low-pass (3 cascaded one-poles = 18 dB/oct) -----------------------------------------
            float* frame = pcm + (size_t)f * channels;
            float lpMono = 0.0f;
            for (int c = 0; c < ch; c++) {
                const float x = frame[c];
                m_lp[0][c] += m_lpCoef * (x - m_lp[0][c]);
                m_lp[1][c] += m_lpCoef * (m_lp[0][c] - m_lp[1][c]);
                m_lp[2][c] += m_lpCoef * (m_lp[1][c] - m_lp[2][c]);
                frame[c] = (x + (m_lp[2][c] - x) * wet) * duck;
                if (c < 2) lpMono += m_lp[2][c];
            }
            lpMono *= (ch >= 2 ? 0.5f : 1.0f);

            // ---- small dark room (stereo Freeverb-style), fed from the LOW-PASSED signal + the thumps --------
            const float send = (lpMono * 0.55f + thump * 0.30f) * HFX_RV_SEND;
            float rev[2] = {0.0f, 0.0f};
            for (int s = 0; s < 2; s++) {
                float acc = 0.0f;
                for (int k = 0; k < 4; k++) acc += comb(m_comb[s][k], send, 0.86f, 0.55f);
                acc = allpass(m_ap[s][0], acc);
                acc = allpass(m_ap[s][1], acc);
                rev[s] = acc;
            }
            const float rvWet = 1.6f * m_muffleCur;
            for (int c = 0; c < ch; c++) {
                frame[c] += thump + rev[c & 1] * rvWet;
            }
        }
    }

private:
    struct Line { std::vector<float> b; int i = 0; float lp = 0.0f; };

    static float clamp01(float v) { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); }

    float noise() {                                   // xorshift32 -> [-1, 1]
        m_rng ^= m_rng << 13; m_rng ^= m_rng >> 17; m_rng ^= m_rng << 5;
        return (float)(m_rng & 0xFFFFFF) / (float)0x7FFFFF - 1.0f;
    }

    // one heartbeat "thump": sine body that falls in pitch + its 2nd harmonic (so phone speakers carry it)
    // + a soft low-passed noise transient. t = seconds since this thump started (negative = not started).
    float heartBeat(float t, float f0, float df, float amp) {
        if (t < 0.0f || t > 0.5f) return 0.0f;
        const float tau = 0.05f;
        const float ph = 6.2831853f * (f0 * t + df * tau * (1.0f - std::exp(-t / tau)));
        const float env = (1.0f - std::exp(-t / 0.004f)) * std::exp(-t / 0.085f);
        m_nzHeart += 0.10f * (noise() - m_nzHeart);
        return ((std::sin(ph) + 0.4f * std::sin(2.0f * ph)) * env + m_nzHeart * std::exp(-t / 0.02f) * 0.7f) * amp;
    }

    float comb(Line& L, float in, float fb, float damp) {
        const float out = L.b[L.i];
        L.lp = out * (1.0f - damp) + L.lp * damp;
        L.b[L.i] = in + L.lp * fb;
        if (++L.i >= (int)L.b.size()) L.i = 0;
        return out;
    }
    float allpass(Line& L, float in) {
        const float bo = L.b[L.i];
        const float out = -in + bo;
        L.b[L.i] = in + bo * 0.5f;
        if (++L.i >= (int)L.b.size()) L.i = 0;
        return out;
    }

    void ensureReverb(int freq) {
        if (m_rvFs == freq) return;
        m_rvFs = freq;
        static const int combBase[4] = {1116, 1188, 1277, 1356};
        static const int apBase[2]   = {556, 441};
        const float sc = (float)freq / 44100.0f;
        for (int s = 0; s < 2; s++) {
            const int spread = s * 23;
            for (int k = 0; k < 4; k++) { m_comb[s][k].b.assign((size_t)((combBase[k] + spread) * sc) + 1, 0.0f); m_comb[s][k].i = 0; m_comb[s][k].lp = 0.0f; }
            for (int k = 0; k < 2; k++) { m_ap[s][k].b.assign((size_t)((apBase[k] + spread) * sc) + 1, 0.0f);   m_ap[s][k].i = 0; }
        }
    }

    void reset() {
        m_muffleCur = m_heartCur = 0.0f;
        m_heartWasOn = false; m_boomActive = false; m_dirty = false;
        for (auto& st : m_lp) for (float& v : st) v = 0.0f;
        for (auto& s : m_comb) for (auto& L : s) { std::fill(L.b.begin(), L.b.end(), 0.0f); L.lp = 0.0f; }
        for (auto& s : m_ap)   for (auto& L : s)   std::fill(L.b.begin(), L.b.end(), 0.0f);
    }

    std::atomic<float> m_muffleTarget{0.0f}, m_heartTarget{0.0f}, m_boomTrigger{0.0f};

    // audio-thread state
    float m_muffleCur = 0.0f, m_heartCur = 0.0f, m_lpCoef = 1.0f;
    float m_lp[3][8] = {};
    int   m_coefCountdown = 0;
    bool  m_heartWasOn = false, m_boomActive = false, m_dirty = false;
    float m_hbTime = 0.0f, m_boomTime = 0.0f, m_boomAmp = 0.0f;
    float m_nzHeart = 0.0f, m_nzBoom = 0.0f;
    uint32_t m_rng = 0x1234ABCDu;
    Line m_comb[2][4];
    Line m_ap[2][2];
    int  m_rvFs = 0;
};
