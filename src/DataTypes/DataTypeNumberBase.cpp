/*
 * Copyright 2016-2023 ClickHouse, Inc.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


/*
 * This file may have been modified by Bytedance Ltd. and/or its affiliates (“ Bytedance's Modifications”).
 * All Bytedance's Modifications are Copyright (2023) Bytedance Ltd. and/or its affiliates.
 */

#include <type_traits>
#include <DataTypes/DataTypeNumberBase.h>
#include <Columns/ColumnVector.h>
#include <Columns/ColumnConst.h>
#include <IO/ReadHelpers.h>
#include <IO/WriteHelpers.h>
#include <IO/ReadBufferFromString.h>
#include <Common/NaNUtils.h>
#include <Common/typeid_cast.h>
#include <Common/assert_cast.h>
#include <Formats/FormatSettings.h>


namespace DB
{

template <typename T>
Field DataTypeNumberBase<T>::getDefault() const
{
    return NearestFieldType<FieldType>();
}

template <typename T>
MutableColumnPtr DataTypeNumberBase<T>::createColumn() const
{
    if (enable_zero_cpy_read) {
        return ColumnVector<T, true>::create();
    } else {
        return ColumnVector<T, false>::create();
    }
    
}

template <typename T>
bool DataTypeNumberBase<T>::isValueRepresentedByInteger() const
{
    return is_integer_v<T>;
}

template <typename T>
bool DataTypeNumberBase<T>::isValueRepresentedByUnsignedInteger() const
{
    return is_integer_v<T> && is_unsigned_v<T>;
}

template <typename T>
Field DataTypeNumberBase<T>::stringToVisitorField(const String& ins) const
{
    // This routine assume ins was generated by FieldVisitorToString
    ReadBufferFromString rb(ins);

    NearestFieldType<FieldType> res;
    readText(res, rb);
    return res;
}

template <typename T>
String DataTypeNumberBase<T>::stringToVisitorString(const String& ins) const
{
    Field field = stringToVisitorField(ins);
    T val = DB::get<T>(field);
    return String(reinterpret_cast<const char *>(&val), getSizeOfValueInMemory());
}

/// Explicit template instantiations - to avoid code bloat in headers.
template class DataTypeNumberBase<UInt8>;
template class DataTypeNumberBase<UInt16>;
template class DataTypeNumberBase<UInt32>;
template class DataTypeNumberBase<UInt64>;
template class DataTypeNumberBase<UInt128>;
template class DataTypeNumberBase<UInt256>;
template class DataTypeNumberBase<Int8>;
template class DataTypeNumberBase<Int16>;
template class DataTypeNumberBase<Int32>;
template class DataTypeNumberBase<Int64>;
template class DataTypeNumberBase<Int128>;
template class DataTypeNumberBase<Int256>;
template class DataTypeNumberBase<Float32>;
template class DataTypeNumberBase<Float64>;

}
