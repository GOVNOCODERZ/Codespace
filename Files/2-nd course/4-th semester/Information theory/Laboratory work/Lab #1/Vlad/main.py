from header import TextEntropyAnalyzer

def main():
    analyzer = TextEntropyAnalyzer()
    
    while True:
        print("\n--- МЕНЮ ---")
        print("1. Загрузить файл")
        print("2. Проанализировать файл")
        print("3. Результаты анализа")
        print("4. Таблица вероятностей")
        print("0. Выход")
        
        choice = input("Ваш выбор: ")

        if choice == "1":
            filename = input("Введите название файла для анализа (Enter - 'input.txt'): ").strip() or "input.txt"
            if(analyzer.load_file(filename)):
                print(f"Файл {filename} был успешно загружен.")
        elif choice == "2":
            if(analyzer.analyze()):
                print("Анализ файла завершён.")
        elif choice == "3":
            analyzer.print_report()
        elif choice == "4":
            analyzer.print_probability_table(None)
        elif choice == "0":
            print("Выход из программы.")
            break
        else:
            print("Неверный ввод.")

if __name__ == "__main__":
    main()