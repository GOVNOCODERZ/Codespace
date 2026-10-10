// Лабораторная работа: «Периферийные устройства ЭВМ». Вариант №4.
// Устройство «счетчик импульсов»: значение увеличивается при каждом чтении из порта.
//
// Целевая платформа: .NET 6+ (Windows). Для COM-порта нужен пакет System.IO.Ports:
//   dotnet add package System.IO.Ports
// Под Linux/macOS программа тоже запустится (COM и HID недоступны, остальное работает).

using System;
using System.IO.Ports;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using Microsoft.Win32.SafeHandles;

namespace PeripheralLab
{
    // -----------------------------------------------------------------------
    // Базовый интерфейс порта ввода-вывода
    // -----------------------------------------------------------------------
    public interface IPort
    {
        uint Read();            // чтение из порта
        void Write(uint value); // запись в порт
    }

    // -----------------------------------------------------------------------
    // Устройство «счетчик импульсов» (вариант №4).
    // Каждое чтение порта возвращает текущее значение и увеличивает его на 1.
    // -----------------------------------------------------------------------
    public class PulseCounter : IPort
    {
        private uint _value;   // регистр счетчика
        public ulong Reads { get; private set; }   // сколько раз порт был прочитан

        public uint Read()
        {
            Reads++;
            unchecked { _value++; }   // значение растет при каждом чтении
            return _value;
        }

        public void Write(uint v) => _value = v;   // запись = загрузка начального значения

        public void Reset()
        {
            _value = 0;
            Reads = 0;
        }
    }

    // -----------------------------------------------------------------------
    // Виртуальный порт светодиодов (8 бит)
    // -----------------------------------------------------------------------
    public class LedPort : IPort
    {
        private byte _reg;

        public uint Read() => _reg;
        public void Write(uint v) => _reg = (byte)(v & 0xFF);

        public string Show()
        {
            var sb = new StringBuilder();
            for (int i = 7; i >= 0; i--)
                sb.Append(((_reg >> i) & 1) != 0 ? "[#]" : "[ ]");
            return sb.ToString();
        }
    }

    // -----------------------------------------------------------------------
    // COM-порт. Для проверки без физического порта используется пара
    // виртуальных портов (com0com) и терминал на втором конце.
    // -----------------------------------------------------------------------
    public static class ComTester
    {
        public static void Run(string name)
        {
            try
            {
                using (var sp = new SerialPort(name, 9600, Parity.None, 8, StopBits.One))
                {
                    sp.ReadTimeout = 1000;    // ждем ответ не более 1 с
                    sp.WriteTimeout = 1000;
                    sp.Open();

                    sp.Write("TEST");
                    Console.WriteLine("Sent 4 bytes: TEST");

                    try
                    {
                        var buf = new byte[64];
                        int n = sp.Read(buf, 0, buf.Length);   // ждем первые байты
                        Thread.Sleep(50);                       // даем дойти остальным
                        if (sp.BytesToRead > 0 && n < buf.Length)
                            n += sp.Read(buf, n, Math.Min(sp.BytesToRead, buf.Length - n));
                        Console.WriteLine($"Received {n} bytes: {Encoding.ASCII.GetString(buf, 0, n)}");
                    }
                    catch (TimeoutException)
                    {
                        Console.WriteLine("No response (timeout)");
                    }
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Cannot use {name}: {ex.Message}");
            }
        }
    }

    // -----------------------------------------------------------------------
    // USB HID: перебор устройств, открытие, чтение входного отчета (WinAPI)
    // -----------------------------------------------------------------------
    public static class UsbHidTester
    {
        // ---- константы WinAPI ----
        private const uint DIGCF_PRESENT = 0x02;
        private const uint DIGCF_DEVICEINTERFACE = 0x10;
        private const uint GENERIC_READ = 0x80000000;
        private const uint FILE_SHARE_READ = 1;
        private const uint FILE_SHARE_WRITE = 2;
        private const uint OPEN_EXISTING = 3;
        private const uint FILE_FLAG_OVERLAPPED = 0x40000000;
        private const int ERROR_IO_PENDING = 997;
        private const int HIDP_STATUS_SUCCESS = 0x110000;
        private static readonly IntPtr InvalidHandle = new IntPtr(-1);

