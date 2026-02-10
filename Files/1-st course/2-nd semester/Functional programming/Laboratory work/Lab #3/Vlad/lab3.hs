geoMean :: [Double] -> Double
geoMean xs
    | null xs    = error "Список пуст!"
    | otherwise  = product' ** (1.0 / fromIntegral count)
    where
        (product', count) = foldr (\x (p, c) -> (x * p, c + 1)) (1, 0) xs

scalarProduct :: Num a => [a] -> [a] -> a
scalarProduct list1 list2 =
    foldr (+) 0 (zipWith (*) list1 list2)

countNegat :: [Int] -> Int
countNegat = foldr (\x acc -> if x < 0 then acc + 1 else acc) 0

quicksort :: (Ord a) => [a] -> [a]
quicksort [] = []
quicksort (x:xs) =
    let
        as = [a | a <- xs, a <= x]
        bs = [b | b <- xs, b > x]
    in
        quicksort as ++ [x] ++ quicksort bs

genericQuicksort :: (a -> a -> Bool) -> [a] -> [a]
genericQuicksort _ [] = []
genericQuicksort compareFunc (x:xs) =
    let
        as = [a | a <- xs, compareFunc a x]
        bs = [b | b <- xs, not (compareFunc b x)]
    in
        genericQuicksort compareFunc as ++ [x] ++ genericQuicksort compareFunc bs




main :: IO ()
main = do
    putStrLn "Геометрическое среднее списка вещественных чисел [2.0, 4.0, 8.0]: "
    print (geoMean [2.0, 4.0, 8.0])

    putStrLn "Скалярное произведение двух списков [2.0, 4.0, 8.0] [3.0, 5.0, 9.0]: "
    print (scalarProduct [2.0, 4.0, 8.0] [3.0, 5.0, 9.0])

    putStrLn "Количество отрицательных элементов в списке [3, -1, -4, 2, 0, -5]: "
    print(countNegat [3, -1, -4, 2, 0, -5])

    putStrLn "Быстрая сортировка списка [3, -1, -4, 2, 0, -5]: "
    print (quicksort [3, 1, 4, 1, 5, 9])

    putStrLn "Обобщённая быстрая сортировка списка [3, -1, -4, 2, 0, -5] (по убыванию): "
    print (genericQuicksort (>=) [3, 1, 4, 1, 5, 9])