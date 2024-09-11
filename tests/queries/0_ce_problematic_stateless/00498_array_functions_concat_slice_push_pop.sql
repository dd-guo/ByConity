select 'const args';
select 'concat';
select arrayConcat(emptyArrayUInt8());
select arrayConcat(emptyArrayUInt8(), emptyArrayUInt8());
select arrayConcat(emptyArrayUInt8(), emptyArrayUInt8(), emptyArrayUInt8());
select arrayConcat([Null], emptyArrayUInt8());
select arrayConcat([Null], emptyArrayUInt8(), [1]);
select arrayConcat([1, 2], [-1, -2], [0.3, 0.7], [Null]);
select arrayConcat(Null, emptyArrayUInt8());
select arrayConcat([1], [-1], Null);
select arrayConcat([1, 2], [3, 4]);
select arrayConcat([1], [2, 3, 4]);
select arrayConcat(emptyArrayUInt8(), emptyArrayUInt8());
SELECT arrayConcat(['abc'], ['def', 'gh', 'qwe']);
SELECT arrayConcat([1, NULL, 2], [3, NULL, 4]);
select arrayConcat([1, Null, 2], [3, 4]);

select 'slice';
select arraySlice(Null, 1, 2);
select arraySlice([1, 2, 3, 4, 5, 6], Null, Null);
select arraySlice([1, 2, 3, 4, 5, 6], 2, Null);
select arraySlice([1, 2, 3, 4, 5, 6], Null, 4);
select arraySlice([1, 2, 3, 4, 5, 6], Null, -2);
select arraySlice([1, 2, 3, 4, 5, 6], -3, Null);
select arraySlice([1, 2, 3, 4, 5, 6], 2, 3);
select arraySlice([1, 2, 3, 4, 5, 6], 2, -2);
select arraySlice([1, 2, 3, 4, 5, 6], -4, 2);
select arraySlice([1, 2, 3, 4, 5, 6], -4, -2);
select arraySlice([1, 2, 3, 4, 5, 6], 2, 0);
select arraySlice([1, 2, 3, 4, 5, 6], -10, 15);
select arraySlice([1, 2, 3, 4, 5, 6], -15, 10);
select arraySlice([1, 2, 3, 4, 5, 6], -15, 9);
select arraySlice([1, 2, 3, 4, 5, 6], 10, 0);
select arraySlice([1, 2, 3, 4, 5, 6], 10, -1);
select arraySlice([1, 2, 3, 4, 5, 6], 10, 1);
select arraySlice([1, 2, Null, 4, 5, 6], 2, 4);
select arraySlice(['a', 'b', 'c', 'd', 'e'], 2, 3);
select arraySlice([Null, 'b', Null, 'd', 'e'], 2, 3);

select 'push back';
select arrayPushBack(Null, 1);
select arrayPushBack([1], 1);
select arrayPushBack([Null], 1);
select arrayPushBack([0.5, 0.7], 1);
select arrayPushBack([1], -1);
select arrayPushBack(['a', 'b'], 'cd');
select arrayPushBack(emptyArrayUInt8(), 1);
select arrayPushBack(emptyArrayUInt8(), -1);
select arrayPushBack(toNullable([2,1,3]), 4);
select arrayPushBack(toNullable([2,toNullable(1),3]), 4);
select arraySort(arrayPushBack(toNullable([2,1,3]), 4));

select 'push front';
select arrayPushFront(Null, 1);
select arrayPushFront([1], 1);
select arrayPushFront([Null], 1);
select arrayPushFront([0.5, 0.7], 1);
select arrayPushFront([1], -1);
select arrayPushFront(['a', 'b'], 'cd');
select arrayPushFront(emptyArrayUInt8(), 1);
select arrayPushFront(emptyArrayUInt8(), -1);

select 'pop back';
select arrayPopBack(Null);
select arrayPopBack(emptyArrayUInt8());
select arrayPopBack([1]);
select arrayPopBack([1, 2, 3]);
select arrayPopBack([0.1, 0.2, 0.3]);
select arrayPopBack(['a', 'b', 'c']);

