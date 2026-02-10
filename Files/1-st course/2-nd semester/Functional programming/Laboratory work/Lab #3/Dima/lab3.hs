f1 :: [Double] -> Double
f1 xs = (foldr (*) 1 xs) ** (1/ fromIntegral (length xs))

f2 :: [Double] -> [Double] -> Double
f2 a b = foldr (+) 0 (zipWith (*) a b)

f3 :: [Int] -> Int
f3 xs = length [x | x <- xs, x < 0]

f4 :: (Ord a) => [a] -> [a]
f4 [] = []
f4 (x:xs) = f4 [a | a <- xs, a <= x] ++ [x] ++ f4 [b | b <- xs, b > x]

f5 :: (a -> a -> Bool) -> [a] -> [a]
f5 _ [] = []
f5 cmp(x:xs) = 
    let as = f5 cmp [a | a <- xs, cmp a x]
        bs = f5 cmp [b | b <- xs, not (cmp b x)]
    in as ++ [x] ++ bs

main :: IO()
main = do
    putStrLn "Результаты:"
    print(f1 [1.23, 3.45, 6.122, 5.214])
    print(f2 [1.23, 2.51, 5.215, 2.66, 1.25] [2.22, 3.33, 4.44, 5.55, 6.66])
    print(f3 [-1,2,-3,4,-5])
    print(f4 [9, 1, 5, 7, 2, 4, 3, 6, 8])
    print(f5 (>) [9, 1, 4, 2, 8, 3])