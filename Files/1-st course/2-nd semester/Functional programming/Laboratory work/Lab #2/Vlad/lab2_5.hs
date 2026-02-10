subList :: [a] -> Int -> Int -> [a]
subList list n m
    | n > m = []
    | n < 0 || m >= length list = []
    | otherwise = take (m - n + 1) (drop n list)


main :: IO ()
main = do
    let myList = [1, 2, 3, 4, 5, 6, 7, 8, 9]
    putStrLn "Исходный список:"
    print myList

    putStrLn "Подсписок от индекса 2 до 5:"
    print (subList myList 2 5)  

    putStrLn "Подсписок от индекса 1 до 7:"
    print (subList myList 1 7)

    putStrLn "Пустой подсписок при некорректных индексах (n > m, например):"
    print (subList myList 5 2)

    putStrLn "Пустой подсписок при индексах вне диапазона (n < 0, например):"
    print (subList myList (-1) 10)