select 'pop front';
select arrayPopFront(Null);
select arrayPopFront(emptyArrayUInt8());
select arrayPopFront([1]);
select arrayPopFront([1, 2, 3]);
select arrayPopFront([0.1, 0.2, 0.3]);
select arrayPopFront(['a', 'b', 'c']);

DROP TABLE if exists array_functions;
select '';
select 'table';
create table array_functions (arr1 Array(Int8), arr2 Array(Int8), o Int8, no Nullable(Int8), l Int8, nl Nullable(Int8)) engine = TinyLog;
insert into array_functions values ([], [], 1, Null, 1, Null), ([], [1], 1, Null, 1, Null), ([1, 2, 3, 4, 5], [6, 7], 2, Null, 1, Null), ([1, 2, 3, 4, 5, 6, 7], [8], 2, 2, 3, 3), ([1, 2, 3, 4, 5, 6, 7], [], 2, Null, -3, -3), ([1, 2, 3, 4, 5, 6, 7], [], 2, Null, -3, Null), ([1, 2, 3, 4, 5, 6, 7], [], -5, -5, 4, 4), ([1, 2, 3, 4, 5, 6, 7], [], -5, -5, -3, -3);

select * from array_functions;
select 'concat arr1, arr2';
select arrayConcat(arr1, arr2), arr1, arr2 from array_functions;
select 'concat arr1, arr2, arr1';
select arrayConcat(arr1, arr2, arr1), arr1, arr2 from array_functions;

select 'arraySlice(arr1, o, l)';
select arr1, o, l, arraySlice(arr1, o, l) from array_functions;
select 'arraySlice(arr1, no, nl)';
select arr1, no, nl, arraySlice(arr1, no, nl) from array_functions;
select 'arraySlice(arr1, 2, l)';
select arr1, 2, l, arraySlice(arr1, 2, l) from array_functions;
select 'arraySlice(arr1, o, 2)';
select arr1, o, 2, arraySlice(arr1, o, 2) from array_functions;
select 'arraySlice(arr1, 2, nl)';
select arr1, 2, nl, arraySlice(arr1, 2, nl) from array_functions;
select 'arraySlice(arr1, no, 2)';
select arr1, no, 2, arraySlice(arr1, no, 2) from array_functions;
select 'arraySlice(arr1, -4, l)';
select arr1, 2, l, arraySlice(arr1, -4, l) from array_functions;
select 'arraySlice(arr1, o, -2)';
select arr1, o, 2, arraySlice(arr1, o, -2) from array_functions;
select 'arraySlice(arr1, -4, nl)';
select arr1, 2, nl, arraySlice(arr1, -4, nl) from array_functions;
select 'arraySlice(arr1, no, -2)';
select arr1, no, 2, arraySlice(arr1, no, -2) from array_functions;
select 'arraySlice(arr1, 2, 4)';
select arr1, 2, 4, arraySlice(arr1, 2, 4) from array_functions;
select 'arraySlice(arr1, 2, -4)';
select arr1, 2, 4, arraySlice(arr1, 2, -4) from array_functions;
select 'arraySlice(arr1, -4, 2)';
select arr1, 2, 4, arraySlice(arr1, -4, 2) from array_functions;
select 'arraySlice(arr1, -4, -1)';
select arr1, 2, 4, arraySlice(arr1, -4, -1) from array_functions;

select 'arrayPushFront(arr1, 1)';
select arr1, arrayPushFront(arr1, 1) from array_functions;
select 'arrayPushFront(arr1, 0.1)';
select arr1, arrayPushFront(arr1, 0.1) from array_functions;
select 'arrayPushFront(arr1, l)';
select arr1, arrayPushFront(arr1, l) from array_functions;
select 'arrayPushFront(arr1, nl)';
select arr1, arrayPushFront(arr1, nl) from array_functions;
select 'arrayPushFront([1, 2, 3], l)';
select arrayPushFront([1, 2, 3], l) from array_functions;
select 'arrayPushFront([1, 2, 3], nl)' from array_functions;
select arrayPushFront([1, 2, 3], nl) from array_functions;

