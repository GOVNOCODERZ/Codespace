-- Программа для вывода аргументов командной строки
import System.Environment (getArgs)

main :: IO ()
main = do
    args <- getArgs
    putStrLn "Аргументы командной строки:"
    mapM_ putStrLn args