-- optimizer will rewrite DISTINCT to AGGREGATE, making WITH TOTALS in a nested AGGREGATION, which is not suppprted yet
SET enable_optimizer=0;

SELECT hex(toString(uniqState(toNullable(1)))) WITH TOTALS;
SELECT '---';
SELECT hex(toString(uniqState(x))) FROM (SELECT toNullable(1) AS x) WITH TOTALS;
SELECT '---';
SELECT DISTINCT hex(toString(uniqState(x))) FROM (SELECT materialize(1) AS k, toNullable(1) AS x FROM numbers(1)) GROUP BY k WITH TOTALS ORDER BY k;
SELECT '---';
SELECT DISTINCT hex(toString(uniqState(x))) FROM (SELECT materialize(1) AS k, toNullable(1) AS x FROM numbers(10)) GROUP BY k WITH TOTALS ORDER BY k;
SELECT '---';
SELECT DISTINCT hex(toString(uniqState(x))) FROM (SELECT intDiv(number, 3) AS k, toNullable(1) AS x FROM numbers(10)) GROUP BY k WITH TOTALS ORDER BY k;
SELECT '---';
SELECT DISTINCT hex(toString(uniqState(x))) FROM (SELECT intDiv(number, 3) AS k, toNullable(1) AS x FROM system.numbers LIMIT 100000) GROUP BY k WITH TOTALS ORDER BY k;
SELECT '---';
SELECT DISTINCT arrayUniq(finalizeAggregation(groupArrayState(x))) FROM (SELECT intDiv(number, 3) AS k, toNullable(1) AS x FROM system.numbers LIMIT 100000) GROUP BY k WITH TOTALS ORDER BY k;
SELECT '---';
SELECT k, finalizeAggregation(uniqState(x)) FROM (SELECT intDiv(number, 3) AS k, toNullable(1) AS x FROM system.numbers LIMIT 100000) GROUP BY k WITH TOTALS ORDER BY k LIMIT 5;
SELECT '---';
SELECT k, finalizeAggregation(uniqState(x)) FROM (WITH toNullable(number = 3 ? 3 : 1) AS d SELECT intDiv(number, 3) AS k, number % d AS x FROM system.numbers LIMIT 100000) GROUP BY k WITH TOTALS ORDER BY k LIMIT 5;
SELECT '---';
SELECT k, finalizeAggregation(quantilesTimingState(0.5)(x)) FROM (WITH toNullable(number = 3 ? 3 : 1) AS d SELECT intDiv(number, 3) AS k, number % d AS x FROM system.numbers LIMIT 100000) GROUP BY k WITH TOTALS ORDER BY k LIMIT 5;
SELECT '---';
SELECT k, finalizeAggregation(quantilesTimingState(0.5)(x)) FROM (SELECT intDiv(number, if(number = 9223372036854775807, -2, if(number = 3, number = if(number = 1, NULL, 3), 1)) AS d) AS k, number % d AS x FROM system.numbers LIMIT 100000) GROUP BY k WITH TOTALS ORDER BY k ASC LIMIT 5;
SELECT '---';
SELECT DISTINCT hex(toString(uniqState(x))) FROM (SELECT materialize(1) AS k, toNullable(1) AS x FROM numbers(1)) GROUP BY k WITH ROLLUP ORDER BY k;
SELECT '---';
SELECT DISTINCT hex(toString(uniqState(x))) FROM (SELECT materialize(1) AS k, toNullable(1) AS x FROM numbers(1)) GROUP BY k WITH CUBE ORDER BY k;
SELECT '---';