select 'arrayPushBack(arr1, 1)';
select arr1, arrayPushBack(arr1, 1) from array_functions;
select 'arrayPushBack(arr1, 0.1)';
select arr1, arrayPushBack(arr1, 0.1) from array_functions;
select 'arrayPushBack(arr1, l)';
select arr1, arrayPushBack(arr1, l) from array_functions;
select 'arrayPushBack(arr1, nl)';
select arr1, arrayPushBack(arr1, nl) from array_functions;
select 'arrayPushBack([1, 2, 3], l)';
select arrayPushBack([1, 2, 3], l) from array_functions;
select 'arrayPushBack([1, 2, 3], nl)';
select arrayPushBack([1, 2, 3], nl) from array_functions;

select 'arrayPopFront(arr1)';
select arr1, arrayPopFront(arr1) from array_functions;
select 'arrayPopBack(arr1)';
select arr1, arrayPopBack(arr1) from array_functions;


DROP TABLE if exists array_functions;
select '';
select 'table';
create table array_functions (arr1 Array(Nullable(Int8)), arr2 Array(Nullable(Float32)), o Int8, no Nullable(Int8), l Int8, nl Nullable(Int8)) engine = TinyLog;
insert into array_functions values ([], [], 1, Null, 1, Null), ([], [1, Null], 1, Null, 1, Null), ([1, 2, 3, 4, 5], [6, Null], 2, Null, 1, Null), ([1, Null, 3, 4, Null, 6, 7], [8], 2, 2, 3, 3),([1, 2, 3, Null, 5, 6, 7], [Null, 1], 2, Null, -3, -3),([1, 2, 3, 4, 5, Null, 7], [1, Null], 2, Null, -3, Null), ([1, 2, 3, 4, 5, 6, 7], [1, 2], -5, -5, 4, 4),([1, Null, 3, Null, 5, 6, 7], [], -5, -5, -3, -3);

select * from array_functions;
select 'concat arr1, arr2';
select arrayConcat(arr1, arr2), arr1, arr2 from array_functions;
select 'concat arr1, arr2, arr1';
select arrayConcat(arr1, arr2, arr1), arr1, arr2 from array_functions;

select 'arraySlice(arr1, o, l)';
select arr1, o, l, arraySlice(arr1, o, l) from array_functions;
select 'arraySlice(arr1, no, nl)';
select arr1, no, nl, arraySlice(arr1, no, nl) from array_functions;
select 'arraySlice(arr1, 2, l)';
select arr1, 2, l, arraySlice(arr1, 2, l) from array_functions;
select 'arraySlice(arr1, o, 2)';
select arr1, o, 2, arraySlice(arr1, o, 2) from array_functions;
select 'arraySlice(arr1, 2, nl)';
select arr1, 2, nl, arraySlice(arr1, 2, nl) from array_functions;
select 'arraySlice(arr1, no, 2)';
select arr1, no, 2, arraySlice(arr1, no, 2) from array_functions;
select 'arraySlice(arr1, -4, l)';
select arr1, 2, l, arraySlice(arr1, -4, l) from array_functions;
select 'arraySlice(arr1, o, -2)';
select arr1, o, 2, arraySlice(arr1, o, -2) from array_functions;
select 'arraySlice(arr1, -4, nl)';
select arr1, 2, nl, arraySlice(arr1, -4, nl) from array_functions;
select 'arraySlice(arr1, no, -2)';
select arr1, no, 2, arraySlice(arr1, no, -2) from array_functions;
select 'arraySlice(arr1, 2, 4)';
select arr1, 2, 4, arraySlice(arr1, 2, 4) from array_functions;
select 'arraySlice(arr1, 2, -4)';
select arr1, 2, 4, arraySlice(arr1, 2, -4) from array_functions;
select 'arraySlice(arr1, -4, 2)';
select arr1, 2, 4, arraySlice(arr1, -4, 2) from array_functions;
select 'arraySlice(arr1, -4, -1)';
select arr1, 2, 4, arraySlice(arr1, -4, -1) from array_functions;

