#pragma once

#include <cmath>
#include <cstring>

// Сортировка пузырьком массива замеров времени (для расчёта медианы)
inline void bubbleSort(double* mas, int cnt) {
    for (int i = 0; i < cnt - 1; i++) {
        bool swapped = false;
        for (int j = 0; j < cnt - 1 - i; j++) {
            if (mas[j] > mas[j + 1]) {
                double tmp = mas[j];
                mas[j] = mas[j + 1];
                mas[j + 1] = tmp;
                swapped = true;
            }
        }
        if (!swapped) {
            break;
        }
    }
}

// Вариант 1: расчёт среднего времени по доверительному интервалу относительно среднего (avg +/- SD)
inline double AvgTrustedIntervalAVG(double*& times, int cnt) {
    if (cnt <= 0) {
        return 0.0;
    }
    if (cnt == 1) {
        return times[0];
    }

    // 1. Вычисление среднего арифметического исходной выборки
    double sum = 0.0;
    for (int i = 0; i < cnt; i++) {
        sum += times[i];
    }
    double avg = sum / cnt;

    // 2. Вычисление выборочного стандартного отклонения SD = sqrt(sum((Xi - avg)^2) / (N - 1))
    double sqDiffSum = 0.0;
    for (int i = 0; i < cnt; i++) {
        double diff = times[i] - avg;
        sqDiffSum += diff * diff;
    }
    double sd = std::sqrt(sqDiffSum / static_cast<double>(cnt - 1));

    // 3. Фильтрация выбросов по условию avg - sd <= Xi <= avg + sd
    double newSum = 0.0;
    int newCnt = 0;
    for (int i = 0; i < cnt; i++) {
        if (times[i] >= avg - sd && times[i] <= avg + sd) {
            newSum += times[i];
            newCnt++;
        }
    }

    // Защита от деления на ноль
    if (newCnt == 0) {
        newCnt = 1;
        return avg;
    }
    return newSum / newCnt;
}

// Вариант 2 (предпочтительный): расчёт среднего времени по доверительному интервалу относительно медианы (med +/- SD)
inline double AvgTrustedIntervalMed(double*& times, int cnt) {
    if (cnt <= 0) {
        return 0.0;
    }
    if (cnt == 1) {
        return times[0];
    }

    // 1. Вычисление среднего арифметического avg для оценки стандартного отклонения
    double sum = 0.0;
    for (int i = 0; i < cnt; i++) {
        sum += times[i];
    }
    double avg = sum / cnt;

    // 2. Вычисление стандартного отклонения SD по формуле SD = sqrt(1/(N-1) * sum((Xi - avg)^2))
    double sqDiffSum = 0.0;
    for (int i = 0; i < cnt; i++) {
        double diff = times[i] - avg;
        sqDiffSum += diff * diff;
    }
    double sd = std::sqrt(sqDiffSum / static_cast<double>(cnt - 1));

    // 3. Сортировка копии выборки методом пузырька для вычисления медианы без потери порядка замеров
    double* sorted = new double[cnt];
    std::memcpy(sorted, times, static_cast<size_t>(cnt) * sizeof(double));
    bubbleSort(sorted, cnt);

    double med = 0.0;
    if (cnt % 2 == 0) {
        med = 0.5 * (sorted[cnt / 2 - 1] + sorted[cnt / 2]);
    } else {
        med = sorted[cnt / 2];
    }
    delete[] sorted;

    // 4. Фильтрация выбросов по условию med - sd <= Xi <= med + sd
    double newSum = 0.0;
    int newCnt = 0;
    for (int i = 0; i < cnt; i++) {
        if (times[i] >= med - sd && times[i] <= med + sd) {
            newSum += times[i];
            newCnt++;
        }
    }

    // Защита от деления на ноль
    if (newCnt == 0) {
        newCnt = 1;
        return med;
    }
    return newSum / newCnt;
}

// Подробная статистика серии замеров (для протоколирования и построения графиков доверительного интервала)
struct TrustedStats {
    int totalRuns = 0;
    int keptRuns = 0;
    int discardedRuns = 0;
    double rawAvg = 0.0;
    double median = 0.0;
    double sd = 0.0;
    double trustedAvgMed = 0.0;
    double trustedAvgMean = 0.0;
};

inline TrustedStats computeTrustedStats(double*& times, int cnt) {
    TrustedStats st;
    st.totalRuns = cnt;
    if (cnt <= 0) {
        return st;
    }

    double sum = 0.0;
    for (int i = 0; i < cnt; i++) {
        sum += times[i];
    }
    st.rawAvg = sum / cnt;

    if (cnt > 1) {
        double sqDiffSum = 0.0;
        for (int i = 0; i < cnt; i++) {
            double diff = times[i] - st.rawAvg;
            sqDiffSum += diff * diff;
        }
        st.sd = std::sqrt(sqDiffSum / static_cast<double>(cnt - 1));
    }

    double* sorted = new double[cnt];
    std::memcpy(sorted, times, static_cast<size_t>(cnt) * sizeof(double));
    bubbleSort(sorted, cnt);
    st.median = (cnt % 2 == 0) ? 0.5 * (sorted[cnt / 2 - 1] + sorted[cnt / 2]) : sorted[cnt / 2];
    delete[] sorted;

    int kept = 0;
    for (int i = 0; i < cnt; i++) {
        if (times[i] >= st.median - st.sd && times[i] <= st.median + st.sd) {
            kept++;
        }
    }
    st.keptRuns = kept;
    st.discardedRuns = cnt - kept;
    st.trustedAvgMed = AvgTrustedIntervalMed(times, cnt);
    st.trustedAvgMean = AvgTrustedIntervalAVG(times, cnt);
    return st;
}
