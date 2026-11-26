// Лабораторная работа: «Периферийные устройства ЭВМ». Вариант №4.
// Устройство «счетчик импульсов»: значение увеличивается при каждом чтении из порта.
//
// Сборка (Visual Studio, Developer Command Prompt):  cl /EHsc /utf-8 main.cpp
// Сборка (MinGW):  g++ -std=c++17 main.cpp -o lab -lsetupapi -lhid
// Под Linux программа тоже собирается (COM и HID недоступны, остальное работает).

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <setupapi.h>
extern "C" {
#include <hidsdi.h>
}
#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "hid.lib")
#endif

// ---------------------------------------------------------------------------
// Базовый интерфейс порта ввода-вывода
// ---------------------------------------------------------------------------
class IPort {
public:
    virtual ~IPort() = default;
    virtual uint32_t read() = 0;               // чтение из порта
    virtual void write(uint32_t value) = 0;    // запись в порт
};

// ---------------------------------------------------------------------------
// Устройство «счетчик импульсов» (вариант №4).
// Каждое чтение порта возвращает текущее значение и увеличивает его на 1.
// ---------------------------------------------------------------------------
class PulseCounter : public IPort {
    uint32_t value_ = 0;        // регистр счетчика
    uint64_t reads_ = 0;        // сколько раз порт был прочитан
public:
    uint32_t read() override {
        ++reads_;
        return ++value_;        // значение растет при каждом чтении
    }
    void write(uint32_t v) override { value_ = v; }   // запись = загрузка начального значения
    void reset() { value_ = 0; reads_ = 0; }
    uint64_t reads() const { return reads_; }
};

// ---------------------------------------------------------------------------
// Виртуальный порт светодиодов (8 бит)
// ---------------------------------------------------------------------------
class LedPort : public IPort {
    uint8_t reg_ = 0;
public:
    uint32_t read() override { return reg_; }
    void write(uint32_t v) override { reg_ = static_cast<uint8_t>(v & 0xFF); }
    std::string show() const {
        std::string s;
        for (int i = 7; i >= 0; --i) s += ((reg_ >> i) & 1) ? "[#]" : "[ ]";
        return s;
    }
};

// ---------------------------------------------------------------------------
// COM-порт (Windows API). Для проверки без физического порта используется
// пара виртуальных портов (com0com) и терминал/эмулятор устройства на втором конце.
// ---------------------------------------------------------------------------
static void comTest(const std::string& name) {
#ifdef _WIN32
    std::string path = "\\\\.\\" + name;
    HANDLE h = CreateFileA(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr,
                           OPEN_EXISTING, 0, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        std::cout << "Не удалось открыть " << name
                  << " (код ошибки " << GetLastError() << ")\n";
        return;
    }
    DCB dcb{};
    dcb.DCBlength = sizeof(dcb);
    GetCommState(h, &dcb);
    dcb.BaudRate = CBR_9600;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    SetCommState(h, &dcb);

    COMMTIMEOUTS to{};
    to.ReadIntervalTimeout = 50;
    to.ReadTotalTimeoutConstant = 1000;   // ждем ответ не более 1 с
    to.WriteTotalTimeoutConstant = 1000;
    SetCommTimeouts(h, &to);

    const char msg[] = "TEST";
    DWORD n = 0;
    if (!WriteFile(h, msg, 4, &n, nullptr)) {
        std::cout << "Ошибка записи\n";
    } else {
        std::cout << "Отправлено " << n << " байт: TEST\n";
        char buf[64] = {};
        if (ReadFile(h, buf, sizeof(buf) - 1, &n, nullptr) && n > 0)
            std::cout << "Получено " << n << " байт: " << std::string(buf, n) << "\n";
        else
            std::cout << "Ответ не получен (тайм-аут)\n";
    }
    CloseHandle(h);
#else
    std::cout << "COM-порт (" << name << ") доступен только в Windows\n";
#endif
}

