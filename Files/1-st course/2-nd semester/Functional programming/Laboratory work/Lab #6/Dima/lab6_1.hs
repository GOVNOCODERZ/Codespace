main :: IO ()
main = do
    putStrLn "Введите первое число:"
    input1 <- getLine
    putStrLn "Введите второе число:"
    input2 <- getLine
    let num1 = read input1 :: Int
        num2 = read input2 :: Int
    putStrLn $ "Сумма: " ++ show (num1 + num2)