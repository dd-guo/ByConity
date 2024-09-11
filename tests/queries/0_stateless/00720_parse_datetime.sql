-- year
select parseDateTime('2020', '%Y', 'UTC') = toDateTime('2020-01-01', 'UTC');

-- month
select parseDateTime('02', '%m', 'UTC') = toDateTime('2000-02-01', 'UTC');
select parseDateTime('07', '%m', 'UTC') = toDateTime('2000-07-01', 'UTC');
select parseDateTime('11-', '%m-', 'UTC') = toDateTime('2000-11-01', 'UTC');
select parseDateTime('00', '%m'); -- { serverError 41 }
select parseDateTime('13', '%m'); -- { serverError 41 }
select parseDateTime('12345', '%m'); -- { serverError 41 }
select parseDateTime('02', '%c', 'UTC') = toDateTime('2000-02-01', 'UTC');
select parseDateTime('07', '%c', 'UTC') = toDateTime('2000-07-01', 'UTC');
select parseDateTime('11-', '%c-', 'UTC') = toDateTime('2000-11-01', 'UTC');
select parseDateTime('00', '%c'); -- { serverError 41 }
select parseDateTime('13', '%c'); -- { serverError 41 }
select parseDateTime('12345', '%c'); -- { serverError 41 }
select parseDateTime('jun', '%b', 'UTC') = toDateTime('2000-06-01', 'UTC');
select parseDateTime('JUN', '%b', 'UTC') = toDateTime('2000-06-01', 'UTC');
select parseDateTime('abc', '%b'); -- { serverError 41 }
set formatdatetime_parsedatetime_m_is_month_name = 1;
select parseDateTime('may', '%M', 'UTC') = toDateTime('2000-05-01', 'UTC');
select parseDateTime('MAY', '%M', 'UTC') = toDateTime('2000-05-01', 'UTC');
select parseDateTime('september', '%M', 'UTC') = toDateTime('2000-09-01', 'UTC');
select parseDateTime('summer', '%M'); -- { serverError 41 }
set formatdatetime_parsedatetime_m_is_month_name = 0;
select parseDateTime('08', '%M', 'UTC') = toDateTime('1970-01-01 00:08:00', 'UTC');
select parseDateTime('59', '%M', 'UTC') = toDateTime('1970-01-01 00:59:00', 'UTC');
select parseDateTime('00/', '%M/', 'UTC') = toDateTime('1970-01-01 00:00:00', 'UTC');
select parseDateTime('60', '%M', 'UTC'); -- { serverError 41 }
select parseDateTime('-1', '%M', 'UTC'); -- { serverError 41 }
select parseDateTime('123456789', '%M', 'UTC'); -- { serverError 41 }
set formatdatetime_parsedatetime_m_is_month_name = 1;

-- day of month
select parseDateTime('07', '%d', 'UTC') = toDateTime('2000-01-07', 'UTC');
select parseDateTime('01', '%d', 'UTC') = toDateTime('2000-01-01', 'UTC');
select parseDateTime('/11', '/%d', 'UTC') = toDateTime('2000-01-11', 'UTC');
select parseDateTime('00', '%d'); -- { serverError 41 }
select parseDateTime('32', '%d'); -- { serverError 41 }
select parseDateTime('12345', '%d'); -- { serverError 41 }
select parseDateTime('02-31', '%m-%d'); -- { serverError 41 }
select parseDateTime('04-31', '%m-%d'); -- { serverError 41 }
-- The last one is chosen if multiple months of year if supplied
select parseDateTime('01 31 20 02', '%m %d %d %m', 'UTC') = toDateTime('2000-02-20', 'UTC');
select parseDateTime('02 31 20 04', '%m %d %d %m', 'UTC') = toDateTime('2000-04-20', 'UTC');
select parseDateTime('02 31 01', '%m %d %m', 'UTC') = toDateTime('2000-01-31', 'UTC');
select parseDateTime('2000-02-29', '%Y-%m-%d', 'UTC') = toDateTime('2000-02-29', 'UTC');
select parseDateTime('2001-02-29', '%Y-%m-%d'); -- { serverError 41 }

