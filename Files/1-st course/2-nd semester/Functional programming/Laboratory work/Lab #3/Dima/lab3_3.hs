arsr :: [Double] -> Double
arsr xs = foldr (+) 0 xs / fromIntegral (length xs)

geosr :: [Double] -> Double
geosr xs = product xs ** (1 / fromIntegral (length xs))

main :: IO ()
main = do
    let num = [1.15, 2.34, 3.65, 4.51, 5.41]
    putStrLn $ "Арифметическое среднее: " ++ show (arsr num)
    putStrLn $ "Геометрическое среднее: " ++ show (geosr num)