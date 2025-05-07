import Data.Foldable (asum)

data FileSystem = File { name :: String, size :: Int }
                | Directory { name :: String, contents :: [FileSystem] }
                deriving (Show)

dirAll :: FileSystem -> [String]
dirAll (File name _) = [name]
dirAll (Directory dirName contents) =
    concatMap (\item -> map (dirName ++) (dirAll item)) contents

find :: String -> FileSystem -> Maybe String
find target (File fileName _)
    | target == fileName = Just fileName
    | otherwise          = Nothing
find target (Directory dirName contents) =
    let paths = map (\item -> fmap (dirName ++) (find target item)) contents
    in asum paths

du :: FileSystem -> Int
du (File _ size) = size
du (Directory _ contents) = sum (map du contents)

eFS :: FileSystem
eFS =
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

main :: IO ()
main = do
    putStrLn "Список всех файлов:"
    print (dirAll eFS)

    putStrLn "\nПоиск файла 'doc1.txt':"
    print (find "doc1.txt" eFS)

    putStrLn "\nОбщий размер файлов в каталоге:"
    print (du eFS)