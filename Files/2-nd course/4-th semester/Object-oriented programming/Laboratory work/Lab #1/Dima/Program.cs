namespace Lab1;
class Program{
    static void Main(string[] args){
        string FilePath = "input.txt";
        string OriginalText = ReadFromFile(FilePath);
        Console.WriteLine("=== Исходный текст из файла ===");
        Console.WriteLine(OriginalText);
        Console.WriteLine("\n=== Обработанный текст ===");
        Console.WriteLine(ProcessText(OriginalText));
    }
    static string ProcessText(string text){
        string newtext = " ";
        for (int i = 0; i < text.Length; i++){
            if (text[i] == ' ' && text[i + 1] == '-'){
                while (text[i] != '.' && text[i] != '!' && text[i] != '?'){
                    newtext += text[i];
                    i++;
                }
            }
        }
        return newtext;
    }
        

    static string ReadFromFile(string filepath)
    {
        StreamReader reader = new StreamReader(filepath);
        string content = reader.ReadToEnd(); 
        reader.Close();
        return content;
    }
}