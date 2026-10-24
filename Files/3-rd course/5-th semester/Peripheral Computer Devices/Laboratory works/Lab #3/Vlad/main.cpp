// Лабораторная работа: «Моделирование взаимодействия с периферийными устройствами на C++».
//
// Что моделируется:
//   * "АЦП" - устройство, которое с заданной частотой выдает 12-битные отсчеты;
//   * регистры устройства (STATUS, DATA, CONTROL, TIMESTAMP) с ключевым словом volatile;
//   * кольцевой буфер (ring buffer) для обмена данными между обработчиком и приложением;
//   * два способа работы: опрос (polling) и "прерывания" (callback);
//   * сравнение: потери данных, время отклика, загрузка процессора.
//
// Сборка:
//   MSVC (Developer Command Prompt):  cl /EHsc /O2 /std:c++17 main.cpp
//   MinGW (Windows):                  g++ -std=c++17 -O2 main.cpp -o lab3 -lwinmm
//   Linux / macOS:                    g++ -std=c++17 -O2 main.cpp -o lab3 -pthread
//
// Запуск:  lab3 [частота_отсчетов_Гц] [секунд_на_сценарий]    (по умолчанию 1000 Гц, 5 с)

#include <atomic>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#else
#include <time.h>
#endif

using Clock = std::chrono::steady_clock;

static constexpr double kPi = 3.14159265358979323846;

static int64_t nowNs() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
               Clock::now().time_since_epoch()).count();
}

// Процессорное время ТЕКУЩЕГО потока в секундах (нужно для оценки загрузки ЦП)
static double threadCpuSeconds() {
#ifdef _WIN32
    FILETIME c, e, k, u;
    GetThreadTimes(GetCurrentThread(), &c, &e, &k, &u);
    auto toSec = [](const FILETIME& f) {
        ULARGE_INTEGER x;
        x.LowPart = f.dwLowDateTime;
        x.HighPart = f.dwHighDateTime;
        return static_cast<double>(x.QuadPart) * 1e-7;   // единицы по 100 нс
    };
    return toSec(k) + toSec(u);
#else
    timespec ts;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts);
    return static_cast<double>(ts.tv_sec) + static_cast<double>(ts.tv_nsec) * 1e-9;
#endif
}

// ---------------------------------------------------------------------------
// Отсчет АЦП: значение + момент, когда устройство его сформировало
// ---------------------------------------------------------------------------
struct Sample {
    uint32_t value = 0;
    int64_t tsNs = 0;
};

// ---------------------------------------------------------------------------
// Кольцевой буфер "один производитель - один потребитель" без блокировок.
// head_ - сколько элементов записано, tail_ - сколько прочитано (счетчики растут
// монотонно, индекс в массиве = счетчик & (N-1)). Размер N - степень двойки.
// ---------------------------------------------------------------------------
template <typename T, size_t N>
class RingBuffer {
    static_assert(N > 1 && (N & (N - 1)) == 0, "N must be a power of two");
    T buf_[N];
    std::atomic<size_t> head_{0};   // индекс записи (меняет только производитель)
    std::atomic<size_t> tail_{0};   // индекс чтения (меняет только потребитель)
public:
    bool push(const T& v) {
        size_t h = head_.load(std::memory_order_relaxed);
        size_t t = tail_.load(std::memory_order_acquire);
        if (h - t == N) return false;               // буфер полон
        buf_[h & (N - 1)] = v;
        head_.store(h + 1, std::memory_order_release);
        return true;
    }
    bool pop(T& out) {
        size_t t = tail_.load(std::memory_order_relaxed);
        size_t h = head_.load(std::memory_order_acquire);
        if (h == t) return false;                   // буфер пуст
        out = buf_[t & (N - 1)];
        tail_.store(t + 1, std::memory_order_release);
        return true;
    }
    bool empty() const {
        return head_.load(std::memory_order_acquire) == tail_.load(std::memory_order_acquire);
    }
};