-- day of year
select parseDateTime('001', '%j', 'UTC') = toDateTime('2000-01-01', 'UTC');
select parseDateTime('007', '%j', 'UTC') = toDateTime('2000-01-07', 'UTC');
select parseDateTime('/031/', '/%j/', 'UTC') = toDateTime('2000-01-31', 'UTC');
select parseDateTime('032', '%j', 'UTC') = toDateTime('2000-02-01', 'UTC');
select parseDateTime('060', '%j', 'UTC') = toDateTime('2000-02-29', 'UTC');
select parseDateTime('365', '%j', 'UTC') = toDateTime('2000-12-30', 'UTC');
select parseDateTime('366', '%j', 'UTC') = toDateTime('2000-12-31', 'UTC');
select parseDateTime('1980 001', '%Y %j', 'UTC') = toDateTime('1980-01-01', 'UTC');
select parseDateTime('1980 007', '%Y %j', 'UTC') = toDateTime('1980-01-07', 'UTC');
select parseDateTime('1980 /007', '%Y /%j', 'UTC') = toDateTime('1980-01-07', 'UTC');
select parseDateTime('1980 /031/', '%Y /%j/', 'UTC') = toDateTime('1980-01-31', 'UTC');
select parseDateTime('1980 032', '%Y %j', 'UTC') = toDateTime('1980-02-01', 'UTC');
select parseDateTime('1980 060', '%Y %j', 'UTC') = toDateTime('1980-02-29', 'UTC');
select parseDateTime('1980 366', '%Y %j', 'UTC') = toDateTime('1980-12-31', 'UTC');
select parseDateTime('1981 366', '%Y %j'); -- { serverError 41 }
select parseDateTime('367', '%j'); -- { serverError 41 }
select parseDateTime('000', '%j'); -- { serverError 41 }
-- The last one is chosen if multiple day of years are supplied.
select parseDateTime('2000 366 2001', '%Y %j %Y'); -- { serverError 41 }
select parseDateTime('2001 366 2000', '%Y %j %Y', 'UTC') = toDateTime('2000-12-31', 'UTC');

-- hour of day
select parseDateTime('07', '%H', 'UTC') = toDateTime('1970-01-01 07:00:00', 'UTC');
select parseDateTime('23', '%H', 'UTC') = toDateTime('1970-01-01 23:00:00', 'UTC');
select parseDateTime('00', '%H', 'UTC') = toDateTime('1970-01-01 00:00:00', 'UTC');
select parseDateTime('10', '%H', 'UTC') = toDateTime('1970-01-01 10:00:00', 'UTC');
select parseDateTime('24', '%H', 'UTC'); -- { serverError 41 }
select parseDateTime('-1', '%H', 'UTC'); -- { serverError 41 }
select parseDateTime('1234567', '%H', 'UTC'); -- { serverError 41 }
select parseDateTime('07', '%k', 'UTC') = toDateTime('1970-01-01 07:00:00', 'UTC');
select parseDateTime('23', '%k', 'UTC') = toDateTime('1970-01-01 23:00:00', 'UTC');
select parseDateTime('00', '%k', 'UTC') = toDateTime('1970-01-01 00:00:00', 'UTC');
select parseDateTime('10', '%k', 'UTC') = toDateTime('1970-01-01 10:00:00', 'UTC');
select parseDateTime('24', '%k', 'UTC'); -- { serverError 41 }
select parseDateTime('-1', '%k', 'UTC'); -- { serverError 41 }
select parseDateTime('1234567', '%k', 'UTC'); -- { serverError 41 }