select 'arrayPushFront(arr1, 1)';
select arr1, arrayPushFront(arr1, 1) from array_functions;
select 'arrayPushFront(arr1, 0.1)';
select arr1, arrayPushFront(arr1, 0.1) from array_functions;
select 'arrayPushFront(arr1, l)';
select arr1, arrayPushFront(arr1, l) from array_functions;
select 'arrayPushFront(arr1, nl)';
select arr1, arrayPushFront(arr1, nl) from array_functions;
select 'arrayPushFront([1, 2, 3], l)';
select arrayPushFront([1, 2, 3], l) from array_functions;
select 'arrayPushFront([1, 2, 3], nl)' from array_functions;
select arrayPushFront([1, 2, 3], nl) from array_functions;

select 'arrayPushBack(arr1, 1)';
select arr1, arrayPushBack(arr1, 1) from array_functions;
select 'arrayPushBack(arr1, 0.1)';
select arr1, arrayPushBack(arr1, 0.1) from array_functions;
select 'arrayPushBack(arr1, l)';
select arr1, arrayPushBack(arr1, l) from array_functions;
select 'arrayPushBack(arr1, nl)';
select arr1, arrayPushBack(arr1, nl) from array_functions;
select 'arrayPushBack([1, 2, 3], l)';
select arrayPushBack([1, 2, 3], l) from array_functions;
select 'arrayPushBack([1, 2, 3], nl)';
select arrayPushBack([1, 2, 3], nl) from array_functions;

select 'arrayPopFront(arr1)';
select arr1, arrayPopFront(arr1) from array_functions;
select 'arrayPopBack(arr1)';
select arr1, arrayPopBack(arr1) from array_functions;


DROP TABLE if exists array_functions;
select '';
select 'table';
create table array_functions (arr1 Array(Nullable(Int8)), arr2 Array(UInt8), o Int8, no Nullable(Int8), l Int8, nl Nullable(Int8)) engine = TinyLog;
insert into array_functions values ([], [], 1, Null, 1, Null), ([], [1, 2], 1, Null, 1, Null), ([1, 2, 3, 4, 5], [6, 7], 2, Null, 1, Null), ([1, Null,3,4, Null, 6, 7], [8], 2, 2, 3, 3),([1, 2, 3, Null, 5, 6, 7], [0, 1], 2, Null, -3, -3),([1, 2, 3, 4, 5, Null, 7], [1, 2], 2, Null, -3, Null),([1, 2, 3,4, 5, 6, 7], [1, 2], -5, -5, 4, 4),([1, Null, 3, Null, 5, 6, 7], [], -5, -5, -3, -3);

select * from array_functions;
select 'concat arr1, arr2';
select arrayConcat(arr1, arr2), arr1, arr2 from array_functions;
select 'concat arr1, arr2, arr1';
select arrayConcat(arr1, arr2, arr1), arr1, arr2 from array_functions;


select * from array_functions;
select 'concat arr1, arr2';
select arrayConcat(arr1, arr2), arr1, arr2 from array_functions;
select 'concat arr1, arr2, arr1';
select arrayConcat(arr1, arr2, arr1), arr1, arr2 from array_functions;

