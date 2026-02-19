// 12. Составить описание класса для представления времени. Предусмотреть возможности установки 
//времени и изменения его отдельных полей (час, мин, секунда) с проверкой допустимости 
//вводимых значений. Создать методы изменения времени на заданное количество часов, минут и секунд. 


using System;

namespace Lab2
{
    class Time
    {
        private int _hours;
        private int _minutes;
        private int _seconds;

        public int Hours
        {
            get => _hours;
            set
            {
                if (value < 0 || value > 23) throw new ArgumentOutOfRangeException("Часы должны быть от 0 до 23");
                _hours = value;
            }
        }

        public int Minutes
        {
            get => _minutes;
            set
            {
                if (value < 0 || value > 59) throw new ArgumentOutOfRangeException("Минуты должны быть от 0 до 59");
                _minutes = value;
            }
        }

        public int Seconds
        {
            get => _seconds;
            set
            {
                if (value < 0 || value > 59) throw new ArgumentOutOfRangeException("Секунды должны быть от 0 до 59");
                _seconds = value;
            }
        }

        public Time(int hours, int minutes, int seconds)
        {
            // Используем свойства, чтобы сразу прошла проверка валидности
            Hours = hours;
            Minutes = minutes;
            Seconds = seconds;
        }

        // Общий метод для изменения времени
        public void AddSeconds(int secondsToAdd)
        {
            long totalSeconds = _hours * 3600L + _minutes * 60 + _seconds + secondsToAdd;

            const int secondsInDay = 86400;
            totalSeconds %= secondsInDay;
            if (totalSeconds < 0) totalSeconds += secondsInDay;

            _hours = (int)(totalSeconds / 3600);
            _minutes = (int)((totalSeconds % 3600) / 60);
            _seconds = (int)(totalSeconds % 60);
        }

        public void AddMinutes(int minutes) => AddSeconds(minutes * 60);
        public void AddHours(int hours) => AddSeconds(hours * 3600);

        public override string ToString() => $"{Hours:D2}:{Minutes:D2}:{Seconds:D2}";
    }

    class Program
    {
        static void Main()
        {
            Console.OutputEncoding = System.Text.Encoding.UTF8;
            Console.InputEncoding = System.Text.Encoding.UTF8;
            Time time = new Time(0, 0, 0); // Инициализация объекта
            bool exit = false;

            while (!exit)
            {
                Console.WriteLine($"\nТекущее время: {time}");
                Console.WriteLine("1 - Установить время");
                Console.WriteLine("2 - Изменить время (прибавить/вычесть)");
                Console.WriteLine("0 - Выход");
                Console.Write("Выбор: ");

                if (!int.TryParse(Console.ReadLine(), out int choice)) continue;

                switch (choice)
                {
                    case 1:
                        try
                        {
                            time.Hours = GetSafeInput("Часы", 0, 23);
                            time.Minutes = GetSafeInput("Минуты", 0, 59);
                            time.Seconds = GetSafeInput("Секунды", 0, 59);
                        }
                        catch (Exception ex) { Console.WriteLine($"Ошибка: {ex.Message}"); }
                        break;

                    case 2:
                        ModifyTimeMenu(time);
                        break;

                    case 0:
                        exit = true;
                        break;
                }
            }
        }

        static void ModifyTimeMenu(Time time)
        {
            Console.WriteLine("1 - Прибавить время\n2 - Вычесть время");
            if (!int.TryParse(Console.ReadLine(), out int direction)) return;

            Console.WriteLine("Что меняем? 1-Часы, 2-Минуты, 3-Секунды");
            if (!int.TryParse(Console.ReadLine(), out int type)) return;

            Console.Write("Количество: ");
            if (!int.TryParse(Console.ReadLine(), out int value)) return;

            if (direction == 2) value = -value; // Для вычитания делаем значение отрицательным

            switch (type)
            {
                case 1: time.AddHours(value); break;
                case 2: time.AddMinutes(value); break;
                case 3: time.AddSeconds(value); break;
            }
        }

        static int GetSafeInput(string prompt, int min, int max)
        {
            int val;
            while (true)
            {
                Console.Write($"{prompt} ({min}-{max}): ");
                if (int.TryParse(Console.ReadLine(), out val) && val >= min && val <= max)
                    return val;
                Console.WriteLine("Некорректный ввод!");
            }
        }
    }
}