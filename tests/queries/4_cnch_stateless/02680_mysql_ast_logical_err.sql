SELECT count() FROM mysql(mysql('127.0.0.1:9004', currentDatabase(0), 'foo', 'default', ''), '127.0.0.1:9004', currentDatabase(0), 'foo', '', ''); -- { serverError UNKNOWN_FUNCTION }
-- SELECT count() FROM mysql(mysql('127.0.0.1:9004', currentDatabase(0), 'foo', 'default', '', SETTINGS connection_pool_size = 1), '127.0.0.1:9004', currentDatabase(0), 'foo', '', ''); -- { serverError UNKNOWN_FUNCTION }
