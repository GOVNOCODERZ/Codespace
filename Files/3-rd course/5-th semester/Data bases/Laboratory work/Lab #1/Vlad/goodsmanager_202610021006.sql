--
-- Скрипт сгенерирован Devart dbForge Studio 2020 for MySQL, Версия 9.0.567.0
-- Домашняя страница продукта: http://www.devart.com/ru/dbforge/mysql/studio
-- Дата скрипта: 02.10.2026 10:06:56
-- Версия сервера: 5.7.11
-- Версия клиента: 4.1
--

-- 
-- Отключение внешних ключей
-- 
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;

-- 
-- Установить режим SQL (SQL mode)
-- 
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;

-- 
-- Установка кодировки, с использованием которой клиент будет посылать запросы на сервер
--
SET NAMES 'utf8';

--
-- Установка базы данных по умолчанию
--
USE goodsmanager;

--
-- Удалить таблицу `op_goods_list`
--
DROP TABLE IF EXISTS op_goods_list;

--
-- Удалить представление `view_goods_operations`
--
DROP VIEW IF EXISTS view_goods_operations CASCADE;

--
-- Удалить таблицу `operations`
--
DROP TABLE IF EXISTS operations;

--
-- Удалить таблицу `suppliers_catalog`
--
DROP TABLE IF EXISTS suppliers_catalog;

--
-- Удалить представление `view_storage`
--
DROP VIEW IF EXISTS view_storage CASCADE;

--
-- Удалить таблицу `storage`
--
DROP TABLE IF EXISTS storage;

--
-- Удалить представление `view_goods_catalog`
--
DROP VIEW IF EXISTS view_goods_catalog CASCADE;

--
-- Удалить таблицу `goods_catalog`
--
DROP TABLE IF EXISTS goods_catalog;

--
-- Удалить таблицу `type_goods`
--
DROP TABLE IF EXISTS type_goods;

--
-- Установка базы данных по умолчанию
--
USE goodsmanager;

--
-- Создать таблицу `type_goods`
--
CREATE TABLE type_goods (
  IDtg int(11) NOT NULL AUTO_INCREMENT COMMENT 'Код типа товара',
  tgName varchar(50) NOT NULL COMMENT 'Наименование типа товара',
  PRIMARY KEY (IDtg)
)
ENGINE = INNODB,
AUTO_INCREMENT = 4,
AVG_ROW_LENGTH = 5461,
CHARACTER SET utf8mb4,
COLLATE utf8mb4_general_ci,
COMMENT = 'Перечень возможных типов товаров';

--
-- Создать индекс `UK_type_goods_tgName` для объекта типа таблица `type_goods`
--
ALTER TABLE type_goods
ADD UNIQUE INDEX UK_type_goods_tgName (tgName);

--
-- Создать таблицу `goods_catalog`
--
CREATE TABLE goods_catalog (
  IDgc int(11) NOT NULL AUTO_INCREMENT COMMENT 'Код товара',
  gcName varchar(100) NOT NULL COMMENT 'Наименование товара',
  IDtg int(11) NOT NULL COMMENT 'Код типа товара',
  gcDescription varchar(255) DEFAULT NULL COMMENT 'Описание товара',
  gcPhoto longblob DEFAULT NULL COMMENT 'Фотография товара',
  gcCost decimal(10, 2) DEFAULT 1.00 COMMENT 'Стоимость единицы товара',
  PRIMARY KEY (IDgc)
)
ENGINE = INNODB,
AUTO_INCREMENT = 7,
AVG_ROW_LENGTH = 3276,
CHARACTER SET utf8mb4,
COLLATE utf8mb4_general_ci,
COMMENT = 'Перечень товаров';

--
-- Создать индекс `IDX_goods_catalog` для объекта типа таблица `goods_catalog`
--
ALTER TABLE goods_catalog
ADD INDEX IDX_goods_catalog (IDtg, gcName);

--
-- Создать индекс `UK_goods_catalog_gcName` для объекта типа таблица `goods_catalog`
--
ALTER TABLE goods_catalog
ADD UNIQUE INDEX UK_goods_catalog_gcName (gcName);

--
-- Создать внешний ключ
--
ALTER TABLE goods_catalog
ADD CONSTRAINT FK_goods_catalog_IDtg FOREIGN KEY (IDtg)
REFERENCES type_goods (IDtg) ON DELETE NO ACTION;

--
-- Создать представление `view_goods_catalog`
--
CREATE
DEFINER = 'root'@'localhost'
VIEW view_goods_catalog
AS
SELECT
  `goods_catalog`.`IDgc` AS `IDgc`,
  `goods_catalog`.`gcName` AS `gcName`,
  `type_goods`.`tgName` AS `tgName`,
  `goods_catalog`.`gcDescription` AS `gcDescription`,
  `goods_catalog`.`gcPhoto` AS `gcPhoto`,
  `goods_catalog`.`gcCost` AS `gcCost`
