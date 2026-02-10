import Control.Monad (replicateM)
import Data.Foldable (foldl')

upo::Integer -> [Integer]
upo n = [1..n]
nat::Integer -> [Integer]
nat n = [1..n]
nnat::Integer -> [Integer]
nnat n = [1,3..n]
chnat::Integer -> [Integer]
chnat n = [2,4..n]
cubenat::Integer -> [Integer]
cubenat n = [x^3 | x <- [1..n]]
factor::Integer -> [Integer]
factor n = map fact [1..n]
    where
        fact 0 = 1
        fact x = x*fact(x-1)
tenstep::Integer -> [Integer]
tenstep n = [10^x | x <- [0..(n-1)]]
tr::Integer -> [Integer]
tr n = [x*(x+1) `div` 2 | x <- [1..n]]
pir::Integer -> [Integer]
pir n = [x*(x+1)*(x+2) `div` 6 | x <- [1..n]]

main::IO()
main = do
    putStrLn "Упорядоченный список"
    print(upo 15)
    putStrLn "Список натуральных чисел."
    print(nat 15)
    putStrLn "Список нечетных натуральных чисел"
    print(nnat 15)
    putStrLn "Список четных натуральных чисел"
    print(chnat 15)
    putStrLn "Список кубов натуральных чисел."
    print(cubenat 15)
    putStrLn "Список факториалов."
    print(factor 15)
    putStrLn "Список степеней десятки."
    print(tenstep 6)
    putStrLn "Список треугольных чисел"
    print(tr 6)
    putStrLn "Список пирамидальных чисел"
    print(pir 6)
