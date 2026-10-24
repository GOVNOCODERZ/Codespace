// Лабораторная работа №5: «Высокоуровневая работа с периферийными устройствами».
// Ввод видеопотока с Web-камеры через OpenCV (обертка OpenCvSharp4),
// преобразование изображения, измерение FPS и доли времени процессора.
//
// Сборка и запуск (.NET 6+, Windows):
//   dotnet new console -n CameraLab
//   cd CameraLab
//   dotnet add package OpenCvSharp4
//   dotnet add package OpenCvSharp4.runtime.win
//   (заменить Program.cs этим файлом)
//   dotnet run                 // обычный режим: пауза 33 мс на кадр (~30 FPS)
//   dotnet run -- fast         // без паузы: показывает предел скорости обработки
//
// Управление в окне: 0 - без преобразования, 1 - сглаживание,
// 2 - только зеленый канал (ручной доступ к пикселям), 3 - границы (Canny), Esc - выход.

using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using OpenCvSharp;

namespace CameraLab
{
    public static class Program
    {
        private enum Mode { None, Blur, GreenOnly, Edges }

        private static byte[] _pixels = new byte[0]; // буфер для ручного доступа к пикселям

        // Преобразования изображения
        private static void Transform(Mat src, Mat dst, Mode mode)
        {
            switch (mode)
            {
                case Mode.Blur:
                {
                    // аналог cvSmooth(frame, image, CV_BLUR, 3, 3)
                    Cv2.Blur(src, dst, new Size(3, 3));
                    break;
                }

                case Mode.GreenOnly:
                {
                    // ручное редактирование данных: обнуляем Blue и Red (как в примере из методички)
                    src.CopyTo(dst);
                    int len = (int)(dst.Total() * dst.ElemSize());
                    if (_pixels.Length != len) _pixels = new byte[len];

                    Marshal.Copy(dst.Data, _pixels, 0, len);
                    for (int i = 0; i + 2 < len; i += 3)
                    {
                        _pixels[i] = 0; // Blue
                        _pixels[i + 2] = 0; // Red
                    }
                    Marshal.Copy(_pixels, 0, dst.Data, len);
                    break;
                }

                case Mode.Edges:
                {
                    using (var gray = new Mat())
                    using (var edges = new Mat())
                    {
                        Cv2.CvtColor(src, gray, ColorConversionCodes.BGR2GRAY);
                        Cv2.Canny(gray, edges, 80, 160);
                        Cv2.CvtColor(edges, dst, ColorConversionCodes.GRAY2BGR);
                    }
                    break;
                }

                default:
                    src.CopyTo(dst);
                    break;
            }
        }