FROM (`goods_catalog`
  JOIN `type_goods`
    ON ((`goods_catalog`.`IDtg` = `type_goods`.`IDtg`)));

--
-- Создать таблицу `storage`
--
CREATE TABLE storage (
  IDstorage int(11) NOT NULL AUTO_INCREMENT COMMENT 'Код складской записи',
  IDgc int(11) NOT NULL COMMENT 'Код товара',
  sCount int(11) NOT NULL DEFAULT 0 COMMENT 'Количество товара',
  PRIMARY KEY (IDstorage)
)
ENGINE = INNODB,
AUTO_INCREMENT = 6,
AVG_ROW_LENGTH = 3276,
CHARACTER SET utf8mb4,
COLLATE utf8mb4_general_ci,
COMMENT = 'Складские запасы';

--
-- Создать внешний ключ
--
ALTER TABLE storage
ADD CONSTRAINT FK_storage_IDgc FOREIGN KEY (IDgc)
REFERENCES goods_catalog (IDgc) ON DELETE NO ACTION;

--
-- Создать представление `view_storage`
--
CREATE
DEFINER = 'root'@'localhost'
VIEW view_storage
AS
SELECT
  `storage`.`IDstorage` AS `IDstorage`,
  `storage`.`IDgc` AS `IDgc`,
  `storage`.`sCount` AS `sCount`,
  `goods_catalog`.`gcName` AS `gcName`,
  `goods_catalog`.`IDtg` AS `IDtg`,
  `type_goods`.`tgName` AS `tgName`
FROM ((`storage`
  JOIN `goods_catalog`
    ON ((`storage`.`IDgc` = `goods_catalog`.`IDgc`)))
  JOIN `type_goods`
    ON ((`goods_catalog`.`IDtg` = `type_goods`.`IDtg`)));

--
-- Создать таблицу `suppliers_catalog`
--
CREATE TABLE suppliers_catalog (
  IDsc int(11) NOT NULL AUTO_INCREMENT COMMENT 'Код поставщика',
  scName varchar(50) NOT NULL COMMENT 'Название поставщика',
  scAddress varchar(100) DEFAULT NULL COMMENT 'Адрес поставщика',
  scPhone varchar(15) DEFAULT NULL COMMENT 'Телефон поставщика',
  scEmail varchar(50) DEFAULT NULL COMMENT 'e-mail поставщика',
  PRIMARY KEY (IDsc)
)
ENGINE = INNODB,
AUTO_INCREMENT = 3,
AVG_ROW_LENGTH = 8192,
CHARACTER SET utf8mb4,
COLLATE utf8mb4_general_ci,
COMMENT = 'Перечень поставщиков';

--
-- Создать индекс `UK_suppliers_catalog_scEmail` для объекта типа таблица `suppliers_catalog`
--
ALTER TABLE suppliers_catalog
ADD UNIQUE INDEX UK_suppliers_catalog_scEmail (scEmail);

--
-- Создать индекс `UK_suppliers_catalog_scName` для объекта типа таблица `suppliers_catalog`
--
ALTER TABLE suppliers_catalog
ADD UNIQUE INDEX UK_suppliers_catalog_scName (scName);

--
-- Создать таблицу `operations`
--
CREATE TABLE operations (
  IDoperation int(11) NOT NULL AUTO_INCREMENT COMMENT 'Код операции',
  oDateTime datetime NOT NULL COMMENT 'Дата операции',
  oIsSale tinyint(1) DEFAULT 1 COMMENT 'Тип операции (1 - продажа, 0 - поставка)',
  IDsc int(11) DEFAULT NULL COMMENT 'Клд поставщика',
  PRIMARY KEY (IDoperation)
)
ENGINE = INNODB,
AUTO_INCREMENT = 4,
AVG_ROW_LENGTH = 5461,
CHARACTER SET utf8mb4,
COLLATE utf8mb4_general_ci,
COMMENT = 'Операции с товарами (приход/продажа)';

--
-- Создать индекс `IDX_operations_oDateTime` для объекта типа таблица `operations`
--
ALTER TABLE operations
ADD INDEX IDX_operations_oDateTime (oDateTime);

--
-- Создать внешний ключ
--
ALTER TABLE operations
ADD CONSTRAINT FK_operations_IDsc FOREIGN KEY (IDsc)
REFERENCES suppliers_catalog (IDsc) ON DELETE NO ACTION;

--
-- Создать представление `view_goods_operations`
--
CREATE
DEFINER = 'root'@'localhost'
VIEW view_goods_operations
AS
SELECT
  `operations`.`IDoperation` AS `IDoperation`,
  `operations`.`oDateTime` AS `oDateTime`,
  `operations`.`oIsSale` AS `oIsSale`,
  `suppliers_catalog`.`scName` AS `scName`
