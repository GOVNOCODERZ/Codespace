namespace Lab1;
class Program{
    static void Main(string[] args){
        string filePath = "input.txt";
        string originalText = ReadFromFile(filePath);
        Console.WriteLine("=== Исходный текст из файла ===");
        Console.WriteLine(originalText);
        Console.WriteLine("\n=== Обработанный текст ===");
        Console.WriteLine(ProcessText(originalText));
    }
    static string ProcessText(string text)  
    {  
        var resultBuilder = new System.Text.StringBuilder();  
        for (int i = 0; i < text.Length - 1; i++)  
        {  
            if (text[i] == ' ' && text[i + 1] == '-')  
            {  
                int startIndex = i;  
                int endIndex = i;  
                while (endIndex < text.Length && text[endIndex] != '.' && text[endIndex] != '!' && text[endIndex] != '?')  
                {  
                    endIndex++;  
                }  

                resultBuilder.Append(text, startIndex, endIndex - startIndex);  
                i = endIndex - 1;
            }  
        }  
        return " " + resultBuilder.ToString();  
    }  
        

    static string ReadFromFile(string filepath)
    {
        return File.ReadAllText(filepath);  
    }
}