        // ---- структуры ----
        [StructLayout(LayoutKind.Sequential)]
        private struct SP_DEVICE_INTERFACE_DATA
        {
            public uint cbSize;
            public Guid InterfaceClassGuid;
            public uint Flags;
            public UIntPtr Reserved;
        }

        [StructLayout(LayoutKind.Sequential)]
        private struct HIDD_ATTRIBUTES
        {
            public uint Size;
            public ushort VendorID;
            public ushort ProductID;
            public ushort VersionNumber;
        }

        [StructLayout(LayoutKind.Sequential)]
        private struct HIDP_CAPS
        {
            public ushort Usage;
            public ushort UsagePage;
            public ushort InputReportByteLength;
            public ushort OutputReportByteLength;
            public ushort FeatureReportByteLength;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 17)]
            public ushort[] Reserved;
            public ushort NumberLinkCollectionNodes;
            public ushort NumberInputButtonCaps;
            public ushort NumberInputValueCaps;
            public ushort NumberInputDataIndices;
            public ushort NumberOutputButtonCaps;
            public ushort NumberOutputValueCaps;
            public ushort NumberOutputDataIndices;
            public ushort NumberFeatureButtonCaps;
            public ushort NumberFeatureValueCaps;
            public ushort NumberFeatureDataIndices;
        }

        // ---- hid.dll ----
        [DllImport("hid.dll")]
        private static extern void HidD_GetHidGuid(out Guid hidGuid);

        [DllImport("hid.dll", SetLastError = true)]
        private static extern bool HidD_GetAttributes(SafeFileHandle h, ref HIDD_ATTRIBUTES attr);

