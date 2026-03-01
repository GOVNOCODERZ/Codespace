namespace LabWork3
{
    public class Staff
    {
        public string? FullName { get; set; }
        public int Age { get; set; }

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
            if (obj is not Staff other || GetType() != obj.GetType())  
            {  
                return false;  
            }  
            return FullName == other.FullName && Age == other.Age;  
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
            if (!base.Equals(obj))  
            {  
                return false;  
            }  
            var other = (Worker)obj;  
            return Rank == other.Rank;  
        }  

        public override int GetHashCode()
        {
            return HashCode.Combine(FullName, Age, Rank);
        }
    }

    public class Engineer : Staff
    {
        public string? Specialization { get; set; }

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
            if (!base.Equals(obj))  
            {  
                return false;  
            }  
            var other = (Engineer)obj;  
            return Specialization == other.Specialization;  
        }

        public override int GetHashCode()
        {
            return HashCode.Combine(FullName, Age, Specialization);
        }
    }

    public class Administrator : Staff
    {
        public string? Position { get; set; }

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
            if (!base.Equals(obj))  
            {  
                return false;  
            }  
            var other = (Administrator)obj;  
            return Position == other.Position;  
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

            for (var i = 0; i < staff_list.Length; i++)  
            {  
                var person = staff_list[i];  
                Console.WriteLine($"{i + 1}) {person.ToString()}");  
                person.Print();  
                Console.WriteLine(new string('-', 20));
            } 
            
            Console.WriteLine($"Сравнение 1) и 2): {staff_list[0].Equals(staff_list[1])}");
            Console.WriteLine($"Сравнение 1) и 3): {staff_list[0].Equals(staff_list[2])}");
        }
    }
}