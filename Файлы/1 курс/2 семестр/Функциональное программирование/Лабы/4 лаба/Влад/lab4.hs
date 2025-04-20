--ОБЪЯВЛЕНИЕ ТИПОВ ДАННЫХ
data SimpleType = IntType | DoubleType | StringType
  deriving (Eq, Show)

data Field = Field { fieldName :: String, fieldType :: SimpleType }
  deriving (Eq, Show)

data StructType = StructType { structName :: String, fields :: [Field] }
  deriving (Eq, Show)

data Type = Simple SimpleType | Struct StructType
  deriving (Eq, Show)

type Database = [(String, Type)]


--ОБЪЯВЛЕНИЕ ФУНКЦИЙ
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


--ОБЪЯВЛЕНИЕ БАЗЫ ДАННЫХ ДЛЯ ТЕСТА
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


--ПРОВЕРКА РАБОТЫ ФУНКЦИЙ
main :: IO ()
main = do
  putStrLn "Пример работы функций:"

  print $ isStructured (Simple IntType)
  print $ isStructured (Struct (StructType "Person" []))

  print $ getType "age" exampleDatabase
  print $ getType "nonexistent" exampleDatabase

  print $ getFields "person" exampleDatabase
  print $ getFields "age" exampleDatabase

  print $ getByType IntType exampleDatabase
  print $ getByType StringType exampleDatabase

  print $ getByTypes [IntType, StringType] exampleDatabase
  print $ getByTypes [DoubleType] exampleDatabase