// ---------------------------------------------------------------------------
// USB HID: перебор устройств, открытие первого доступного, чтение отчета
// ---------------------------------------------------------------------------
static void usbTest() {
#ifdef _WIN32
    GUID guid;
    HidD_GetHidGuid(&guid);
    HDEVINFO info = SetupDiGetClassDevsA(&guid, nullptr, nullptr,
                                         DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (info == INVALID_HANDLE_VALUE) {
        std::cout << "Список HID-устройств недоступен\n";
        return;
    }

    SP_DEVICE_INTERFACE_DATA ifd{};
    ifd.cbSize = sizeof(ifd);
    int found = 0;
    for (DWORD i = 0; SetupDiEnumDeviceInterfaces(info, nullptr, &guid, i, &ifd); ++i) {
        DWORD need = 0;
        SetupDiGetDeviceInterfaceDetailA(info, &ifd, nullptr, 0, &need, nullptr);
        auto* det = static_cast<SP_DEVICE_INTERFACE_DETAIL_DATA_A*>(malloc(need));
        det->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_A);
        if (SetupDiGetDeviceInterfaceDetailA(info, &ifd, det, need, nullptr, nullptr)) {
            // dwDesiredAccess = 0: достаточно для запроса атрибутов,
            // работает и для клавиатур/мышей
            HANDLE h = CreateFileA(det->DevicePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                   nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);
            if (h != INVALID_HANDLE_VALUE) {
                HIDD_ATTRIBUTES at{};
                at.Size = sizeof(at);
                if (HidD_GetAttributes(h, &at)) {
                    wchar_t prod[128] = L"";
                    HidD_GetProductString(h, prod, sizeof(prod));
                    ++found;
                    std::cout << found << ") VID=" << std::hex << std::uppercase << std::setw(4)
                              << std::setfill('0') << at.VendorID << " PID=" << std::setw(4)
                              << at.ProductID << std::dec << std::setfill(' ') << "  ";
                    std::wcout << prod << L"\n";
                    if (found == 1) {
                        // чтение одного входного отчета (до 1 с)
                        HANDLE hr = CreateFileA(det->DevicePath, GENERIC_READ,
                                                FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                                                OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);
                        if (hr != INVALID_HANDLE_VALUE) {
                            // размер входного отчета берем из дескриптора устройства
                            USHORT len = 0;
                            PHIDP_PREPARSED_DATA pp = nullptr;
                            if (HidD_GetPreparsedData(h, &pp)) {
                                HIDP_CAPS caps{};
                                if (HidP_GetCaps(pp, &caps) == HIDP_STATUS_SUCCESS)
                                    len = caps.InputReportByteLength;
                                HidD_FreePreparsedData(pp);
                            }
                            if (len == 0) len = 65;
                            std::vector<unsigned char> buf(len, 0);
                            OVERLAPPED ov{};
                            ov.hEvent = CreateEventA(nullptr, TRUE, FALSE, nullptr);
                            DWORD got = 0;
                            ReadFile(hr, buf.data(), len, nullptr, &ov);
                            if (WaitForSingleObject(ov.hEvent, 1000) == WAIT_OBJECT_0 &&
                                GetOverlappedResult(hr, &ov, &got, FALSE)) {
                                std::cout << "   отчет (" << got << " байт):";
                                for (DWORD k = 0; k < got; ++k)
                                    std::cout << ' ' << std::hex << std::setw(2)
                                              << std::setfill('0') << int(buf[k])
                                              << std::dec << std::setfill(' ');
                                std::cout << "\n";
                            } else {
                                CancelIo(hr);
                                std::cout << "   отчет не получен (тайм-аут 1 с)\n";
                            }
                            CloseHandle(ov.hEvent);
                            CloseHandle(hr);
                        } else {
                            std::cout << "   чтение запрещено системой (занято ОС)\n";
                        }
                    }
                }
                CloseHandle(h);
            }
        }
        free(det);
    }
    SetupDiDestroyDeviceInfoList(info);
    if (!found) std::cout << "HID-устройства не найдены\n";
#else
    std::cout << "USB HID (hid.dll / setupapi) доступен только в Windows\n";
#endif
}

// ---------------------------------------------------------------------------
static void printHelp() {
    std::cout <<
        "Команды:\n"
        "  led <0..255>   записать значение в порт светодиодов, показать состояние\n"
        "  com [COMx]     открыть COM-порт (по умолчанию COM3), обмен строкой TEST\n"
        "  usb            найти HID-устройства, открыть первое, прочитать данные\n"
        "  pulse [N]      прочитать порт счетчика импульсов N раз (по умолчанию 1)\n"
        "  load <число>   записать начальное значение в счетчик\n"
        "  reset          сбросить счетчик\n"
        "  help           список команд\n"
        "  exit           выход\n";
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    PulseCounter counter;
    LedPort led;
    unsigned long long total = 0;   // общее число выполненных команд

    std::cout << "=== Периферийные устройства. Вариант 4: счетчик импульсов ===\n";
    printHelp();

    std::string line;
    while (true) {
        std::cout << "> " << std::flush;
        if (!std::getline(std::cin, line)) break;
        std::istringstream in(line);
        std::string cmd;
        in >> cmd;
        if (cmd.empty()) continue;

        if (cmd == "exit") break;
        else if (cmd == "help") printHelp();
        else if (cmd == "led") {
            long v;
            if (!(in >> v) || v < 0 || v > 255) {
                std::cout << "Ошибка: нужно число 0..255\n";
                continue;
            }
            led.write(static_cast<uint32_t>(v));
            std::cout << "Порт LED = " << led.read() << "  " << led.show() << "\n";
        } else if (cmd == "com") {
            std::string name = "COM3";
            in >> name;
            comTest(name);
        } else if (cmd == "usb") usbTest();
        else if (cmd == "pulse") {
            long n = 1;
            in >> n;
            if (n < 1 || n > 1000000) {
                std::cout << "Ошибка: N должно быть от 1 до 1000000\n";
                continue;
            }
            for (long i = 0; i < n; ++i) {
                uint32_t v = counter.read();
                if (n <= 10) std::cout << "Чтение порта: " << v << "\n";
                else if (i == n - 1) std::cout << "Последнее значение: " << v << "\n";
            }
            std::cout << "Всего чтений порта: " << counter.reads() << "\n";
        } else if (cmd == "load") {
            long long v;
            if (!(in >> v) || v < 0 || v > 0xFFFFFFFFLL) {
                std::cout << "Ошибка: нужно число 0..4294967295\n";
                continue;
            }
            counter.write(static_cast<uint32_t>(v));
            std::cout << "Счетчик загружен значением " << v << "\n";
        } else if (cmd == "reset") {
            counter.reset();
            std::cout << "Счетчик сброшен\n";
        } else {
            std::cout << "Неизвестная команда. Введите help\n";
            continue;
        }
        ++total;
    }
    std::cout << "Выполнено команд: " << total << ". Завершение.\n";
    return 0;
}
