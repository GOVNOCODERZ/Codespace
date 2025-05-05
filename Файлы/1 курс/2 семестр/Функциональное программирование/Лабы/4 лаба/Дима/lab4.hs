data RealEstate = Apartment 
    { floor :: Int       -- этаж квартиры
    , totalArea :: Float -- общая площадь квартиры
    , buildingFloors :: Int -- этажность дома
    }
    | Room 
    { floor :: Int       -- этаж квартиры
    , totalArea :: Float -- общая площадь квартиры
    , buildingFloors :: Int -- этажность дома
    , roomArea :: Float  -- площадь комнаты
    }
    | House 
    { area :: Float      -- площадь дома
    }
    deriving (Show, Eq)

-- Тип для базы данных: список пар (объект недвижимости, цена)
type Database = [(RealEstate, Float)]

-- Тип для требований к недвижимости
data Requirement = 
      TypeReq RealEstateType  -- желаемый тип недвижимости
    | MinArea Float           -- минимальная площадь
    | MaxPrice Float          -- максимальная цена
    | FloorConstraint FloorCondition -- ограничения на этаж
    deriving (Show, Eq)

data RealEstateType = AnyType | ApartmentType | RoomType | HouseType
    deriving (Show, Eq)

data FloorCondition = 
      AnyFloor                   -- любой этаж
    | ExactFloor Int             -- конкретный этаж
    | NotExtremeFloors           -- не крайние этажи
    deriving (Show, Eq)


-- 1) Выбирает только частные дома
getHouses :: Database -> Database
getHouses db = filter (\(estate, _) -> case estate of House _ -> True; _ -> False) db

-- 2) Выбирает объекты с ценой меньше указанной
getByPrice :: Float -> Database -> Database
getByPrice maxPrice db = filter (\(_, price) -> price < maxPrice) db

-- 3) Выбирает квартиры на указанном этаже
getByLevel :: Int -> Database -> Database
getByLevel level db = filter (\(estate, _) -> 
    case estate of 
        Apartment fl _ _ -> fl == level
        Room fl _ _ _ -> fl == level
        _ -> False) db

-- 4) Выбирает квартиры не на крайних этажах
getExceptBounds :: Database -> Database
getExceptBounds db = filter (\(estate, _) -> 
    case estate of 
        Apartment fl _ totalFloors -> fl > 1 && fl < totalFloors
        Room fl _ totalFloors _ -> fl > 1 && fl < totalFloors
        _ -> False) db


-- Проверяет, удовлетворяет ли объект недвижимости и его цена одному требованию
satisfiesRequirement :: (RealEstate, Float) -> Requirement -> Bool
satisfiesRequirement (estate, price) req = case req of
    TypeReq t -> case t of
        AnyType -> True
        ApartmentType -> case estate of Apartment _ _ _ -> True; _ -> False
        RoomType -> case estate of Room _ _ _ _ -> True; _ -> False
        HouseType -> case estate of House _ -> True; _ -> False
    MinArea minA -> case estate of
        Apartment _ area _ -> area >= minA
        Room _ area _ roomA -> area >= minA || roomA >= minA
        House area -> area >= minA
    MaxPrice maxP -> price <= maxP
    FloorConstraint fc -> case fc of
        AnyFloor -> True
        ExactFloor lvl -> case estate of
            Apartment fl _ _ -> fl == lvl
            Room fl _ _ _ -> fl == lvl
            House _ -> False
        NotExtremeFloors -> case estate of
            Apartment fl _ totalFloors -> fl > 1 && fl < totalFloors
            Room fl _ totalFloors _ -> fl > 1 && fl < totalFloors
            House _ -> False

-- Основная функция запроса
query :: [Requirement] -> Database -> Database
query reqs db = filter (\item -> all (satisfiesRequirement item) reqs) db

-- Пример базы данных
sampleDB :: Database
sampleDB = 
    [ (Apartment 2 50.0 5, 200000)
    , (Apartment 1 40.0 5, 180000)
    , (Apartment 5 60.0 5, 220000)
    , (Room 3 70.0 5 15.0, 150000)
    , (House 120.0, 350000)
    ]

-- Пример запроса: квартиры не на крайних этажах с ценой до 210000
exampleQuery :: Database
exampleQuery = query [TypeReq ApartmentType, MaxPrice 210000, FloorConstraint NotExtremeFloors] sampleDB

main :: IO ()
main = do
    putStrLn "Результаты запроса:"
    print exampleQuery