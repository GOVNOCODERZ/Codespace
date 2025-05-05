-- Программа для вывода первых n строк файла
import System.Environment (getArgs)
import System.IO

main :: IO ()
main = do
    args <- getArgs
    if length args < 2
        then putStrLn "Использование: program <имя_файла> <количество_строк>"
        else do
            let fileName = args !! 0
                n = read (args !! 1) :: Int
            contents <- readFile fileName
            let linesOfFile = take n (lines contents)
            putStrLn $ "Первые " ++ show n ++ " строк файла '" ++ fileName ++ "':"
            mapM_ putStrLn linesOfFile