FROM (`operations`
  LEFT JOIN `suppliers_catalog`
    ON ((`operations`.`IDsc` = `suppliers_catalog`.`IDsc`)));

--
-- Создать таблицу `op_goods_list`
--
CREATE TABLE op_goods_list (
  IDoperation int(11) NOT NULL COMMENT 'Код операции',
  IDgc int(11) NOT NULL COMMENT 'Код товара',
  ocCount int(11) NOT NULL DEFAULT 1 COMMENT 'Количество товара',
  PRIMARY KEY (IDgc, IDoperation)
)
ENGINE = INNODB,
CHARACTER SET utf8mb4,
COLLATE utf8mb4_general_ci,
COMMENT = 'Состав товаров в операции';

--
-- Создать внешний ключ
--
ALTER TABLE op_goods_list
ADD CONSTRAINT FK_op_goods_list_IDgc FOREIGN KEY (IDgc)
REFERENCES goods_catalog (IDgc) ON DELETE NO ACTION;

--
-- Создать внешний ключ
--
ALTER TABLE op_goods_list
ADD CONSTRAINT FK_op_goods_list_IDoperation FOREIGN KEY (IDoperation)
REFERENCES operations (IDoperation) ON DELETE NO ACTION;

-- 
-- Вывод данных для таблицы suppliers_catalog
--
INSERT INTO suppliers_catalog VALUES
(1, 'Техномаркет', 'Адрес #1', 'Телефон #1', 'info@tehnomark.ru'),
(2, 'КрасТрейд', 'Адрес #2', 'Телефон #2', 'salse@krastrade.com');

-- 
-- Вывод данных для таблицы type_goods
--
INSERT INTO type_goods VALUES
(3, 'Мониторы'),
(1, 'Ноутбуки\r\n'),
(2, 'Системные блоки');

-- 
-- Вывод данных для таблицы operations
--
INSERT INTO operations VALUES
(1, '2026-10-01 12:00:00', 1, NULL),
(2, '2026-10-01 16:30:00', 1, NULL),
(3, '2026-10-02 14:16:30', 0, 1);

-- 
-- Вывод данных для таблицы goods_catalog
--
INSERT INTO goods_catalog VALUES
(1, 'Acer Aspire 1 A115-32-C2Z1\r\n', 1, 'Хар-ки #1', NULL, 25499.00),
(2, 'Lenovo Ideapad 3 15IGL05', 1, 'Хар-ки #2', NULL, 27955.00),
(3, 'DEXP Mars E317', 2, 'Хар-ки #3', NULL, 41999.00),
(4, 'ASUS ROG Stix GL10CS-RU077T', 2, 'Хар-ки #4', NULL, 64990.00),
(5, 'Samsung F27T450FQI', 3, 'Хар-ки #5', NULL, 15199.00);

-- 
-- Вывод данных для таблицы storage
--
INSERT INTO storage VALUES
(1, 1, 0),
(2, 2, 0),
(3, 3, 0),
(4, 4, 0),
(5, 5, 0);

-- 
-- Вывод данных для таблицы op_goods_list
--
-- Таблица goodsmanager.op_goods_list не содержит данных

--
-- Установка базы данных по умолчанию
--
USE goodsmanager;

--
-- Удалить триггер `tr_add_goods_in_catalog`
--
DROP TRIGGER IF EXISTS tr_add_goods_in_catalog;

--
-- Удалить триггер `tr_remove_goods_from_catalogs`
--
DROP TRIGGER IF EXISTS tr_remove_goods_from_catalogs;

--
-- Установка базы данных по умолчанию
--
USE goodsmanager;

DELIMITER $$

--
-- Создать триггер `tr_remove_goods_from_catalogs`
--
CREATE
DEFINER = 'root'@'localhost'
TRIGGER tr_remove_goods_from_catalogs
BEFORE DELETE
ON goods_catalog
FOR EACH ROW
BEGIN
  DELETE
    FROM storage
  WHERE (IDgc = OLD.IDgc)
    AND (sCount = 0);
END
$$

--
-- Создать триггер `tr_add_goods_in_catalog`
--
CREATE
DEFINER = 'root'@'localhost'
TRIGGER tr_add_goods_in_catalog
AFTER INSERT
ON goods_catalog
FOR EACH ROW
BEGIN
  INSERT INTO storage
  SET IDgc = NEW.idgc,
      sCount = 0;
END
$$

DELIMITER ;

-- 
-- Восстановить предыдущий режим SQL (SQL mode)
--
/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;

-- 
-- Включение внешних ключей
-- 
/*!40014 SET FOREIGN_KEY_CHECKS = @OLD_FOREIGN_KEY_CHECKS */;