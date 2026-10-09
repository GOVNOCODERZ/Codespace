// Лабораторная работа №5. Высокоуровневая работа с периферийными устройствами.
// Ввод видеопотока с камеры (или из файла) средствами OpenCV, преобразование
// изображения, показ в окнах, измерение FPS и доли процессорного времени.
//
// Использование:  lab5 [источник] [--no-show] [--delay мс] [--frames N]
//   источник    номер камеры (по умолчанию 0) или имя видеофайла
//   --no-show   не показывать окна (для запуска без графической оболочки)
//   --delay мс  пауза после кадра, по умолчанию 33 мс (~30 кадр/с); 0 - без паузы
//   --frames N  остановиться после N кадров (по умолчанию - до Esc или конца файла)
//
// Linux:   g++ -O2 -std=c++17 lab5.cpp -o lab5 $(pkg-config --cflags --libs opencv4)
// Windows: cl /EHsc /O2 /std:c++17 lab5.cpp /I C:\opencv\build\include ^
//             /link /LIBPATH:C:\opencv\build\x64\vc16\lib opencv_world4100.lib

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

// Процессорное время, затраченное процессом (все потоки), в секундах.
static double cpuSeconds() {
#ifdef _WIN32
    FILETIME c, e, k, u;
    GetProcessTimes(GetCurrentProcess(), &c, &e, &k, &u);
    auto toSec = [](FILETIME f) {
        unsigned long long t = (static_cast<unsigned long long>(f.dwHighDateTime) << 32)
                               | f.dwLowDateTime;
        return t * 1e-7;
    };
    return toSec(k) + toSec(u);
}
#else
    timespec ts;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}
#endif

// Преобразование: сглаживание 3x3 и «ручное» обнуление красного и синего каналов.
static void transform(const cv::Mat& src, cv::Mat& dst) {
    cv::blur(src, dst, cv::Size(3, 3));
    if (dst.type() != CV_8UC3) return;
    for (int y = 0; y < dst.rows; ++y) {
        uchar* ptr = dst.ptr<uchar>(y);
        for (int x = 0; x < dst.cols; ++x) {
            ptr[3 * x] = 0;        // Blue
            ptr[3 * x + 2] = 0;    // Red
        }
    }
}

int main(int argc, char* argv[]) {
    std::string source = "0";
    bool show = true;
    int delayMs = 33;
    long maxFrames = -1;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--no-show") show = false;
        else if (a == "--delay" && i + 1 < argc) delayMs = std::atoi(argv[++i]);
        else if (a == "--frames" && i + 1 < argc) maxFrames = std::atol(argv[++i]);
        else source = a;
    }

    cv::VideoCapture cap;
    bool isCamera = source.find_first_not_of("0123456789") == std::string::npos;
    if (isCamera) cap.open(std::atoi(source.c_str()));
    else cap.open(source);
    if (!cap.isOpened()) {
        std::cerr << "Не удалось открыть источник видео: " << source << "\n";
        return 1;
    }

    using clk = std::chrono::steady_clock;
    auto sec = [](clk::time_point a, clk::time_point b) {
        return std::chrono::duration<double>(b - a).count();
    };

    double tRead = 0, tProc = 0, tShow = 0, tWait = 0;
    long frames = 0;
    cv::Mat frame, result;

    const double cpu0 = cpuSeconds();
    const auto start = clk::now();
    while (maxFrames < 0 || frames < maxFrames) {
        auto t0 = clk::now();
        if (!cap.read(frame) || frame.empty()) break;          // 1. ввод кадра
        auto t1 = clk::now();
        transform(frame, result);                              // 2. преобразование
        auto t2 = clk::now();
        int key = -1;
        if (show) {                                            // 3. показ
            cv::imshow("camera", frame);
            cv::imshow("transformed", result);
        }
        auto t3 = clk::now();
        if (show) key = cv::waitKey(delayMs > 0 ? delayMs : 1);  // 4. ожидание
        else if (delayMs > 0) std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
        auto t4 = clk::now();

        tRead += sec(t0, t1);
        tProc += sec(t1, t2);
        tShow += sec(t2, t3);
        tWait += sec(t3, t4);
        ++frames;
        if (key == 27) break;                                  // Esc
    }
    const double wall = sec(start, clk::now());
    const double cpu = cpuSeconds() - cpu0;
    cap.release();
    if (show) cv::destroyAllWindows();

    if (frames == 0) { std::cerr << "Кадры не получены\n"; return 1; }
    const int cores = cv::getNumberOfCPUs();
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Кадр: " << frame.cols << "x" << frame.rows << ", кадров: " << frames
              << ", время: " << wall << " с\n";
    std::cout << "Скорость: " << frames / wall << " кадр/с\n";
    std::cout << "Среднее время на кадр, мс: ввод " << tRead / frames * 1e3
              << ", преобразование " << tProc / frames * 1e3
              << ", показ " << tShow / frames * 1e3
              << ", ожидание " << tWait / frames * 1e3 << "\n";
    std::cout << "Процессорное время: " << cpu << " с; загрузка " << cpu / wall * 100
              << " % одного ядра, " << cpu / wall / cores * 100 << " % всего процессора ("
              << cores << " лог. ядер)\n";
    return 0;
}
