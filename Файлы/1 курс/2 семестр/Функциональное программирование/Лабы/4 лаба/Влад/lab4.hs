data SimpleType = IntType | DoubleType | StringType
  deriving (Eq, Show)
  
data Field = Field { fieldName :: String, fieldType :: SimpleType }
  deriving (Eq, Show)

data StructType = StructType { structName :: String, fields :: [Field] }
  deriving (Eq, Show)

data Type = Simple SimpleType | Struct StructType
  deriving (Eq, Show)

type Database = [(String, Type)]


isStructured :: Type -> Bool
isStructured (Struct _) = True
isStructured _          = False

getType :: String -> Database -> Maybe Type
getType = lookup

getFields :: String -> Database -> Maybe [Field]
getFields name db =
  case lookup name db of
    Just (Struct (StructType _ fs)) -> Just fs
    _                              -> Nothing

getByType :: SimpleType -> Database -> [String]
getByType t db =
  [ name | (name, typ) <- db, isSimpleType t typ ]
  where
    isSimpleType :: SimpleType -> Type -> Bool
    isSimpleType t' (Simple s) = t' == s
    isSimpleType _ _           = False

getByTypes :: [SimpleType] -> Database -> [String]
getByTypes types db =
  [ name | (name, typ) <- db, any (`isSimpleType` typ) types ]
  where
    isSimpleType :: SimpleType -> Type -> Bool
    isSimpleType t' (Simple s) = t' == s
    isSimpleType _ _           = False


exampleDatabase :: Database
exampleDatabase =
  [ ("age", Simple IntType)
  , ("height", Simple DoubleType)
  , ("name", Simple StringType)
  , ("person", Struct (StructType "Person" [
      Field "name" StringType,
      Field "age" IntType,
      Field "height" DoubleType
    ]))
  ]


main :: IO ()
main = do
  putStrLn "Пример работы функций:"

  putStrLn "Является ли тип 'IntType' сложным типом:"
  print $ isStructured (Simple IntType)
  putStrLn "Является ли тип 'StructType' сложным типом:"
  print $ isStructured (Struct (StructType "Person" []))

  putStrLn "Поиск типа идентификатора по заданному имени 'age':"
  print $ getType "age" exampleDatabase
  putStrLn "Поиск типа идентификатора по заданному имени 'nonexistent':"
  print $ getType "nonexistent" exampleDatabase

  putStrLn "Вывод полей идентификатора структурного типа по имени 'person':"
  print $ getFields "person" exampleDatabase
  putStrLn "Вывод полей идентификатора структурного типа по имени 'age':"
  print $ getFields "age" exampleDatabase

  putStrLn "Вывод списка имён идентификаторов по типу 'IntType':"
  print $ getByType IntType exampleDatabase
  putStrLn "Вывод списка имён идентификаторов по типу 'StringType':"
  print $ getByType StringType exampleDatabase

  putStrLn "Вывод списка имён идентификаторов по числовым типам 'IntType, DoubleType':"
  print $ getByTypes [IntType, DoubleType] exampleDatabase
  putStrLn "Вывод списка имён идентификаторов по типам 'StringType':"
  print $ getByTypes [StringType] exampleDatabase