-- hour of half day
select parseDateTime('07', '%h', 'UTC') = toDateTime('1970-01-01 07:00:00', 'UTC');
select parseDateTime('12', '%h', 'UTC') = toDateTime('1970-01-01 00:00:00', 'UTC');
select parseDateTime('01', '%h', 'UTC') = toDateTime('1970-01-01 01:00:00', 'UTC');
select parseDateTime('10', '%h', 'UTC') = toDateTime('1970-01-01 10:00:00', 'UTC');
select parseDateTime('00', '%h', 'UTC'); -- { serverError 41 }
select parseDateTime('13', '%h', 'UTC'); -- { serverError 41 }
select parseDateTime('123456789', '%h', 'UTC'); -- { serverError 41 }
select parseDateTime('07', '%I', 'UTC') = toDateTime('1970-01-01 07:00:00', 'UTC');
select parseDateTime('12', '%I', 'UTC') = toDateTime('1970-01-01 00:00:00', 'UTC');
select parseDateTime('01', '%I', 'UTC') = toDateTime('1970-01-01 01:00:00', 'UTC');
select parseDateTime('10', '%I', 'UTC') = toDateTime('1970-01-01 10:00:00', 'UTC');
select parseDateTime('00', '%I', 'UTC'); -- { serverError 41 }
select parseDateTime('13', '%I', 'UTC'); -- { serverError 41 }
select parseDateTime('123456789', '%I', 'UTC'); -- { serverError 41 }
select parseDateTime('07', '%l', 'UTC') = toDateTime('1970-01-01 07:00:00', 'UTC');
select parseDateTime('12', '%l', 'UTC') = toDateTime('1970-01-01 00:00:00', 'UTC');
select parseDateTime('01', '%l', 'UTC') = toDateTime('1970-01-01 01:00:00', 'UTC');
select parseDateTime('10', '%l', 'UTC') = toDateTime('1970-01-01 10:00:00', 'UTC');
select parseDateTime('00', '%l', 'UTC'); -- { serverError 41 }
select parseDateTime('13', '%l', 'UTC'); -- { serverError 41 }
select parseDateTime('123456789', '%l', 'UTC'); -- { serverError 41 }

-- half of day
select parseDateTime('07 PM', '%H %p', 'UTC') = toDateTime('1970-01-01 07:00:00', 'UTC');
select parseDateTime('07 AM', '%H %p', 'UTC') = toDateTime('1970-01-01 07:00:00', 'UTC');
select parseDateTime('07 pm', '%H %p', 'UTC') = toDateTime('1970-01-01 07:00:00', 'UTC');
select parseDateTime('07 am', '%H %p', 'UTC') = toDateTime('1970-01-01 07:00:00', 'UTC');
select parseDateTime('00 AM', '%H %p', 'UTC') = toDateTime('1970-01-01 00:00:00', 'UTC');
select parseDateTime('00 PM', '%H %p', 'UTC') = toDateTime('1970-01-01 00:00:00', 'UTC');
select parseDateTime('00 am', '%H %p', 'UTC') = toDateTime('1970-01-01 00:00:00', 'UTC');
select parseDateTime('00 pm', '%H %p', 'UTC') = toDateTime('1970-01-01 00:00:00', 'UTC');
select parseDateTime('01 PM', '%h %p', 'UTC') = toDateTime('1970-01-01 13:00:00', 'UTC');
select parseDateTime('01 AM', '%h %p', 'UTC') = toDateTime('1970-01-01 01:00:00', 'UTC');
select parseDateTime('06 PM', '%h %p', 'UTC') = toDateTime('1970-01-01 18:00:00', 'UTC');
select parseDateTime('06 AM', '%h %p', 'UTC') = toDateTime('1970-01-01 06:00:00', 'UTC');
select parseDateTime('12 PM', '%h %p', 'UTC') = toDateTime('1970-01-01 12:00:00', 'UTC');
select parseDateTime('12 AM', '%h %p', 'UTC') = toDateTime('1970-01-01 00:00:00', 'UTC');