// ---------------------------------------------------------------------------
// Регистры устройства.
// volatile запрещает компилятору кэшировать значение в регистре процессора и
// выкидывать "лишние" чтения: на реальном железе регистр меняет аппаратура,
// и каждое обращение в коде должно превратиться в настоящее обращение к памяти.
// std::atomic добавлен потому, что здесь "аппаратура" - это соседний поток:
// volatile сам по себе не защищает от гонок данных между потоками.
// ---------------------------------------------------------------------------
static constexpr uint32_t STATUS_READY   = 1u << 0;   // новый отсчет готов
static constexpr uint32_t STATUS_OVERRUN = 1u << 1;   // отсчет перезаписан до прочтения
static constexpr uint32_t CTRL_ENABLE    = 1u << 0;   // устройство включено
static constexpr uint32_t CTRL_IRQ_EN    = 1u << 1;   // разрешить прерывание

struct Registers {
    volatile std::atomic<uint32_t> STATUS{0};
    volatile std::atomic<uint32_t> DATA{0};
    volatile std::atomic<uint32_t> CONTROL{0};
    volatile std::atomic<int64_t>  TIMESTAMP{0};
};

// ---------------------------------------------------------------------------
// Модель "АЦП": в отдельном потоке (это наша "аппаратура") с заданной частотой
// кладет новый отсчет в регистр DATA, взводит флаг READY и, если прерывания
// разрешены, вызывает зарегистрированный обработчик (callback = "прерывание").
// ---------------------------------------------------------------------------
class AdcDevice {
public:
    Registers regs;

    explicit AdcDevice(double rateHz) : rateHz_(rateHz) {}
    ~AdcDevice() { stop(); }

    void attachIrq(std::function<void()> handler) { irq_ = std::move(handler); }

    void start() {
        running_.store(true);
        thread_ = std::thread(&AdcDevice::run, this);
    }
    void stop() {
        running_.store(false);
        if (thread_.joinable()) thread_.join();
    }

    uint32_t status() const { return regs.STATUS.load(); }

    // Чтение регистра данных. Как и в настоящем контроллере, чтение DATA
    // атомарно сбрасывает флаги READY/OVERRUN (за это отвечает mutex "шины").
    bool readData(Sample& out) {
        std::lock_guard<std::mutex> lk(bus_);
        if (!(regs.STATUS.load() & STATUS_READY)) return false;
        out.value = regs.DATA.load();
        out.tsNs = regs.TIMESTAMP.load();
        regs.STATUS.fetch_and(~(STATUS_READY | STATUS_OVERRUN));
        return true;
    }

    uint64_t generated() const { return generated_.load(); }
    uint64_t overwritten() const { return overwritten_.load(); }

private:
    void tick() {
        {
            std::lock_guard<std::mutex> lk(bus_);
            if (regs.STATUS.load() & STATUS_READY) {   // прошлый отсчет не успели прочитать
                regs.STATUS.fetch_or(STATUS_OVERRUN);
                ++overwritten_;
            }
            double s = std::sin(2.0 * kPi * 5.0 * static_cast<double>(n_++) / rateHz_);
            regs.TIMESTAMP.store(nowNs());
            regs.DATA.store(static_cast<uint32_t>(2048.0 + 1800.0 * s));   // 12 бит
            regs.STATUS.fetch_or(STATUS_READY);
            ++generated_;
        }
        // Прерывание вызываем уже без блокировки шины, чтобы обработчик мог читать DATA
        if ((regs.CONTROL.load() & CTRL_IRQ_EN) && irq_) irq_();
    }

    void run() {
        using namespace std::chrono;
        const auto period = nanoseconds(static_cast<int64_t>(1e9 / rateHz_));
        auto next = Clock::now() + period;
        while (running_.load()) {
            // ждем момента следующего отсчета: крупную часть спим, хвост "крутимся"
            for (;;) {
                auto now = Clock::now();
                if (now >= next) break;
                auto left = next - now;
                if (left > milliseconds(2)) std::this_thread::sleep_for(left - milliseconds(1));
                else std::this_thread::yield();
                if (!running_.load()) return;
            }
            if (regs.CONTROL.load() & CTRL_ENABLE) tick();
            next += period;
        }
    }

