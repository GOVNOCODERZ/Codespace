import Data.Foldable (asum)

-- Тип данных для представления содержимого каталога
data FileSystem = File { name :: String, size :: Int }
                | Directory { name :: String, contents :: [FileSystem] }
                deriving (Show)

-- Функция dirAll: получить список полных имен всех файлов
dirAll :: FileSystem -> [String]
dirAll (File name _) = [name] -- Для файла просто возвращаем его имя
dirAll (Directory dirName contents) =
    concatMap (\item -> map (dirName ++) (dirAll item)) contents

-- Функция find: найти путь к файлу с заданным именем
find :: String -> FileSystem -> Maybe String
find target (File fileName _)
    | target == fileName = Just fileName -- Если найден файл с нужным именем
    | otherwise          = Nothing       -- Иначе ничего не возвращаем
find target (Directory dirName contents) =
    let paths = map (\item -> fmap (dirName ++) (find target item)) contents
    in asum paths -- Объединяем все Maybe в один результат

-- Функция du: вычислить общий размер файлов в каталоге
du :: FileSystem -> Int
du (File _ size) = size -- Для файла возвращаем его размер
du (Directory _ contents) = sum (map du contents) -- Для каталога суммируем размеры всех элементов

-- Пример файловой системы
exampleFS :: FileSystem
exampleFS =
    Directory "root" [
        File "file1.txt" 100,
        Directory "docs" [
            File "doc1.txt" 200,
            File "doc2.txt" 150
        ],
        Directory "images" [
            File "image1.png" 500,
            Directory "thumbnails" [
                File "thumb1.jpg" 50,
                File "thumb2.jpg" 60
            ]
        ]
    ]

-- Тестирование функций
main :: IO ()
main = do
    putStrLn "Список всех файлов:"
    print (dirAll exampleFS)

    putStrLn "\nПоиск файла 'doc1.txt':"
    print (find "doc1.txt" exampleFS)

    putStrLn "\nОбщий размер файлов в каталоге:"
    print (du exampleFS)