-- minute
select parseDateTime('08', '%i', 'UTC') = toDateTime('1970-01-01 00:08:00', 'UTC');
select parseDateTime('59', '%i', 'UTC') = toDateTime('1970-01-01 00:59:00', 'UTC');
select parseDateTime('00/', '%i/', 'UTC') = toDateTime('1970-01-01 00:00:00', 'UTC');
select parseDateTime('60', '%i', 'UTC'); -- { serverError 41 }
select parseDateTime('-1', '%i', 'UTC'); -- { serverError 41 }
select parseDateTime('123456789', '%i', 'UTC'); -- { serverError 41 }

-- second
select parseDateTime('09', '%s', 'UTC') = toDateTime('1970-01-01 00:00:09', 'UTC');
select parseDateTime('58', '%s', 'UTC') = toDateTime('1970-01-01 00:00:58', 'UTC');
select parseDateTime('00/', '%s/', 'UTC') = toDateTime('1970-01-01 00:00:00', 'UTC');
select parseDateTime('60', '%s', 'UTC'); -- { serverError 41 }
select parseDateTime('-1', '%s', 'UTC'); -- { serverError 41 }
select parseDateTime('123456789', '%s', 'UTC'); -- { serverError 41 }

-- microsecond
select parseDateTime('000000', '%f', 'UTC') = toDateTime('1970-01-01 00:00:00', 'UTC');
select parseDateTime('456789', '%f', 'UTC') = toDateTime('1970-01-01 00:00:00', 'UTC');
select parseDateTime('42', '%f', 'UTC') = toDateTime('1970-01-01 00:00:00', 'UTC'); -- { serverError 243 }
select parseDateTime('12ABCD', '%f', 'UTC') = toDateTime('1970-01-01 00:00:00', 'UTC'); -- { serverError 41 }

-- mixed YMD format
select parseDateTime('2021-01-04+23:00:00.654321', '%Y-%m-%d+%H:%i:%s.%f', 'UTC') = toDateTime('2021-01-04 23:00:00', 'UTC');
select parseDateTime('2019-07-03 11:04:10.975319', '%Y-%m-%d %H:%i:%s.%f', 'UTC') = toDateTime('2019-07-03 11:04:10', 'UTC');
select parseDateTime('10:04:11 03-07-2019.242424', '%s:%i:%H %d-%m-%Y.%f', 'UTC') = toDateTime('2019-07-03 11:04:10', 'UTC');

-- *OrZero, *OrNull, str_to_date
select parseDateTimeOrZero('10:04:11 03-07-2019', '%s:%i:%H %d-%m-%Y', 'UTC') = toDateTime('2019-07-03 11:04:10', 'UTC');
select parseDateTimeOrZero('10:04:11 invalid 03-07-2019', '%s:%i:%H %d-%m-%Y', 'UTC') = toDateTime('1970-01-01 00:00:00', 'UTC');
select parseDateTimeOrNull('10:04:11 03-07-2019', '%s:%i:%H %d-%m-%Y', 'UTC') = toDateTime('2019-07-03 11:04:10', 'UTC');
select parseDateTimeOrNull('10:04:11 invalid 03-07-2019', '%s:%i:%H %d-%m-%Y', 'UTC') IS NULL;
select str_to_date('10:04:11 03-07-2019', '%s:%i:%H %d-%m-%Y', 'UTC') = toDateTime('2019-07-03 11:04:10', 'UTC');
select sTr_To_DaTe('10:04:11 03-07-2019', '%s:%i:%H %d-%m-%Y', 'UTC') = toDateTime('2019-07-03 11:04:10', 'UTC');
select str_to_date('10:04:11 invalid 03-07-2019', '%s:%i:%H %d-%m-%Y', 'UTC') IS NULL;

-- Error handling
select parseDateTime('12 AM'); -- { serverError 42 }
select parseDateTime('12 AM', '%h %p', 'UTC', 'a fourth argument'); -- { serverError 42 }

-- Fuzzer crash bug #53715
select parseDateTime('', '', toString(number)) from numbers(13); -- { serverError 44 }
