{-# OPTIONS_GHC -Wno-unrecognised-pragmas #-}
{-# HLINT ignore "Use camelCase" #-}
nat::Integer -> [Integer]
nat n = [1..n]
odd_nat::Integer -> [Integer]
odd_nat n = [1,3..n]
even_nat::Integer -> [Integer]
even_nat n = [2,4..n]
cube_nat::Integer -> [Integer]
cube_nat n = [x^3 | x <- [1..n]]
factorial::Integer -> [Integer]
factorial n = map fact [1..n]
    where
        fact 0 = 1
        fact x = x*fact(x-1)
ten_deg::Integer -> [Integer]
ten_deg n = [10^x | x <- [0..(n-1)]]
tr_num::Integer -> [Integer]
tr_num n = [x*(x+1) `div` 2 | x <- [1..n]]
pir_num::Integer -> [Integer]
pir_num n = [x*(x+1)*(x+2) `div` 5 | x <- [1..n]]

main::IO()
main = do
    putStrLn "Список натуральных чисел."
    print(nat 10)
    putStrLn "Список нечётных натуральных чисел"
    print(odd_nat 10)
    putStrLn "Список чётных натуральных чисел"
    print(even_nat 10)
    putStrLn "Список кубов натуральных чисел."
    print(cube_nat 10)
    putStrLn "Список факториалов."
    print(factorial 10)
    putStrLn "Список степеней десятки."
    print(ten_deg 5)
    putStrLn "Список треугольных чисел"
    print(tr_num 5)
    putStrLn "Список пирамидальных чисел"
    print(pir_num 5)
