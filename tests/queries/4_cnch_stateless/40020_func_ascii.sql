SELECT ASCII(CAST(-3.6 AS DECIMAL(8, 6)));
SELECT ASCII(CAST(3.6 AS DECIMAL(8, 6)));
SELECT ASCII('t');
SELECT ASCII('techonthenet.com');
SELECT ASCII('T2');
SELECT ASCII('2');
SELECT ASCII('200');
SELECT ASCII(200);
SELECT ASCII(-200);
SELECT ASCII(NULL);
create table table_1(val1 Int64,val2 String, val3 float, val4 DECIMAL(8,6)) engine=CnchMergeTree() order by val1;
insert into table_1 values(-1, 'ada', 2.0, 6.9);
insert into table_1 values(-2, 'bfdf', -3.0, -5.8);
insert into table_1 values(3, '', 2.9999, -5);
insert into table_1 values(4, '===', -551.556, 0);
select ASCII(val1), ASCII(val2), ASCII(val3), ASCII(val4) from table_1 order by val1 ASC;
