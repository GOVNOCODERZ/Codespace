arsr::[Double] -> Double
arsr num = sum num / fromIntegral (length num)

geosr::[Double] -> Double
geosr num = product num ** (1 / fromIntegral (length num))

main::IO ()
main = do
    let num = [1.15, 2.34, 3.65, 4.51, 5.41]
    putStrLn $ "Арифметическое среднее: " ++ show (arsr num)
    putStrLn $ "Геометрическое среднее: " ++ show (geosr num)