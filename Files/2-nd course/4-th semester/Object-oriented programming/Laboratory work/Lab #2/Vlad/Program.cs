namespace LabWork2
{
    /// <summary>
    /// Класс для объектов, представляющих собой квартиру.
    /// </summary>
    class Appartment
    {
        /// <summary>
        /// Количество комнат.
        /// </summary>
        public int Rooms;
        /// <summary>
        /// Номер этажа.
        /// </summary>
        public int Floor;
        /// <summary>
        /// Стоимость квартиры.
        /// </summary>
        public double Price;
        /// <summary>
        /// Район.
        /// </summary>
        public string СityArea;


        /// <summary>
        /// Конструктор по умолчанию.
        /// </summary>
        public Appartment()
        {
            Rooms = 0;
            Floor = 0;
            Price = 0;
            СityArea = string.Empty;
        }

        /// <summary>
        /// Конструктор с параметрами.
        /// </summary>
        public Appartment(int rooms, int floor, double price, string сityArea)
        {
            Rooms = rooms;
            Floor = floor;
            Price = price;
            СityArea = сityArea;
        }

        /// <summary>
        /// Проверяет район на соответствие данному.
        /// </summary>
        /// <param name="сityAreaName">
        /// Данный район.
        /// </param>
        /// <returns>
        /// Булевое значение.
        /// </returns>
        public bool IsCorrectСityArea(string сityAreaName) => СityArea == сityAreaName;

        /// <summary>
        /// Проверяет количество комнат на соответствие данному.
        /// </summary>
        /// <param name="roomsNumber">
        /// Данное количество комнат.
        /// </param>
        /// <returns>
        /// Булевое значение.
        /// </returns>
        public bool HasNRooms(int roomsNumber) => Rooms == roomsNumber;

        /// <summary>
        /// Пробразует данные о квартире в строку для удобства вывода.
        /// </summary>
        /// <returns>
        /// Форматированная строка.
        /// </returns>
        public string DataToString()
        {
            return $"{Rooms} комнат, {Floor} этаж, стоимостью {Price} тыс. руб., в районе {СityArea}";
        }

    }

    /// <summary>
    /// Класс для управления массивом квартир.
    /// </summary>
    class AppartmentsManager
    {
        /// <summary>
        /// Массив всех квартир
        /// </summary>
        public Appartment[] appartments_list;

        /// <summary>
        /// Текущее количество квартир в массиве.
        /// </summary>
        public int appartments_number = 0;
        
        /// <summary>
        /// Максимальный размер массива.
        /// </summary>
        public const int MAX_APPARTMENTS_NUMBER = 10;

        /// <summary>
        /// Конструктор по умолчанию.
        /// </summary>
        public AppartmentsManager()
        {
            appartments_list = new Appartment[MAX_APPARTMENTS_NUMBER];
        }

        /// <summary>
        /// Чтение данных из файла и заполнение ими массива квартир.
        /// </summary>
        /// <param name="filename">
        /// Название файла для считывания.
        /// </param>
        public void InputFromFile(string filename)
        {
            using StreamReader f = new StreamReader(filename);
            string? s;
            int index = 0;

            while ((s = f.ReadLine()) != null)
            {
                // читаем строку файла, парсим параметры и создаём объект квартиры по ним
                string[] obj_params = s.Split(";");
                appartments_list[index] = new Appartment(
                    int.Parse(obj_params[0]),
                    int.Parse(obj_params[1]),
                    double.Parse(obj_params[2]),
                    obj_params[3]
                );
                appartments_number++;
                index++;
            }
        }

        /// <summary>
        /// Вывод в консоль всех квартир.
        /// </summary>
        public void OutputToConsole()
        {
            for (int i = 0; i < appartments_number; i++)
            {
                if(appartments_list[i] != null)
                    Console.WriteLine($"Квартира №{i + 1}: {appartments_list[i].DataToString()}");
            }
        }

        /// <summary>
        /// Вывод в консоль всех квартир, соответствующих данному району и количеству комнат.
        /// </summary>
        /// <param name="сityAreaName"> Название района. </param>
        /// <param name="roomsNumber"> Количество комнат. </param>
        public void FilterByСityAreaAndRooms(string сityAreaName, int roomsNumber)
        {
            for(int i = 0; i < appartments_number; i++)
            {
                Appartment current_appartment = appartments_list[i];
                if (current_appartment.IsCorrectСityArea(сityAreaName) && current_appartment.HasNRooms(roomsNumber))
                {
                    Console.WriteLine($"Квартира №{i + 1}: {current_appartment.DataToString()}");
                }
            }
        }
    }

    class Program
    {
        static void Main(string[] args)
        {
            AppartmentsManager manager = new AppartmentsManager();
            string input_file_name = "input.txt";

            while (true)
            {
                Console.WriteLine("\n=== МЕНЕДЖЕР БАЗЫ ДАННЫХ КВАРТИР ===");
                Console.WriteLine("1. Загрузить данные из файла");
                Console.WriteLine("2. Вывод всего списка квартир");
                Console.WriteLine("3. Фильтрация списка по району и количеству комнат");
                Console.WriteLine("0. Выход");

                string? choice = Console.ReadLine();

                switch (choice)
                {
                    case "1":
                        manager.InputFromFile(input_file_name);
                        Console.WriteLine("Данные успешно загружены.");
                        break;
                    case "2":
                        Console.WriteLine("=== Все квартиры ===");
                        manager.OutputToConsole();
                        break;
                    case "3":
                        Console.WriteLine("Введите название района для фильтрации:");
                        string? сityAreaName = Console.ReadLine();
                        Console.WriteLine("Введите количество комнат для фильтрации:");
                        int roomsNumber = int.Parse(Console.ReadLine());
                        Console.WriteLine($"=== Квартиры в районе {сityAreaName} с количеством комнат {roomsNumber} ===");
                        manager.FilterByСityAreaAndRooms(сityAreaName, roomsNumber);
                        break;
                    case "0":
                        Console.WriteLine("Завершение работы...");
                        return;
                    default:
                        Console.WriteLine("Неверный выбор.");
                        break;
                }
            }
        }
    }
}
