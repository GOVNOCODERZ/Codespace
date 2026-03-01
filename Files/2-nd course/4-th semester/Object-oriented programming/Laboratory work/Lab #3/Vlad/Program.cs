namespace LabWork3
{
    public class Staff
    {
        public string FullName { get; set; }
        public int Age { get; set; }
        public int Experience { get; set; }

        public virtual void Print()
        {
            Console.WriteLine($"ФИО: {FullName}, Возраст: {Age}");
        }

        public override string ToString()
        {
            return $"Сотрудник {FullName}";
        }

        public override bool Equals(object obj)
        {
            if (obj is Staff other)
            {
                return this.FullName == other.FullName && this.Age == other.Age;
            }
            return false;
        }

        public override int GetHashCode()
        {
            return HashCode.Combine(FullName, Age);
        }
    }

    public class Worker : Staff
    {
        public int Rank { get; set; }

        public override void Print()
        {
            base.Print();
            Console.WriteLine($"  Разряд: {Rank}");
        }

        public override string ToString()
        {
            return base.ToString() + $" (Рабочий)";
        }

        public override bool Equals(object obj)
        {
            if (obj is Worker other)
            {
                return base.Equals(obj) && this.Rank == other.Rank;
            }
            return false;
        }

        public override int GetHashCode()
        {
            return HashCode.Combine(FullName, Age, Rank);
        }
    }

    public class Engineer : Staff
    {
        public string Specialization { get; set; }

        public override void Print()
        {
            base.Print();
            Console.WriteLine($"  Специализация: {Specialization}");
        }

        public override string ToString()
        {
            return base.ToString() + $" (Инженер)";
        }

        public override bool Equals(object obj)
        {
            if (obj is Engineer other)
            {
                return base.Equals(obj) && this.Specialization == other.Specialization;
            }
            return false;
        }

        public override int GetHashCode()
        {
            return HashCode.Combine(FullName, Age, Specialization);
        }
    }

    public class Administrator : Staff
    {
        public string Position { get; set; }

        public override void Print()
        {
            base.Print();
            Console.WriteLine($"  Должность: {Position}");
        }

        public override string ToString()
        {
            return base.ToString() + $" (Администратор)";
        }

        public override bool Equals(object obj)
        {
            if (obj is Administrator other)
            {
                return base.Equals(obj) && this.Position == other.Position;
            }
            return false;
        }

        public override int GetHashCode()
        {
            return HashCode.Combine(FullName, Age, Position);
        }
    }

    class Program
    {
        static void Main()
        {
            Staff[] staff_list =
            [
                new Worker { FullName = "Иванов И.И.", Age = 30, Rank = 5 },
                new Worker { FullName = "Иванов И.И.", Age = 30, Rank = 5 },
                new Engineer { FullName = "Петров П.П.", Age = 35, Specialization = "АСУП" },
                new Administrator { FullName = "Сидоров С.С.", Age = 45, Position = "Директор" },
            ];

            foreach (var person in staff_list)
            {
                Console.WriteLine($"{staff_list.IndexOf(person) + 1}) {person.ToString()}");
                person.Print(); // Вызовется метод конкретного класса
                Console.WriteLine(new string('-', 20));
            }
            
            Console.WriteLine($"Сравнение 1) и 2): {staff_list[0].Equals(staff_list[1])}");
            Console.WriteLine($"Сравнение 1) и 3): {staff_list[0].Equals(staff_list[2])}");
        }
    }
}