select 'arraySlice(arr1, o, l)';
select arr1, o, l, arraySlice(arr1, o, l) from array_functions;
select 'arraySlice(arr1, no, nl)';
select arr1, no, nl, arraySlice(arr1, no, nl) from array_functions;
select 'arraySlice(arr1, 2, l)';
select arr1, 2, l, arraySlice(arr1, 2, l) from array_functions;
select 'arraySlice(arr1, o, 2)';
select arr1, o, 2, arraySlice(arr1, o, 2) from array_functions;
select 'arraySlice(arr1, 2, nl)';
select arr1, 2, nl, arraySlice(arr1, 2, nl) from array_functions;
select 'arraySlice(arr1, no, 2)';
select arr1, no, 2, arraySlice(arr1, no, 2) from array_functions;
select 'arraySlice(arr1, -4, l)';
select arr1, 2, l, arraySlice(arr1, -4, l) from array_functions;
select 'arraySlice(arr1, o, -2)';
select arr1, o, 2, arraySlice(arr1, o, -2) from array_functions;
select 'arraySlice(arr1, -4, nl)';
select arr1, 2, nl, arraySlice(arr1, -4, nl) from array_functions;
select 'arraySlice(arr1, no, -2)';
select arr1, no, 2, arraySlice(arr1, no, -2) from array_functions;
select 'arraySlice(arr1, 2, 4)';
select arr1, 2, 4, arraySlice(arr1, 2, 4) from array_functions;
select 'arraySlice(arr1, 2, -4)';
select arr1, 2, 4, arraySlice(arr1, 2, -4) from array_functions;
select 'arraySlice(arr1, -4, 2)';
select arr1, 2, 4, arraySlice(arr1, -4, 2) from array_functions;
select 'arraySlice(arr1, -4, -1)';
select arr1, 2, 4, arraySlice(arr1, -4, -1) from array_functions;

select 'arrayPushFront(arr1, 1)';
select arr1, arrayPushFront(arr1, 1) from array_functions;
select 'arrayPushFront(arr1, 0.1)';
select arr1, arrayPushFront(arr1, 0.1) from array_functions;
select 'arrayPushFront(arr1, l)';
select arr1, arrayPushFront(arr1, l) from array_functions;
select 'arrayPushFront(arr1, nl)';
select arr1, arrayPushFront(arr1, nl) from array_functions;
select 'arrayPushFront([1, 2, 3], l)';
select arrayPushFront([1, 2, 3], l) from array_functions;
select 'arrayPushFront([1, 2, 3], nl)' from array_functions;
select arrayPushFront([1, 2, 3], nl) from array_functions;

select 'arrayPushBack(arr1, 1)';
select arr1, arrayPushBack(arr1, 1) from array_functions;
select 'arrayPushBack(arr1, 0.1)';
select arr1, arrayPushBack(arr1, 0.1) from array_functions;
select 'arrayPushBack(arr1, l)';
select arr1, arrayPushBack(arr1, l) from array_functions;
select 'arrayPushBack(arr1, nl)';
select arr1, arrayPushBack(arr1, nl) from array_functions;
select 'arrayPushBack([1, 2, 3], l)';
select arrayPushBack([1, 2, 3], l) from array_functions;
select 'arrayPushBack([1, 2, 3], nl)';
select arrayPushBack([1, 2, 3], nl) from array_functions;

select 'arrayPopFront(arr1)';
select arr1, arrayPopFront(arr1) from array_functions;
select 'arrayPopBack(arr1)';
select arr1, arrayPopBack(arr1) from array_functions;

DROP TABLE if exists array_functions;
select '';
select 'table';
create table array_functions (arr1 Array(Nullable(String)), arr2 Array(String), val String, val2 Nullable(String), o Int8, no Nullable(Int8), l Int8, nl Nullable(Int8)) engine = TinyLog;
insert into array_functions values ([], [], '', Null, 1, Null, 1, Null), ([], ['1', '2'], 'a', 'b', 1, Null, 1, Null), (['1', '2', '3', '4', '5'], ['6','7'], 'a', Null, 2, Null, 1, Null), (['1', Null, '3', '4', Null, '6', '7'], ['8'], 'a', 'b', 2, 2, 3, 3),(['1', '2', '3', Null, '5', '6', '7'], ['0','1'], 'a', Null, 2, Null, -3, -3),(['1', '2', '3', '4', '5', Null, '7'], ['1', '2'], 'a', 'b', 2, Null, -3, Null),(['1', '2', '3', '4', '5', '6', '7'],['1', '2'], 'a', Null, -5, -5, 4, 4),(['1', Null, '3', Null, '5', '6', '7'], [], 'a', 'b', -5, -5, -3, -3);


