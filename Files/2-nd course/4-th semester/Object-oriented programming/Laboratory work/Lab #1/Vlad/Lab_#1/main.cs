using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace LabWork1
{
    class Textprocessor
    {
        static void Main(string[ ] args)
        {
            
            string OriginalText = ReadFromFile("input.txt");

            Console.WriteLine("=== Исходный текст из файла ===");
            Console.WriteLine(OriginalText);

            Console.WriteLine("\n=== Обработанный текст ===");
            Console.WriteLine(ProcessText(OriginalText));
        }

        /// <summary>
        /// Обрабатывает входящий текст,
        /// построчно преобразую каждую строку.
        /// </summary>
        /// <param name="text"> Входящий текст </param>
        /// <returns> Обработанный текст </returns>
        static string ProcessText(string text)
        {
            string [] Lines = text.Split(new[] { "\r\n", "\r", "\n" }, StringSplitOptions.None);

            for (int i = 0; i < Lines.Length; i++)
            {
                Lines[i] = SwapAdjacentWords(Lines[i]);
            }            

            return string.Join("\n", Lines);
        }
                
        /// <summary>
        /// Меняет местами два соседних слова в строке.
        /// </summary>
        /// <param name="line"> Изначальная строка </param>
        /// <returns> Модифицированная строка </returns>
        static string SwapAdjacentWords(string line)
        {
            string [] Words = line.Split(new[] {" ", ", ", ","}, StringSplitOptions.RemoveEmptyEntries);

            for (int i = 0; i < Words.Length - 1; i += 2)
            {
                (Words[i], Words[i + 1]) = (Words[i + 1], Words[i]);
            }
            return string.Join(" ", Words);
        }

        /// <summary>
        /// Считывает текст из файла.
        /// </summary>
        /// <param name="filename"> Название файла </param>
        /// <returns> Строка с содержимым файла </returns>
        static string ReadFromFile(string filename)
        {
            StreamReader f = new StreamReader(filename);
            return f.ReadToEnd();
        }


    }
}
