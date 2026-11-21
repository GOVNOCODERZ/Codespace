ЛАБОРАТОРНАЯ РАБОТА 4. Алгоритмы обработки массивов на oneTBB (+OpenMP)
===================================================================

СОСТАВ АРХИВА
  src/common.h               — генерация матриц, доверительный интервал,
                                проверка корректности
  src/bmp_io.h                — чтение/запись BMP (перенесено из лабы 3)

  Задание 4.1 (операции над матрицами):
    src/matrix_ops_omp.h      — посл. + OpenMP (FOR, Sections, reduction)
    src/matrix_ops_tbb.h      — oneTBB: task_group, parallel_for (лямбда
                                 и класс), parallel_reduce (3 варианта:
                                 класс, лямбда, combinable)
    src/main_4_1.cpp           — драйвер, таблица 4.1, CSV

  Задание 4.2 (медианный фильтр):
    src/median_filter_omp.h    — посл. + OpenMP (перенесено из лабы 3)
    src/median_filter_tbb.h    — oneTBB (parallel_for по строкам —
                                  тот же уровень распараллеливания, что
                                  и OpenMP-версия, по условию задания)
    src/main_4_2.cpp            — драйвер, таблица 4.2, CSV,
                                  output_X_alg.bmp

  Задание 4.3 (текстурные признаки):
    src/texture_common.h        — гистограмма окрестности, M2/R/U/E,
                                   пороги, раскраска карт, сохранение BMP
    src/texture_features_omp.h  — посл. + OpenMP
    src/texture_features_tbb.h  — oneTBB (parallel_for + class-based
                                   parallel_reduce для min/max, как
                                   рекомендовано в задании)
    src/main_4_3.cpp              — драйвер, таблица 4.3, CSV,
                                   output_X_ver_size.bmp (карты M2/R/U/E)

  images_in/input_1..4.bmp    — тестовые изображения (перенесены из
                                 лабы 3, 1280x720..2560x1440)

ВАЖНО — ЧТО ПРОВЕРЕНО, А ЧТО НЕТ
  Я работал в песочнице без доступа в интернет и без установленного
  Intel oneTBB (libtbb-dev недоступен, Intel oneAPI не ставился). Поэтому:
    - Весь OpenMP-код (main_4_1/4_2/4_3, *_omp.h) РЕАЛЬНО скомпилирован
      и протестирован на корректность (g++ -fopenmp), результаты
      сверены с эталоном (std::sort/прямой расчёт). Все тесты прошли.
    - Весь oneTBB-код (*_tbb.h) написан по документированному API, но
      скомпилировать настоящей библиотекой TBB я не смог. Чтобы всё же
      проверить ЛОГИКУ (а не просто "похоже на TBB"), я написал
      однопоточную заглушку (эмулятор) API oneTBB, которая реально
      разбивает диапазоны пополам (как настоящий TBB) и вызывает
      split-конструкторы/join — и прогнал через неё весь код: все
      split/join-редукции, task_group, parallel_for (лямбда и класс),
      combinable — дают тот же результат, что и последовательная версия,
      на всех Ksize/N/окрестностях, что я проверял.
    - Реальную MНОГОПОТОЧНОСТЬ и быстродействие oneTBB я проверить не
      мог (в заглушке один поток). Это нужно сделать на твоей машине с
      настоящим Intel oneAPI — см. ниже.

СБОРКА (Windows, VS Code + Intel oneAPI, после настройки из прошлого
ответа — "Intel oneAPI: Initialize environment variables")
    cd src
    icx /Qiopenmp /EHsc /std:c++17 main_4_1.cpp /Fe:matrix_app.exe ^
        /I"%TBBROOT%\include" /link "%TBBROOT%\lib\intel64\tbb12.lib"

    icx /Qiopenmp /EHsc /std:c++17 main_4_2.cpp /Fe:median_app4.exe ^
        /I"%TBBROOT%\include" /link "%TBBROOT%\lib\intel64\tbb12.lib"

    icx /Qiopenmp /EHsc /std:c++17 main_4_3.cpp /Fe:texture_app.exe ^
        /I"%TBBROOT%\include" /link "%TBBROOT%\lib\intel64\tbb12.lib"

  (если /Qiopenmp не распознаётся — попробуй /Qopenmp)

  Также не забудь положить tbb12.dll рядом с .exe или в PATH при запуске
  (он обычно уже есть в PATH после инициализации окружения oneAPI).

ЗАПУСК
  Запускать из папки src/ — программы читают/пишут файлы по
  относительным путям ../data, ../images_in, ../images_out (папки data/
  и images_out/ создай заранее рядом с src/, если их нет).

    .\matrix_app.exe     -> data/matrix_results.csv      (Задание 4.1)
    .\median_app4.exe    -> data/median4_results.csv,    (Задание 4.2)
                             images_out/output_*_shell_*.bmp
    .\texture_app.exe    -> data/texture_results.csv,    (Задание 4.3)
                             images_out/output_*_*_*.bmp (карты M2/R/U/E)

  ВНИМАНИЕ по времени выполнения:
    - main_4_1.cpp: матрицы до 8000x8000, 100+ запусков на каждую из
      ~45 комбинаций (функция x вариант x поток) x 3 размера — это
      МНОГО. Сначала прогони с уменьшенными SIZES/RUNS (в начале файла),
      оцени скорость одного прохода, потом увеличивай до требуемых по
      заданию значений (SIZES: 3000-8000, RUNS>=100).
    - main_4_2.cpp / main_4_3.cpp: аналогично лабе 3 — Ksize=13 на
      2560x1440 (или окрестность 9x9 для текстур) может быть медленным,
      планируй время заранее.
    По заданию используется один ("наиболее быстрый") алгоритм сортировки
    из лабы 3 — выбрана сортировка Шелла (ShellSortPlain) по теоретическим
    соображениям (быстрее чет-нечетной перестановки на малых массивах
    вставками). Если по факту замеров из лабы 3 на твоей машине окажется
    быстрее чет-нечетная — замени ShellSortPlain на OddEvenSortPlain в
    main_4_2.cpp (две строки: SortFn/SortTag) и пересобери.

ЧТО ПРИСЛАТЬ ДЛЯ ОТЧЁТА
  1. Полный вывод консоли всех трёх приложений (или скриншоты).
  2. data/matrix_results.csv, data/median4_results.csv,
     data/texture_results.csv.
  3. Несколько файлов из images_out/ (медианный фильтр + карты
     текстурных признаков по каждому признаку).
  4. Конфигурация ПК/ОС (если не брать из переписки).
  По этим данным соберу табл. 4.1/4.2/4.3 (xlsx) и docx-отчёт с выводами
  и ответами на контрольные вопросы.
