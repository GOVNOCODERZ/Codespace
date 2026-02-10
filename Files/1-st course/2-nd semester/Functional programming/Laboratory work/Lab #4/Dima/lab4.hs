data RealEstate = Apartment 
    { floor :: Int
    , totalArea :: Float
    , buildingFloors :: Int
    }
    | Room 
    { floor :: Int
    , totalArea :: Float
    , buildingFloors :: Int
    , roomArea :: Float
    }
    | House 
    { area :: Float
    }
    deriving (Show, Eq)

type Database = [(RealEstate, Float)]

data Requirement = 
      TypeReq RealEstateType
    | MinArea Float
    | MaxPrice Float
    | FloorConstraint FloorCondition
    deriving (Show, Eq)

data RealEstateType = AnyType | ApartmentType | RoomType | HouseType
    deriving (Show, Eq)

data FloorCondition = 
      AnyFloor
    | ExactFloor Int
    | NotExtremeFloors
    deriving (Show, Eq)


getHouses :: Database -> Database
getHouses db = filter (\(estate, _) -> case estate of House _ -> True; _ -> False) db

getByPrice :: Float -> Database -> Database
getByPrice maxPrice db = filter (\(_, price) -> price < maxPrice) db

getByLevel :: Int -> Database -> Database
getByLevel level db = filter (\(estate, _) -> 
    case estate of 
        Apartment fl _ _ -> fl == level
        Room fl _ _ _ -> fl == level
        _ -> False) db

getExceptBounds :: Database -> Database
getExceptBounds db = filter (\(estate, _) -> 
    case estate of 
        Apartment fl _ totalFloors -> fl > 1 && fl < totalFloors
        Room fl _ totalFloors _ -> fl > 1 && fl < totalFloors
        _ -> False) db


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

query :: [Requirement] -> Database -> Database
query reqs db = filter (\item -> all (satisfiesRequirement item) reqs) db

aBD :: Database
aBD = 
    [ (Apartment 2 50.0 5, 200000)
    , (Apartment 1 40.0 5, 180000)
    , (Apartment 5 60.0 5, 220000)
    , (Room 3 70.0 5 15.0, 150000)
    , (House 120.0, 350000)
    ]

eQuery :: Database
eQuery = query [TypeReq ApartmentType, MaxPrice 210000, FloorConstraint NotExtremeFloors] aBD

main :: IO ()
main = do
    putStrLn "Результаты запроса:"
    print eQuery