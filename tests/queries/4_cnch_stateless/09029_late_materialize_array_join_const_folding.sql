SET log_queries = 1;
SELECT 1 LIMIT 0;
SYSTEM FLUSH LOGS;

SELECT * FROM system.query_log
WHERE ProfileEvents['Query'] > 0 and current_database = currentDatabase(0) 

LIMIT 0;
