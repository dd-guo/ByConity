-- { echo }
select a, b from (select 1 as a) join (select 2 as b) on 1 = 1 settings enable_optimizer = 1;
select a, b from (select 1 as a) join (select 2 as b where false) on 1 = 1 settings enable_optimizer = 1;
select a, b from (select 1 as a) left join (select 2 as b where false) on 1 = 1 settings enable_optimizer = 1;
