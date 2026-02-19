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
        public int rooms;
        /// <summary>
        /// Номер этажа.
        /// </summary>
        public int floor;
        /// <summary>
        /// Стоимость квартиры.
        /// </summary>
        public double price;
        /// <summary>
        /// Район.
        /// </summary>
        public string cityArea;


        /// <summary>
        /// Конструктор по умолчанию.
        /// </summary>
        public Appartment()
        {
            rooms = 0;
            floor = 0;
            price = 0;
            cityArea = "TEMPLATE";
        }

        /// <summary>
        /// Конструктор с параметрами.
        /// </summary>
        public Appartment(int rooms, int floor, double price, string cityArea)
        {
            this.rooms = rooms;
            this.floor = floor;
            this.price = price;
            this.cityArea = cityArea;
        }

        /// <summary>
        /// Проверяет район на соответствие данному.
        /// </summary>
        /// <param name="cityAreaName">
        /// Данный район.
        /// </param>
        /// <returns>
        /// Булевое значение.
        /// </returns>
        public bool IsCorrectCityArea(string cityAreaName) => cityArea == cityAreaName;

        /// <summary>
        /// Проверяет количество комнат на соответствие данному.
        /// </summary>
        /// <param name="roomsNumber">
        /// Данное количество комнат.
        /// </param>
        /// <returns>
        /// Булевое значение.
        /// </returns>
        public bool HasNRooms(int roomsNumber) => rooms == roomsNumber;

        /// <summary>
        /// Пробразует данные о квартире в строку для удобства вывода.
        /// </summary>
        /// <returns>
        /// Форматированная строка.
        /// </returns>
        public string DataToString()
        {
            return $"{rooms} комнат, {floor} этаж, стоимостью {price} тыс. руб., в районе {cityArea}";
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

        public int appartments_number = 0;
        
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
            StreamReader f = new StreamReader(filename);
            string s;
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
        /// <param name="cityAreaName"> Название района. </param>
        /// <param name="roomsNumber"> Количество комнат. </param>
        public void FilterByCityAreaAndRooms(string cityAreaName, int roomsNumber)
        {
            for(int i = 0; i < appartments_number; i++)
            {
                Appartment current_appartment = appartments_list[i];
                if (current_appartment.IsCorrectCityArea(cityAreaName) && current_appartment.HasNRooms(roomsNumber))
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

                string choice = Console.ReadLine();

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
                        string cityAreaName = Console.ReadLine();
                        Console.WriteLine("Введите количество комнат для фильтрации:");
                        int roomsNumber = int.Parse(Console.ReadLine());
                        Console.WriteLine($"=== Квартиры в районе {cityAreaName} с количеством комнат {roomsNumber} ===");
                        manager.FilterByCityAreaAndRooms(cityAreaName, roomsNumber);
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
