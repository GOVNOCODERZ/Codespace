data Trie = Trie { char     :: Char
                 , isEnd    :: Bool
                 , children :: [Trie]
                 } deriving (Show, Eq)


exists :: String -> Trie -> Bool
exists []     (Trie _ end _) = end
exists (x:xs) (Trie _ _ children) =
    case findChild x children of
        Nothing -> False
        Just child -> exists xs child
    where
        findChild :: Char -> [Trie] -> Maybe Trie
        findChild c [] = Nothing
        findChild c (t:ts)
            | char t == c = Just t
            | otherwise   = findChild c ts

insert :: String -> Trie -> Trie
insert []     (Trie c end children) = Trie c True children
insert (x:xs) (Trie c end children) =
    case findChild x children of
        Nothing -> Trie c end (newChild : children)
        Just child -> Trie c end (replaceChild (insert xs child) children)
    where
        newChild = insert xs (Trie x False [])
        findChild :: Char -> [Trie] -> Maybe Trie
        findChild c [] = Nothing
        findChild c (t:ts)
            | char t == c = Just t
            | otherwise   = findChild c ts
        replaceChild :: Trie -> [Trie] -> [Trie]
        replaceChild _ [] = []
        replaceChild newChild (t:ts)
            | char t == char newChild = newChild : ts
            | otherwise             = t : replaceChild newChild ts

completions :: String -> Trie -> [String]
completions prefix trie =
    case go prefix trie of
        Nothing -> []
        Just subtrie -> map (prefix ++) (allWords subtrie)
    where
        go :: String -> Trie -> Maybe Trie
        go []     t = Just t
        go (x:xs) (Trie _ _ children) =
            case findChild x children of
                Nothing -> Nothing
                Just child -> go xs child
            where
                findChild :: Char -> [Trie] -> Maybe Trie
                findChild c [] = Nothing
                findChild c (t:ts)
                    | char t == c = Just t
                    | otherwise   = findChild c ts

        allWords :: Trie -> [String]
        allWords (Trie c end children) =
            let current = (["" | end])
                childWords = concatMap (\child -> map (char child :) (allWords child)) children
            in current ++ childWords


exampleTrie :: Trie
exampleTrie = foldr insert emptyTrie ["fa", "false", "far", "fare", "fact", "fried", "frieze"]
    where emptyTrie = Trie '\0' False []


main :: IO ()
main = do
  print $ exists "fa" exampleTrie -- True
  print $ exists "far" exampleTrie -- True
  print $ exists "fare" exampleTrie -- True
  print $ exists "fact" exampleTrie -- True
  print $ exists "fried" exampleTrie -- True
  print $ exists "frieze" exampleTrie -- True

  print $ exists "fac" exampleTrie -- False
  print $ exists "nothing" exampleTrie -- False

  print $ completions "fri" exampleTrie -- ["fried","frieze"]
  print $ completions "fa" exampleTrie -- ["fa","false","far","fare","fact"]