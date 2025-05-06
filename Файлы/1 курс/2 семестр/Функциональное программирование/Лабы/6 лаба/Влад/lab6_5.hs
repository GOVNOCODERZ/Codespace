trHeight :: Double -> Double -> Double
trHeight a b = sqrt(a ^ 2 - (b / 2) ^ 2)

main :: IO ()
main = do
    putStrLn "Введите первое число:"
    input1 <- getLine
    putStrLn "Введите второе число:"
    input2 <- getLine
    let a = read input1 :: Double
    let b = read input2 :: Double
    putStrLn ("Боковая сторона треугольника " ++ show a)
    putStrLn ("Основание треугольника " ++ show b)
    let height = trHeight a b
    putStrLn ("Отсюда следует, что высота равнобедренного треугольника равна " ++ show height)