select * from array_functions;
select 'concat arr1, arr2';
select arrayConcat(arr1, arr2), arr1, arr2 from array_functions;
select 'concat arr1, arr2, arr1';
select arrayConcat(arr1, arr2, arr1), arr1, arr2 from array_functions;

select 'arraySlice(arr1, o, l)';
select arr1, o, l, arraySlice(arr1, o, l) from array_functions;
select 'arraySlice(arr1, no, nl)';
select arr1, no, nl, arraySlice(arr1, no, nl) from array_functions;
select 'arraySlice(arr1, 2, l)';
select arr1, 2, l, arraySlice(arr1, 2, l) from array_functions;
select 'arraySlice(arr1, o, 2)';
select arr1, o, 2, arraySlice(arr1, o, 2) from array_functions;
select 'arraySlice(arr1, 2, nl)';
select arr1, 2, nl, arraySlice(arr1, 2, nl) from array_functions;
select 'arraySlice(arr1, no, 2)';
select arr1, no, 2, arraySlice(arr1, no, 2) from array_functions;
select 'arraySlice(arr1, -4, l)';
select arr1, 2, l, arraySlice(arr1, -4, l) from array_functions;
select 'arraySlice(arr1, o, -2)';
select arr1, o, 2, arraySlice(arr1, o, -2) from array_functions;
select 'arraySlice(arr1, -4, nl)';
select arr1, 2, nl, arraySlice(arr1, -4, nl) from array_functions;
select 'arraySlice(arr1, no, -2)';
select arr1, no, 2, arraySlice(arr1, no, -2) from array_functions;
select 'arraySlice(arr1, 2, 4)';
select arr1, 2, 4, arraySlice(arr1, 2, 4) from array_functions;
select 'arraySlice(arr1, 2, -4)';
select arr1, 2, 4, arraySlice(arr1, 2, -4) from array_functions;
select 'arraySlice(arr1, -4, 2)';
select arr1, 2, 4, arraySlice(arr1, -4, 2) from array_functions;
select 'arraySlice(arr1, -4, -1)';
select arr1, 2, 4, arraySlice(arr1, -4, -1) from array_functions;

select 'arrayPushFront(arr1, 1)';
select arr1, arrayPushFront(arr1, '1') from array_functions;
select 'arrayPushFront(arr1, val)';
select arr1, arrayPushFront(arr1, val) from array_functions;
select 'arrayPushFront(arr1, val2)';
select arr1, arrayPushFront(arr1, val2) from array_functions;
select 'arrayPushFront([a, b, c], val)';
select arrayPushFront(['a', 'b', 'c'], val) from array_functions;
select 'arrayPushFront([a, b, c], val2)';
select arrayPushFront(['a', 'b', 'c'], val2) from array_functions;

select 'arrayPushBack(arr1, 1)';
select arr1, arrayPushBack(arr1, '1') from array_functions;
select 'arrayPushBack(arr1, val)';
select arr1, arrayPushBack(arr1, val) from array_functions;
select 'arrayPushBack(arr1, val2)';
select arr1, arrayPushBack(arr1, val2) from array_functions;
select 'arrayPushBack([a, b, c], val)';
select arrayPushBack(['a', 'b', 'c'], val) from array_functions;
select 'arrayPushBack([a, b, c], val2)';
select arrayPushBack(['a', 'b', 'c'], val2) from array_functions;

select 'arrayPopFront(arr1)';
select arr1, arrayPopFront(arr1) from array_functions;
select 'arrayPopBack(arr1)';
select arr1, arrayPopBack(arr1) from array_functions;

DROP TABLE if exists array_functions;