        [DllImport("hid.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        private static extern bool HidD_GetProductString(SafeFileHandle h, StringBuilder buf, uint bufLenBytes);

        [DllImport("hid.dll", SetLastError = true)]
        private static extern bool HidD_GetPreparsedData(SafeFileHandle h, out IntPtr preparsed);

        [DllImport("hid.dll")]
        private static extern bool HidD_FreePreparsedData(IntPtr preparsed);

        [DllImport("hid.dll")]
        private static extern int HidP_GetCaps(IntPtr preparsed, out HIDP_CAPS caps);

        // ---- setupapi.dll ----
        [DllImport("setupapi.dll", EntryPoint = "SetupDiGetClassDevsW", CharSet = CharSet.Unicode, SetLastError = true)]
        private static extern IntPtr SetupDiGetClassDevs(ref Guid classGuid, IntPtr enumerator,
                                                         IntPtr hwndParent, uint flags);

        [DllImport("setupapi.dll", SetLastError = true)]
        private static extern bool SetupDiEnumDeviceInterfaces(IntPtr devInfoSet, IntPtr devInfoData,
                                                               ref Guid interfaceClassGuid, uint memberIndex,
                                                               ref SP_DEVICE_INTERFACE_DATA ifData);

        [DllImport("setupapi.dll", EntryPoint = "SetupDiGetDeviceInterfaceDetailW", CharSet = CharSet.Unicode, SetLastError = true)]
        private static extern bool SetupDiGetDeviceInterfaceDetail(IntPtr devInfoSet,
                                                                   ref SP_DEVICE_INTERFACE_DATA ifData,
                                                                   IntPtr detailData, uint detailSize,
                                                                   out uint requiredSize, IntPtr devInfoData);

        [DllImport("setupapi.dll", SetLastError = true)]
        private static extern bool SetupDiDestroyDeviceInfoList(IntPtr devInfoSet);

        // ---- kernel32.dll ----
        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        private static extern SafeFileHandle CreateFile(string fileName, uint access, uint share,
                                                        IntPtr security, uint disposition,
                                                        uint flags, IntPtr template);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool ReadFile(SafeFileHandle h, IntPtr buffer, uint toRead,
                                            IntPtr bytesRead, ref NativeOverlapped ov);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool GetOverlappedResult(SafeFileHandle h, ref NativeOverlapped ov,
                                                       out uint transferred, bool wait);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool CancelIo(SafeFileHandle h);

        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        private static extern IntPtr CreateEvent(IntPtr security, bool manualReset, bool initialState, string name);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern uint WaitForSingleObject(IntPtr h, uint milliseconds);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool CloseHandle(IntPtr h);

        // -------------------------------------------------------------------
        public static void Run()
        {
            Guid guid;
            HidD_GetHidGuid(out guid);

            IntPtr info = SetupDiGetClassDevs(ref guid, IntPtr.Zero, IntPtr.Zero,
                                              DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
            if (info == InvalidHandle)
            {
                Console.WriteLine("HID device list is not available");
                return;
            }

            try
            {
                var ifd = new SP_DEVICE_INTERFACE_DATA();
                ifd.cbSize = (uint)Marshal.SizeOf<SP_DEVICE_INTERFACE_DATA>();
                int found = 0;

                for (uint i = 0; SetupDiEnumDeviceInterfaces(info, IntPtr.Zero, ref guid, i, ref ifd); i++)
                {
                    // 1) узнаем нужный размер буфера, 2) получаем путь к устройству
                    uint need;
                    SetupDiGetDeviceInterfaceDetail(info, ref ifd, IntPtr.Zero, 0, out need, IntPtr.Zero);
                    IntPtr det = Marshal.AllocHGlobal((int)need);
                    try
                    {
                        // cbSize структуры SP_DEVICE_INTERFACE_DETAIL_DATA_W: 8 (x64) или 6 (x86)
                        Marshal.WriteInt32(det, IntPtr.Size == 8 ? 8 : 6);
                        if (!SetupDiGetDeviceInterfaceDetail(info, ref ifd, det, need, out _, IntPtr.Zero))
                            continue;

                        string path = Marshal.PtrToStringUni(IntPtr.Add(det, 4));

                        // dwDesiredAccess = 0: достаточно для запроса атрибутов,
                        // работает и для клавиатур/мышей
                        using (SafeFileHandle h = CreateFile(path, 0, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                                             IntPtr.Zero, OPEN_EXISTING,
                                                             FILE_FLAG_OVERLAPPED, IntPtr.Zero))
                        {
                            if (h.IsInvalid) continue;

                            var at = new HIDD_ATTRIBUTES();
                            at.Size = (uint)Marshal.SizeOf<HIDD_ATTRIBUTES>();
                            if (!HidD_GetAttributes(h, ref at)) continue;

                            var prod = new StringBuilder(128);
                            HidD_GetProductString(h, prod, 256);   // размер в байтах

                            found++;
                            Console.WriteLine($"{found}) VID={at.VendorID:X4} PID={at.ProductID:X4}  {prod}");

                            if (found == 1)
                                ReadReport(path, h);   // читаем отчет только у первого устройства
                        }
                    }
                    finally
                    {
                        Marshal.FreeHGlobal(det);
                    }
                }

                if (found == 0) Console.WriteLine("No HID devices found");
            }
            finally
            {
                SetupDiDestroyDeviceInfoList(info);
            }
        }

        // Чтение одного входного отчета (ожидание до 1 с)
        private static void ReadReport(string path, SafeFileHandle attrHandle)
        {
            using (SafeFileHandle hr = CreateFile(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                                  IntPtr.Zero, OPEN_EXISTING,
                                                  FILE_FLAG_OVERLAPPED, IntPtr.Zero))
            {
                if (hr.IsInvalid)
                {
                    Console.WriteLine("   read denied by the system (device is used by the OS)");
                    return;
                }

                // размер входного отчета берем из дескриптора устройства
                int len = 0;
                IntPtr pp;
                if (HidD_GetPreparsedData(attrHandle, out pp))
                {
                    HIDP_CAPS caps;
                    if (HidP_GetCaps(pp, out caps) == HIDP_STATUS_SUCCESS)
                        len = caps.InputReportByteLength;
                    HidD_FreePreparsedData(pp);
                }
                if (len == 0) len = 65;

                // неуправляемый буфер: он должен оставаться на месте, пока идет асинхронное чтение
                IntPtr buf = Marshal.AllocHGlobal(len);
                IntPtr ev = CreateEvent(IntPtr.Zero, true, false, null);
                try
                {
                    var ov = new NativeOverlapped { EventHandle = ev };

                    bool ok = ReadFile(hr, buf, (uint)len, IntPtr.Zero, ref ov);
                    int err = Marshal.GetLastWin32Error();
                    if (!ok && err != ERROR_IO_PENDING)
                    {
                        Console.WriteLine($"   read error (code {err})");
                        return;
                    }

                    uint got;
                    if (WaitForSingleObject(ev, 1000) == 0 && GetOverlappedResult(hr, ref ov, out got, false))
                    {
                        var data = new byte[got];
                        Marshal.Copy(buf, data, 0, (int)got);
                        Console.WriteLine($"   report ({got} bytes): {BitConverter.ToString(data).Replace('-', ' ')}");
                    }
                    else
                    {
                        CancelIo(hr);
                        GetOverlappedResult(hr, ref ov, out _, true);   // дожидаемся отмены
                        Console.WriteLine("   no report received (timeout 1 s)");
                    }
                }
                finally
                {
                    CloseHandle(ev);
                    Marshal.FreeHGlobal(buf);
                }
            }
        }
    }

    // -----------------------------------------------------------------------
    public static class Program
    {
        private static void PrintHelp()
        {
            Console.WriteLine(
                "Commands:\n" +
                "  led <0..255>   write value to the LED port, show its state\n" +
                "  com [COMx]     open COM port (COM3 by default), exchange the TEST string\n" +
                "  usb            list HID devices, open the first one, read a report\n" +
                "  pulse [N]      read the pulse counter port N times (1 by default)\n" +
                "  load <number>  write initial value to the counter\n" +
                "  reset          reset the counter\n" +
                "  help           show this list\n" +
                "  exit           quit");
        }

        public static void Main()
        {
            var counter = new PulseCounter();
            var led = new LedPort();
            ulong total = 0;   // общее число выполненных команд

            Console.WriteLine("=== Peripheral devices. Variant 4: pulse counter ===");
            PrintHelp();

            while (true)
            {
                Console.Write("> ");
                string line = Console.ReadLine();
                if (line == null) break;   // конец ввода (Ctrl+Z / Ctrl+D)

                string[] parts = line.Split(new[] { ' ', '\t' }, StringSplitOptions.RemoveEmptyEntries);
                if (parts.Length == 0) continue;

                string cmd = parts[0].ToLowerInvariant();
                string arg = parts.Length > 1 ? parts[1] : null;

                if (cmd == "exit")
                {
                    break;
                }
                else if (cmd == "help")
                {
                    PrintHelp();
                }
                else if (cmd == "led")
                {
                    if (!int.TryParse(arg, out int v) || v < 0 || v > 255)
                    {
                        Console.WriteLine("Error: a number 0..255 is required");
                        continue;
                    }
                    led.Write((uint)v);
                    Console.WriteLine($"LED port = {led.Read()}  {led.Show()}");
                }
                else if (cmd == "com")
                {
                    if (!OperatingSystem.IsWindows())
                        Console.WriteLine("COM port is only supported on Windows in this lab");
                    else
                        ComTester.Run(arg ?? "COM3");
                }
                else if (cmd == "usb")
                {
                    if (!OperatingSystem.IsWindows())
                        Console.WriteLine("USB HID (hid.dll / setupapi) is only available on Windows");
                    else
                        UsbHidTester.Run();
                }
                else if (cmd == "pulse")
                {
                    int n = 1;
                    if (arg != null && !int.TryParse(arg, out n))
                        n = 0;
                    if (n < 1 || n > 1000000)
                    {
                        Console.WriteLine("Error: N must be between 1 and 1000000");
                        continue;
                    }
                    for (int i = 0; i < n; i++)
                    {
                        uint v = counter.Read();
                        if (n <= 10) Console.WriteLine($"Port read: {v}");
                        else if (i == n - 1) Console.WriteLine($"Last value: {v}");
                    }
                    Console.WriteLine($"Total port reads: {counter.Reads}");
                }
                else if (cmd == "load")
                {
                    if (!uint.TryParse(arg, out uint v))
                    {
                        Console.WriteLine("Error: a number 0..4294967295 is required");
                        continue;
                    }
                    counter.Write(v);
                    Console.WriteLine($"Counter loaded with {v}");
                }
                else if (cmd == "reset")
                {
                    counter.Reset();
                    Console.WriteLine("Counter reset");
                }
                else
                {
                    Console.WriteLine("Unknown command. Type help");
                    continue;
                }

                total++;
            }

            Console.WriteLine($"Commands executed: {total}. Shutting down.");
        }
    }
}