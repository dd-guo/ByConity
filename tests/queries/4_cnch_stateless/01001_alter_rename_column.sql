SELECT 'TEST RENAME COLUMN';
DROP TABLE IF EXISTS t_alter_rename_column;
CREATE TABLE t_alter_rename_column(k Int32, m Int32) ENGINE = CnchMergeTree ORDER BY k;
SYSTEM START MERGES t_alter_rename_column;

INSERT INTO t_alter_rename_column VALUES(1,1);
ALTER TABLE t_alter_rename_column RENAME COLUMN m to m1;

SELECT m1 FROM t_alter_rename_column SETTINGS enable_optimizer = 0;
SELECT m1 FROM t_alter_rename_column SETTINGS enable_optimizer = 1;

SELECT sleepEachRow(3) FROM numbers(2) FORMAT Null;

SELECT m1 FROM t_alter_rename_column SETTINGS enable_optimizer = 0;
SELECT m1 FROM t_alter_rename_column SETTINGS enable_optimizer = 1;

DROP TABLE t_alter_rename_column;

SELECT 'TEST RENAME COLUMN WITH CONCURRENT MERGE';
DROP TABLE IF EXISTS t_rename_with_concurrent_merge;
CREATE TABLE t_rename_with_concurrent_merge(k Int32, s String) ENGINE = CnchMergeTree ORDER BY k;
SYSTEM START MERGES t_rename_with_concurrent_merge;

INSERT INTO t_rename_with_concurrent_merge VALUES (1, '10000000');
INSERT INTO t_rename_with_concurrent_merge VALUES (1, '10000000');
INSERT INTO t_rename_with_concurrent_merge VALUES (1, '10000000');
INSERT INTO t_rename_with_concurrent_merge VALUES (1, '10000000');
INSERT INTO t_rename_with_concurrent_merge VALUES (1, '10000000');
INSERT INTO t_rename_with_concurrent_merge VALUES (1, '10000000');
INSERT INTO t_rename_with_concurrent_merge VALUES (1, '10000000');
INSERT INTO t_rename_with_concurrent_merge VALUES (1, '10000000');
INSERT INTO t_rename_with_concurrent_merge VALUES (1, '10000000');
INSERT INTO t_rename_with_concurrent_merge VALUES (1, '10000000');
INSERT INTO t_rename_with_concurrent_merge VALUES (1, '10000000');
INSERT INTO t_rename_with_concurrent_merge VALUES (1, '10000000');
-- trigger a merge task that source parts are before RENAME and merged part will be committed after RENAME.

ALTER TABLE t_rename_with_concurrent_merge RENAME COLUMN s to s1;

-- wait the merge task finish
SELECT sleepEachRow(3) FROM numbers(5) FORMAT Null;

SELECT count(*) FROM t_rename_with_concurrent_merge WHERE s1 = '';
