trHeight :: Double -> Double -> Double
trHeight a b = sqrt(a ^ 2 - (b / 2) ^ 2)

main :: IO ()
main = do
    let a = 5.0
    let b = 8.0
    putStrLn ("Боковая сторона треугольника " ++ show a)
    putStrLn ("Основание треугольника " ++ show b)
    let height = trHeight a b
    putStrLn ("Отсюда следует, что высота равнобедренного треугольника равна " ++ show height)