    double rateHz_;
    uint64_t n_ = 0;
    std::atomic<bool> running_{false};
    std::atomic<uint64_t> generated_{0};
    std::atomic<uint64_t> overwritten_{0};
    std::function<void()> irq_;
    std::mutex bus_;
    std::thread thread_;
};

// ---------------------------------------------------------------------------
// Один сценарий измерений
// ---------------------------------------------------------------------------
struct Result {
    std::string name;
    uint64_t generated = 0, processed = 0, ringDrops = 0;
    double avgLatUs = 0, maxLatUs = 0, cpuPct = 0, isrPct = 0;
};

struct Stats {
    uint64_t n = 0;
    double sumUs = 0, maxUs = 0;
    uint64_t checksum = 0;   // "полезная работа" над отсчетами
};

static Result runScenario(const std::string& name, bool useIrq, int pollIntervalUs,
                          double rateHz, int seconds) {
    using namespace std::chrono;

    AdcDevice dev(rateHz);
    RingBuffer<Sample, 1024> ring;
    Stats st;
    uint64_t ringDrops = 0;   // пишет только поток-производитель
    uint64_t isrNs = 0;       // суммарное время работы обработчика прерывания

    // Обработка отсчета приложением: замеряем время отклика (от формирования
    // отсчета устройством до момента, когда приложение до него добралось)
    auto process = [&](const Sample& s) {
        double us = static_cast<double>(nowNs() - s.tsNs) / 1000.0;
        ++st.n;
        st.sumUs += us;
        if (us > st.maxUs) st.maxUs = us;
        st.checksum += s.value;
    };

    std::mutex m;
    std::condition_variable cv;

    if (useIrq) {
        // "Обработчик прерывания": читает регистр, кладет отсчет в буфер, будит приложение
        dev.attachIrq([&]() {
            auto t0 = Clock::now();
            Sample s;
            if (dev.readData(s)) {
                if (!ring.push(s)) ++ringDrops;
            }
            { std::lock_guard<std::mutex> lk(m); }   // чтобы не потерять пробуждение
            cv.notify_one();
            isrNs += static_cast<uint64_t>(
                duration_cast<nanoseconds>(Clock::now() - t0).count());
        });
    }

    dev.regs.CONTROL.store(CTRL_ENABLE | (useIrq ? CTRL_IRQ_EN : 0u));
    dev.start();

    auto tStart = Clock::now();
    auto end = tStart + std::chrono::seconds(seconds);
    double cpu0 = threadCpuSeconds();

    if (useIrq) {
        // Приложение спит, пока прерывание не положит данные в буфер
        while (Clock::now() < end) {
            {
                std::unique_lock<std::mutex> lk(m);
                cv.wait_until(lk, end, [&] { return !ring.empty(); });
            }
            Sample s;
            while (ring.pop(s)) process(s);
        }
    } else {
        // Опрос: сами крутимся и спрашиваем регистр STATUS
        while (Clock::now() < end) {
            if (dev.status() & STATUS_READY) {
                Sample s;
                if (dev.readData(s)) {
                    if (!ring.push(s)) ++ringDrops;
                }
            }
            Sample s;
            while (ring.pop(s)) process(s);
            if (pollIntervalUs > 0)
                std::this_thread::sleep_for(microseconds(pollIntervalUs));
        }
    }

    double cpu1 = threadCpuSeconds();
    double wall = duration<double>(Clock::now() - tStart).count();
    dev.stop();

    // Дочитываем то, что осталось
    {
        Sample s;
        if (dev.readData(s)) ring.push(s);
        while (ring.pop(s)) process(s);
    }

    Result r;
    r.name = name;
    r.generated = dev.generated();
    r.processed = st.n;
    r.ringDrops = ringDrops;
    r.avgLatUs = st.n ? st.sumUs / static_cast<double>(st.n) : 0.0;
    r.maxLatUs = st.maxUs;
    double isrSec = static_cast<double>(isrNs) * 1e-9;
    r.isrPct = isrSec / wall * 100.0;
    r.cpuPct = ((cpu1 - cpu0) + isrSec) / wall * 100.0;
    return r;
}

