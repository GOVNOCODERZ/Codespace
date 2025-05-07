import System.Environment (getArgs)
import System.IO

main :: IO ()
main = do
    args <- getArgs
    if null args
        then putStrLn "Имя файла не указано!"
        else do
            let fileName = head args
            contents <- readFile fileName
            putStrLn $ "Содержимое файла '" ++ fileName ++ "':"
            putStr contents