// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Diagnostics.CodeAnalysis;

namespace System.Data
{
    /// <summary>
    /// This static class defines the DataRow extension methods.
    /// </summary>
    public static class DataRowExtensions
    {
        /// <summary>
        /// This method provides access to the values in each of the columns in a given row.
        /// This method makes casts unnecessary when accessing columns.
        /// Additionally, Field supports nullable types and maps automatically between DBNull and
        /// Nullable when the generic type is nullable.
        /// </summary>
        /// <param name="row">The input DataRow</param>
        /// <param name="columnName">The input column name specifying which row value to retrieve.</param>
        /// <returns>The DataRow value for the column specified.</returns>
        [return: MaybeNull]
        public static T Field<T>(this DataRow row, string columnName)
        {
            DataSetUtil.CheckArgumentNull(row, nameof(row));
            return UnboxT<T>.s_unbox(row[columnName]);
        }

        /// <summary>
        /// This method provides access to the values in each of the columns in a given row.
        /// This method makes casts unnecessary when accessing columns.
        /// Additionally, Field supports nullable types and maps automatically between DBNull and
        /// Nullable when the generic type is nullable.
        /// </summary>
        /// <param name="row">The input DataRow</param>
        /// <param name="column">The input DataColumn specifying which row value to retrieve.</param>
        /// <returns>The DataRow value for the column specified.</returns>
        [return: MaybeNull]
        public static T Field<T>(this DataRow row, DataColumn column)
        {
            DataSetUtil.CheckArgumentNull(row, nameof(row));
            return UnboxT<T>.s_unbox(row[column]);
        }

        /// <summary>
        /// This method provides access to the values in each of the columns in a given row.
        /// This method makes casts unnecessary when accessing columns.
        /// Additionally, Field supports nullable types and maps automatically between DBNull and
        /// Nullable when the generic type is nullable.
        /// </summary>
        /// <param name="row">The input DataRow</param>
        /// <param name="columnIndex">The input ordinal specifying which row value to retrieve.</param>
        /// <returns>The DataRow value for the column specified.</returns>
        [return: MaybeNull]
        public static T Field<T>(this DataRow row, int columnIndex)
        {
            DataSetUtil.CheckArgumentNull(row, nameof(row));
            return UnboxT<T>.s_unbox(row[columnIndex]);
        }

        /// <summary>
        /// This method provides access to the values in each of the columns in a given row.
        /// This method makes casts unnecessary when accessing columns.
        /// Additionally, Field supports nullable types and maps automatically between DBNull and
        /// Nullable when the generic type is nullable.
        /// </summary>
        /// <param name="row">The input DataRow</param>
        /// <param name="columnIndex">The input ordinal specifying which row value to retrieve.</param>
        /// <param name="version">The DataRow version for which row value to retrieve.</param>
        /// <returns>The DataRow value for the column specified.</returns>
        [return: MaybeNull]
        public static T Field<T>(this DataRow row, int columnIndex, DataRowVersion version)
        {
            DataSetUtil.CheckArgumentNull(row, nameof(row));
            return UnboxT<T>.s_unbox(row[columnIndex, version]);
        }

        /// <summary>
        /// This method provides access to the values in each of the columns in a given row.
        /// This method makes casts unnecessary when accessing columns.
        /// Additionally, Field supports nullable types and maps automatically between DBNull and
        /// Nullable when the generic type is nullable.
        /// </summary>
        /// <param name="row">The input DataRow</param>
        /// <param name="columnName">The input column name specifying which row value to retrieve.</param>
        /// <param name="version">The DataRow version for which row value to retrieve.</param>
        /// <returns>The DataRow value for the column specified.</returns>
        [return: MaybeNull]
        public static T Field<T>(this DataRow row, string columnName, DataRowVersion version)
        {
            DataSetUtil.CheckArgumentNull(row, nameof(row));
            return UnboxT<T>.s_unbox(row[columnName, version]);
        }

        /// <summary>
        /// This method provides access to the values in each of the columns in a given row.
        /// This method makes casts unnecessary when accessing columns.
        /// Additionally, Field supports nullable types and maps automatically between DBNull and
        /// Nullable when the generic type is nullable.
        /// </summary>
        /// <param name="row">The input DataRow</param>
        /// <param name="column">The input DataColumn specifying which row value to retrieve.</param>
        /// <param name="version">The DataRow version for which row value to retrieve.</param>
        /// <returns>The DataRow value for the column specified.</returns>
        [return: MaybeNull]
        public static T Field<T>(this DataRow row, DataColumn column, DataRowVersion version)
        {
            DataSetUtil.CheckArgumentNull(row, nameof(row));
            return UnboxT<T>.s_unbox(row[column, version]);
        }

        /// <summary>
        /// This method sets a new value for the specified column for the DataRow it's called on.
        /// </summary>
        /// <param name="row">The input DataRow.</param>
        /// <param name="columnIndex">The input ordinal specifying which row value to set.</param>
        /// <param name="value">The new row value for the specified column.</param>
        public static void SetField<T>(this DataRow row, int columnIndex, [AllowNull] T value)
        {
            DataSetUtil.CheckArgumentNull(row, nameof(row));
            row[columnIndex] = (object?)value ?? DBNull.Value;
        }

        /// <summary>
        /// This method sets a new value for the specified column for the DataRow it's called on.
        /// </summary>
        /// <param name="row">The input DataRow.</param>
        /// <param name="columnName">The input column name specifying which row value to retrieve.</param>
        /// <param name="value">The new row value for the specified column.</param>
        public static void SetField<T>(this DataRow row, string columnName, [AllowNull] T value)
        {
            DataSetUtil.CheckArgumentNull(row, nameof(row));
            row[columnName] = (object?)value ?? DBNull.Value;
        }

        /// <summary>
        /// This method sets a new value for the specified column for the DataRow it's called on.
        /// </summary>
        /// <param name="row">The input DataRow.</param>
        /// <param name="column">The input DataColumn specifying which row value to retrieve.</param>
        /// <param name="value">The new row value for the specified column.</param>
        public static void SetField<T>(this DataRow row, DataColumn column, [AllowNull] T value)
        {
            DataSetUtil.CheckArgumentNull(row, nameof(row));
            row[column] = (object?)value ?? DBNull.Value;
        }

        private static class UnboxT<T>
        {
            internal static readonly Converter<object, T> s_unbox = Create();

            private static Converter<object, T> Create()
            {
                if (typeof(T).IsValueType)
                {
                    return typeof(T).IsGenericType && typeof(T).GetGenericTypeDefinition() == typeof(Nullable<>)
                        ? (Converter<object, T>)Delegate.CreateDelegate(
                            typeof(Converter<object, T>),
                                typeof(UnboxT<T>)
                                    .GetMethod("NullableField", System.Reflection.BindingFlags.Static | System.Reflection.BindingFlags.NonPublic)!
                                    .MakeGenericMethod(typeof(T).GetGenericArguments()[0]))
                        : ValueField;
                }

                return ReferenceField;
            }

            [return: MaybeNull]
            private static T ReferenceField(object value)
                => value == DBNull.Value ? default : (T)value;

            private static T ValueField(object value)
                => value == DBNull.Value
                    ? throw DataSetUtil.InvalidCast(SR.Format(SR.DataSetLinq_NonNullableCast, typeof(T)))
                    : (T)value;

            [return: MaybeNull]
            private static Nullable<TElem> NullableField<TElem>(object value) where TElem : struct
                => value == DBNull.Value ? default : new Nullable<TElem>((TElem)value);
        }
    }
}