// ---------------------------------------------------------------------------
// Быстрая проверка кольцевого буфера
// ---------------------------------------------------------------------------
static void ringSelfTest() {
    RingBuffer<int, 8> rb;
    int accepted = 0, rejected = 0;
    for (int i = 1; i <= 10; ++i) (rb.push(i) ? accepted : rejected)++;
    std::cout << "Ring buffer test (capacity 8): pushed 10, accepted " << accepted
              << ", rejected " << rejected << " (buffer full)\n";
    std::cout << "Popped in FIFO order:";
    int v;
    while (rb.pop(v)) std::cout << ' ' << v;
    std::cout << "\nPop from empty buffer: " << (rb.pop(v) ? "got value" : "empty, as expected")
              << "\n\n";
}

int main(int argc, char** argv) {
    double rateHz = argc > 1 ? std::atof(argv[1]) : 1000.0;
    int seconds = argc > 2 ? std::atoi(argv[2]) : 5;
    if (rateHz < 10.0 || rateHz > 100000.0 || seconds < 1 || seconds > 60) {
        std::cout << "Usage: lab3 [rate_hz 10..100000] [seconds 1..60]\n";
        return 1;
    }

#ifdef _WIN32
    timeBeginPeriod(1);   // точность таймера Windows 1 мс (по умолчанию ~15 мс)
#endif

    std::cout << "=== Peripheral device model: polling vs interrupts ===\n";
    std::cout << "ADC rate: " << rateHz << " Hz, " << seconds << " s per scenario\n";
    if (std::thread::hardware_concurrency() < 2)
        std::cout << "WARNING: only one logical core detected. The busy-loop polling thread\n"
                     "will starve the device thread, so its results are not reliable.\n";
    std::cout << "\n";

    ringSelfTest();

    struct Scenario { std::string name; bool irq; int pollUs; };
    std::vector<Scenario> scenarios = {
        {"Polling, busy loop", false, 0},
        {"Polling, every 1 ms", false, 1000},
        {"Polling, every 10 ms", false, 10000},
        {"Interrupt (callback)", true, 0},
    };

    std::vector<Result> results;
    for (const auto& sc : scenarios) {
        std::cout << "Running: " << sc.name << " ..." << std::endl;
        results.push_back(runScenario(sc.name, sc.irq, sc.pollUs, rateHz, seconds));
    }

    std::cout << "\n" << std::left << std::setw(24) << "Mode"
              << std::right << std::setw(11) << "Generated"
              << std::setw(11) << "Processed"
              << std::setw(9) << "Lost"
              << std::setw(14) << "Avg lat, us"
              << std::setw(14) << "Max lat, us"
              << std::setw(9) << "CPU, %"
              << std::setw(10) << "ISR, %" << "\n";
    std::cout << std::string(102, '-') << "\n";
    for (const auto& r : results) {
        uint64_t lost = r.generated > r.processed ? r.generated - r.processed : 0;
        std::cout << std::left << std::setw(24) << r.name << std::right
                  << std::setw(11) << r.generated
                  << std::setw(11) << r.processed
                  << std::setw(9) << lost
                  << std::fixed << std::setprecision(1)
                  << std::setw(14) << r.avgLatUs
                  << std::setw(14) << r.maxLatUs
                  << std::setw(9) << r.cpuPct
                  << std::setw(10) << r.isrPct << "\n";
    }

    std::cout << "\nLost = samples overwritten in the DATA register before being read\n"
                 "       or dropped because the ring buffer was full.\n"
                 "Latency = time from sample creation by the device to its processing.\n"
                 "CPU = CPU time of the application thread (+ interrupt handler time)\n"
                 "      divided by wall-clock time, in % of one core.\n";

#ifdef _WIN32
    timeEndPeriod(1);
#endif
    return 0;
}