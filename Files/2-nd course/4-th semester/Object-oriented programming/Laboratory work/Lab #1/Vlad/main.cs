using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
namespace LabWork1
{
    class Textprocessor
    {
        static void Main(string[ ] args)
        {
            string OriginalText =
            """
            Что не убивает меня, то делает меня сильнее.
            Мудрость приходит слишком поздно.
            Истина — это женщина.
            Бог мёртв.
            В человеке есть нечто, что хочет превзойти себя.
            Нравственность — это предрассудок.
            Сила воли — основа бытия.
            Мир — это хаос.
            Я мыслю, следовательно, я существую — это ошибка.
            Жизнь — это борьба.
            """;

            Console.WriteLine("=== Исходный текст ===");
            Console.WriteLine(OriginalText);

            Console.WriteLine("=== Обработанный текст ===");
            Console.WriteLine(ProcessText(OriginalText));

        }
        /// <summary>
        /// Меняет местами два соседних слова в строке.
        /// </summary>
        /// <param name="line"> Изначальная строка </param>
        /// <returns> Модифицированная строка </returns>
        static string SwapAdjacentWords(string line)
        {
            string [] Words = line.Split(new[] {" ", ", "}, StringSplitOptions.RemoveEmptyEntries);

            for (int i = 0; i < Words.Length - 1; i += 2)
            {
                string TempWord = Words[i];
                Words[i] = Words[i + 1];
                Words[i + 1] = TempWord;
            }
            return string.Join(" ", Words);
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
                var temp = Lines[i];
                Lines[i] = SwapAdjacentWords(temp);
            }            

            return string.Join("\n", Lines);
        }
    }
}