        public static int Main(string[] args)
        {
            // Пауза после показа кадра: 33 мс (~30 кадров/с) или 1 мс в режиме "fast"
            int delay = (args.Length > 0 && args[0] == "fast") ? 1 : 33;
            Mode mode = Mode.Blur;

            using (var capture = new VideoCapture(0)) // первая камера, нумерация с нуля
            {
                if (!capture.IsOpened())
                {
                    Console.WriteLine("Error: cannot open camera 0");
                    return 1;
                }

                Console.WriteLine($"Camera opened. Delay per frame: {delay} ms");
                Console.WriteLine("Keys: 0 none, 1 blur, 2 green only, 3 edges, Esc exit");

                Process proc = Process.GetCurrentProcess();
                double tickMs = 1000.0 / Stopwatch.Frequency;

                // накопители времени по этапам (в тиках Stopwatch)
                long totCapture = 0, totTransform = 0, totShow = 0, totWait = 0;
                long intCapture = 0, intTransform = 0, intShow = 0; // за текущую секунду
                long frames = 0, intFrames = 0;

                Stopwatch wall = Stopwatch.StartNew();
                Stopwatch stage = new Stopwatch();
                TimeSpan cpuStart = proc.TotalProcessorTime;
                TimeSpan intCpuStart = cpuStart;
                long intWallStart = wall.ElapsedTicks;

                using (var frame = new Mat())
                using (var processed = new Mat())
                {
                    while (true)
                    {
                        // 1) ввод кадра с камеры
                        stage.Restart();
                        bool ok = capture.Read(frame);
                        long tCap = stage.ElapsedTicks;
                        if (!ok || frame.Empty()) break;

                        // 2) преобразование изображения
                        stage.Restart();
                        Transform(frame, processed, mode);
                        long tTr = stage.ElapsedTicks;

                        // 3) показ в окнах
                        stage.Restart();
                        Cv2.ImShow("original", frame);
                        Cv2.ImShow("processed", processed);
                        long tShow = stage.ElapsedTicks;

                        // 4) ожидание клавиши (она же служит паузой между кадрами)
                        stage.Restart();
                        int key = Cv2.WaitKey(delay);
                        long tWait = stage.ElapsedTicks;

                        totCapture += tCap; totTransform += tTr; totShow += tShow; totWait += tWait;
                        intCapture += tCap; intTransform += tTr; intShow += tShow;
                        frames++; intFrames++;

                        if (key == 27) break; // Esc
                        if (key == '0') mode = Mode.None;
                        else if (key == '1') mode = Mode.Blur;
                        else if (key == '2') mode = Mode.GreenOnly;
                        else if (key == '3') mode = Mode.Edges;

                        // Раз в секунду печатаем текущие показатели
                        double intSec = (wall.ElapsedTicks - intWallStart) / (double)Stopwatch.Frequency;
                        if (intSec >= 1.0)
                        {
                            proc.Refresh();
                            TimeSpan cpuNow = proc.TotalProcessorTime;
                            double cpuPct = (cpuNow - intCpuStart).TotalSeconds / intSec * 100.0;

                            Console.WriteLine(
                                $"FPS {intFrames / intSec,5:F1} | mode {mode,-9} | " +
                                $"capture {intCapture * tickMs / intFrames,5:F1} ms, " +
                                $"transform {intTransform * tickMs / intFrames,5:F1} ms, " +
                                $"show {intShow * tickMs / intFrames,5:F1} ms | " +
                                $"CPU {cpuPct,5:F1}% of one core");

                            intCpuStart = cpuNow;
                            intWallStart = wall.ElapsedTicks;
                            intFrames = 0; intCapture = 0; intTransform = 0; intShow = 0;
                        }
                    }
                }

                // Итоговый отчет
                wall.Stop();
                proc.Refresh();
                double totalSec = wall.Elapsed.TotalSeconds;
                double cpuSec = (proc.TotalProcessorTime - cpuStart).TotalSeconds;

                if (frames > 0 && totalSec > 0)
                {
                    double busyTicks = totCapture + totTransform + totShow;
                    Console.WriteLine();
                    Console.WriteLine("=== Summary ===");
                    Console.WriteLine($"Frames processed        : {frames}");
                    Console.WriteLine($"Total time              : {totalSec:F2} s");
                    Console.WriteLine($"Average FPS             : {frames / totalSec:F1}");
                    Console.WriteLine($"Avg capture             : {totCapture * tickMs / frames:F2} ms/frame");
                    Console.WriteLine($"Avg transform           : {totTransform * tickMs / frames:F2} ms/frame");
                    Console.WriteLine($"Avg show                : {totShow * tickMs / frames:F2} ms/frame");
                    Console.WriteLine($"Avg wait (WaitKey)      : {totWait * tickMs / frames:F2} ms/frame");
                    Console.WriteLine($"Processing share        : {busyTicks * tickMs / 1000.0 / totalSec * 100:F1}% of wall time " + "(capture + transform + show)");
                    Console.WriteLine($"CPU time used           : {cpuSec:F2} s");
                    Console.WriteLine($"CPU load                : {cpuSec / totalSec * 100:F1}% of one core, " +
                                      $"{cpuSec / totalSec / Environment.ProcessorCount * 100:F1}% of all " + $"{Environment.ProcessorCount} logical cores");
                }

                Cv2.DestroyAllWindows();
            }

            return 0;
        }
    }
}