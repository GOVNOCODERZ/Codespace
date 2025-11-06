from modulee import Tour, TourManager # Импортируем классы из файла models.py

def demonstrate():
    """Функция для демонстрации работы с турами."""
    manager = TourManager()

    # Демонстрация 1: Загрузка из файла (если он существует) или создание вручную
    filename = "tours_all.csv"
    try:
        manager.load_file(filename)
        if not manager._TourManager__tours: # Проверка, что список не пуст
            raise FileNotFoundError # Если файл пуст, переходим к созданию вручную
    except FileNotFoundError:
        print(f"Файл '{filename}' не найден или пуст. Предлагается создать туры вручную.")
        manager.add(Tour("Pegas", "Солнечная Анталья", "Турция", 8, 75000))
        manager.add(Tour("Anex", "Древний Каир", "Египет", 12, 98000))
        manager.add(Tour("Pegas", "Все включено в Кемере", "Турция", 14, 120000))

    print("\n--- Исходный список туров ---")
    manager.display()

    # Демонстрация 2: Сохранение исходного списка в файл
    manager.save_file(filename)

    # Демонстрация 3: Выборки
    print("\n--- Фильтр: туры с длительностью более 10 дней ---")
    long_tours = manager.filter_duration(10)
    manager.display(long_tours)
    manager.save_file("tours_long.csv", long_tours)

    print("\n--- Фильтр: туры от агентства 'Pegas' с длительностью более 7 дней ---")
    pegas_tours = manager.filter_agency_duration("Pegas", 7)
    manager.display(pegas_tours)

    print("\n--- Фильтр: туры в Турцию стоимостью от 50000 до 100000 ---")
    turkey_tours = manager.filter_country_price("Турция", 50000, 100000)
    manager.display(turkey_tours)
    manager.save_file("tours_turkey_price.csv", turkey_tours)

    # Демонстрация 4: Сортировка
    manager.sort_duration()
    manager.display()


if __name__ == "__main__":
    demonstrate()