set enable_optimizer=1;
set enable_prune_source_plan_segment = 1;
DROP TABLE IF EXISTS source_prune_test;
DROP TABLE IF EXISTS source_prune_test_empty;
create table source_prune_test (id_nation Int32, name Nullable(String), region_key Nullable(Int32)) ENGINE=CnchMergeTree() order by id_nation;
create table source_prune_test_empty (id_nation Int32, name Nullable(String), region_key Nullable(Int32)) ENGINE=CnchMergeTree() order by id_nation;
insert into source_prune_test values(8,'a',55);
select '---------select';
select region_key from source_prune_test;
select '---------select empty';
select region_key from source_prune_test_empty;
