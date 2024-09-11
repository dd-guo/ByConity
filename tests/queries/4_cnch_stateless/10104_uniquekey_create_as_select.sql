DROP TABLE IF EXISTS u10104_t1;
DROP TABLE IF EXISTS u10104_t2;

CREATE TABLE u10104_t1(a Int32, b String) Engine=CnchMergeTree() ORDER BY b UNIQUE KEY a;
INSERT INTO u10104_t1 VALUES(1, '2');

CREATE TABLE u10104_t2 Engine=CnchMergeTree UNIQUE KEY(a) AS SELECT * FROM u10104_t1; -- { serverError 36 }
CREATE TABLE u10104_t2 Engine=CnchMergeTree ORDER BY b UNIQUE KEY(a) AS SELECT * FROM u10104_t1;

SELECT * FROM u10104_t2;

DROP TABLE IF EXISTS u10104_t1;
DROP TABLE IF EXISTS u10104_t2;
