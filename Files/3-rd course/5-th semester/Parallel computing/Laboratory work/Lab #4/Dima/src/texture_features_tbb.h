// texture_features_tbb.h
// Задание 4.3. Расчёт карт текстурных признаков — параллельная версия на
// Intel oneTBB.
//
// ВНИМАНИЕ: написано по документированному API oneTBB, не скомпилировано
// в данной песочнице (нет доступа к libtbb-dev/интернету) — собирать и
// проверять нужно в среде с Intel oneAPI Base Toolkit.
#pragma once

#include <tbb/tbb.h>
#include <tbb/global_control.h>
#include <tbb/blocked_range.h>
#include <chrono>
#include <vector>
#include "texture_common.h"

inline double NowSecondsTF() {
    using namespace std::chrono;
    return duration<double>(high_resolution_clock::now().time_since_epoch()).count();
}

// --- Формирование карт признаков: parallel_for по строкам (внешний цикл),
// локальная гистограмма на вызов тела (поток обрабатывает диапазон строк
// последовательно — гонки данных нет, пишем в непересекающиеся idx). ---
inline double ComputeTextureMapsTbb(const uint8_t* bimage, int width, int height,
                                     int RH, int RW, TextureMaps& out, int nth) {
    out.alloc(width, height);
    tbb::global_control gc(tbb::global_control::max_allowed_parallelism, (size_t)nth);
    double* M2 = out.M2.data(); double* R = out.R.data();
    double* U = out.U.data();   double* E = out.E.data();

    double t0 = NowSecondsTF();
    tbb::parallel_for(tbb::blocked_range<int>(0, height),
        [&](const tbb::blocked_range<int>& rows) {
            double hist[256];
            for (int y = rows.begin(); y != rows.end(); ++y) {
                for (int x = 0; x < width; x++) {
                    TextureVals v = ComputeTextureAtPixel(bimage, width, height, x, y, RH, RW, hist);
                    size_t idx = (size_t)y * width + x;
                    M2[idx] = v.m2; R[idx] = v.r; U[idx] = v.u; E[idx] = v.e;
                }
            }
        });
    return NowSecondsTF() - t0;
}

// --- Шаг 1 (oneTBB): редукция min/max — класс (Body со split/join),
// как рекомендовано заданием ("рекомендуется использовать класс для
// редукции"). Один проход сразу по всем 4 картам. ---
class MinMaxAllBody {
public:
    const double *M2_, *R_, *U_, *E_;
    MinMaxAll mm;
    MinMaxAllBody(const double* m2, const double* r, const double* u, const double* e)
        : M2_(m2), R_(r), U_(u), E_(e) {}
    MinMaxAllBody(MinMaxAllBody& s, tbb::split) : M2_(s.M2_), R_(s.R_), U_(s.U_), E_(s.E_) {}
    void operator()(const tbb::blocked_range<size_t>& range) {
        MinMaxAll l = mm;
        for (size_t k = range.begin(); k != range.end(); ++k) {
            if (M2_[k] < l.minM2) l.minM2 = M2_[k]; if (M2_[k] > l.maxM2) l.maxM2 = M2_[k];
            if (R_[k]  < l.minR)  l.minR  = R_[k];  if (R_[k]  > l.maxR)  l.maxR  = R_[k];
            if (U_[k]  < l.minU)  l.minU  = U_[k];  if (U_[k]  > l.maxU)  l.maxU  = U_[k];
            if (E_[k]  < l.minE)  l.minE  = E_[k];  if (E_[k]  > l.maxE)  l.maxE  = E_[k];
        }
        mm = l;
    }
    void join(const MinMaxAllBody& rhs) {
        mm.minM2 = std::min(mm.minM2, rhs.mm.minM2); mm.maxM2 = std::max(mm.maxM2, rhs.mm.maxM2);
        mm.minR  = std::min(mm.minR,  rhs.mm.minR);  mm.maxR  = std::max(mm.maxR,  rhs.mm.maxR);
        mm.minU  = std::min(mm.minU,  rhs.mm.minU);  mm.maxU  = std::max(mm.maxU,  rhs.mm.maxU);
        mm.minE  = std::min(mm.minE,  rhs.mm.minE);  mm.maxE  = std::max(mm.maxE,  rhs.mm.maxE);
    }
};

inline double ComputeMinMaxTbb(const TextureMaps& t, int nth, MinMaxAll& mm) {
    tbb::global_control gc(tbb::global_control::max_allowed_parallelism, (size_t)nth);
    size_t n = (size_t)t.width * t.height;

    double t0 = NowSecondsTF();
    MinMaxAllBody body(t.M2.data(), t.R.data(), t.U.data(), t.E.data());
    tbb::parallel_reduce(tbb::blocked_range<size_t>(0, n), body);
    mm = body.mm;
    return NowSecondsTF() - t0;
}

// --- Шаг 3 (oneTBB): построение карты по порогам, parallel_for по строкам. ---
inline double BuildMapTbb(const std::vector<double>& feature, int width, int height,
                           double T1, double T2, RGBTRIPLE** outMap, int nth) {
    tbb::global_control gc(tbb::global_control::max_allowed_parallelism, (size_t)nth);
    const double* f = feature.data();

    double t0 = NowSecondsTF();
    tbb::parallel_for(tbb::blocked_range<int>(0, height),
        [&](const tbb::blocked_range<int>& rows) {
            for (int y = rows.begin(); y != rows.end(); ++y)
                for (int x = 0; x < width; x++)
                    outMap[y][x] = ColorByThreshold(f[(size_t)y * width + x], T1, T2);
        });
    return NowSecondsTF() - t0;
}
