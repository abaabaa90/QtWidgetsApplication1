//// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
//// SPDX-License-Identifier: BSD-3-Clause
///**
// * @file   vtkStringScanner.h
// * @brief  优化的C++工具，用于从字符串和文件中扫描值。
// *
// * 这个头文件提供了高效的、替代常见C/C++字符串处理函数（如`scanf`、`atoi`等）的工具。
// *
// * 它包含将字符串转换为数字以及从字符串和文件扫描值的实用工具。
// *
// * 请参考文档以获取有关如何使用这里提供的现代、类型安全对应函数替换标准C函数的指导。
// *
// * 1. C/C++有以下函数可以将一个/多个char*或string转换为数字：
// *    1. atof, atoi, atol, atoll,
// *    2. std::stof, std::stod, std::stold, std::stoi, std::stol, std::stoll, std::stoul, std::stoull
// *    3. strtof, strtod, strtold, strtol, strtoll/_strtoi64, strtoul, strtoull
// *    4. sscanf, sscanf_s, vsscanf, vsscanf_s
// *    5. strptime
// *    6. std::from_chars
// *    7. std::get_time
// * 这些函数应该被替换为：
// *    1. vtk::from_chars, vtk::scan_int, vtk::scan_value, 如果需要转换一个数字
// *    2. vtk::scan, 如果需要转换一个/多个数字（可选使用特定格式）
// *
// * 2. C/C++有以下函数从stdin/文件扫描一个/多个数字：
// *    1. scanf, scanf_s, vscanf, vscanf_s,
// *    2. fscanf, fscanf_s, vfscanf, vfscanf_s,
// *    3. std::cin, std::ifstream, vtksys::ifstream
// * 这些函数应该被替换为：
// *    1. vtk::scan_value, 如果需要转换一个数字
// *    2. vtk::input, vtk::scan, 如果需要转换一个/多个数字（可选使用特定格式）
// */
//#ifndef vtkStringScanner_h
//#define vtkStringScanner_h
//
//#include "vtkCharConvCompatibility.h" // For std::chars_format, std::from_chars_result
//#include "vtkCommonCoreModule.h"      // For export macro
//#include "vtkLogger.h"                // For vtkLogF
//
//#include "vtkfast_float.h" // For fast_float
//
//#include "vtk_scn.h" // For scn
// // clang-format off
//#include VTK_SCN(scn/chrono.h) // For scn's chrono
//#include VTK_SCN(scn/scan.h)   // For scn's scan
//// clang-format on
//
//#include <array>       // For std::array
//#include <string_view> // For std::string_view
//
//namespace vtk
//{
//    VTK_ABI_NAMESPACE_BEGIN
//        ///@{
//        /**
//         * Given a char* first and char* last, convert it to a number, and return a from_chars_result;
//         *
//         * @note The standard library provides these functions as of C++17, but they aren't as fast,
//         * or fully implemented.
//         */
//        template <typename T, typename = FASTFLOAT_ENABLE_IF(fast_float::is_supported_float_type<T>::value)>
//    VTK_ALWAYS_INLINE auto from_chars(const char* first, const char* last, T& value,
//        std::chars_format format = std::chars_format::general) -> std::from_chars_result
//    {
//        static constexpr std::array<fast_float::chars_format, 5> std_to_fast_float_chars_format = { {//静态常量数组用于格式映射
//          fast_float::chars_format::general,    // 0: Default general
//          fast_float::chars_format::scientific, // 1: scientific
//          fast_float::chars_format::fixed,      // 2: fixed
//          fast_float::chars_format::general,    // 3: general
//          fast_float::chars_format::hex,        // 4: hex
//        } };
//        auto result = fast_float::from_chars<T>(first, last, value,
//            std_to_fast_float_chars_format[static_cast<std::underlying_type_t<std::chars_format>>(format)]);
//        return { result.ptr, result.ec };
//    }
//    template <typename T,
//        typename = FASTFLOAT_ENABLE_IF(fast_float::is_supported_integer_type<T>::value)>
//    VTK_ALWAYS_INLINE auto from_chars(const char* first, const char* last, T& value, int base = 10)
//        -> std::from_chars_result
//    {
//        auto result = fast_float::from_chars<T>(first, last, value, base);
//        return { result.ptr, result.ec };
//    }
//    ///@}
//
//    ///@{
//    /**
//     * Given a std::string_view str, convert it to a number, and return a from_chars_result;
//     *
//     * @note The standard library does not provide these functions.
//     */
//    template <typename T, typename = FASTFLOAT_ENABLE_IF(fast_float::is_supported_float_type<T>::value)>
//    VTK_ALWAYS_INLINE auto from_chars(const std::string_view str, T& value,
//        std::chars_format format = std::chars_format::general) -> std::from_chars_result
//    {
//        return vtk::from_chars<T>(str.data(), str.data() + str.size(), value, format);
//    }
//    template <typename T,
//        typename = FASTFLOAT_ENABLE_IF(fast_float::is_supported_integer_type<T>::value)>
//    VTK_ALWAYS_INLINE auto from_chars(const std::string_view str, T& value, int base = 10)
//        -> std::from_chars_result
//    {
//        return vtk::from_chars<T>(str.data(), str.data() + str.size(), value, base);
//    }
//    ///@}
//
//    // Create a macro that evaluates the result of from_chars, checks for errors, and executes a command
//#define VTK_FROM_CHARS_RESULT_IF_ERROR_COMMAND(from_chars_result, value, command)                  \
//  switch (from_chars_result.ec)                                                                    \
//  {                                                                                                \
//    case std::errc::invalid_argument:                                                              \
//    {                                                                                              \
//      vtkLogF(ERROR, "The given argument was invalid, failed to get the converted " #value ".");   \
//      command;                                                                                     \
//    }                                                                                              \
//    case std::errc::result_out_of_range:                                                           \
//    {                                                                                              \
//      vtkLogF(ERROR, "The result is out of range, failed to get the converted " #value ".");       \
//      command;                                                                                     \
//    }                                                                                              \
//    default:                                                                                       \
//    {                                                                                              \
//    }                                                                                              \
//  }
//// Create a macro that evaluates the result of from_chars, checks for errors, and breaks
//#define VTK_FROM_CHARS_RESULT_IF_ERROR_BREAK(from_chars_result, value)                             \
//  VTK_FROM_CHARS_RESULT_IF_ERROR_COMMAND(from_chars_result, value, break)
//// Create a macro that evaluates the result of from_chars, checks for errors, and returns a value
//#define VTK_FROM_CHARS_RESULT_IF_ERROR_RETURN(from_chars_result, value, returnValue)               \
//  VTK_FROM_CHARS_RESULT_IF_ERROR_COMMAND(from_chars_result, value, return returnValue)
//
//// Helper for token‐pasting
//#define VTK_FROM_CHARS_CONCAT_INNER(a, b) a##b
//#define VTK_FROM_CHARS_CONCAT(a, b) VTK_FROM_CHARS_CONCAT_INNER(a, b)
//
//// Create a macro that executes from_chars, checks for errors, and executes a command
//#define VTK_FROM_CHARS_IF_ERROR_COMMAND(string, value, command)                                    \
//  auto VTK_FROM_CHARS_CONCAT(_from_chars_result_, __LINE__) = vtk::from_chars(string, value);      \
//  VTK_FROM_CHARS_RESULT_IF_ERROR_COMMAND(                                                          \
//    VTK_FROM_CHARS_CONCAT(_from_chars_result_, __LINE__), value, command)
//// Create a macro that executes from_chars, checks for errors, and breaks
//#define VTK_FROM_CHARS_IF_ERROR_BREAK(string, value)                                               \
//  VTK_FROM_CHARS_IF_ERROR_COMMAND(string, value, break)
//// Create a macro that executes from_chars, checks for errors, and returns a value
//#define VTK_FROM_CHARS_IF_ERROR_RETURN(string, value, returnValue)                                 \
//  VTK_FROM_CHARS_IF_ERROR_COMMAND(string, value, return returnValue)
//
//// Create a macro that executes from_chars with a param, checks for errors, and executes a command
//#define VTK_FROM_CHARS_WITH_PARAM_IF_ERROR_COMMAND(string, value, param, command)                  \
//  auto VTK_FROM_CHARS_CONCAT(_from_chars_result_, __LINE__) =                                      \
//    vtk::from_chars(string, value, param);                                                         \
//  VTK_FROM_CHARS_RESULT_IF_ERROR_COMMAND(                                                          \
//    VTK_FROM_CHARS_CONCAT(_from_chars_result_, __LINE__), value, command)
//// Create a macro that executes from_chars with param, checks for errors, and breaks
//#define VTK_FROM_CHARS_WITH_PARAM_IF_ERROR_BREAK(string, value, param)                             \
//  VTK_FROM_CHARS_WITH_PARAM_IF_ERROR_COMMAND(string, value, param, break)
//// Create a macro that executes from_chars with param, checks for errors, and returns a value
//#define VTK_FROM_CHARS_WITH_PARAM_IF_ERROR_RETURN(string, value, param, returnValue)               \
//  VTK_FROM_CHARS_WITH_PARAM_IF_ERROR_COMMAND(string, value, param, return returnValue)
//
///**
// * The result type of a scan operation.
// */
//    using scn::scan_result_type;
//
//    ///@{
//    /**
//     * Given a source (string, stdin, file) convert it to a number, and return a scan_result;
//     *
//     * @note The standard library does not provide this function.
//     */
//    using scn::scan_int;
//    using scn::scan_value;
//    ///@}
//
//    /**
//     * Given a source (string, stdin, file) and a format string, convert it to variables, and return a
//     * scan_result;
//     *
//     * @note The standard library does not provide this function.
//     */
//    using scn::scan;
//
//    /**
//     * With stdin as source and a format string, convert it to variables, and return a scan_result;
//     *
//     * @note The standard library does not provide this function.
//     */
//    using scn::input;
//
//    VTK_ABI_NAMESPACE_END
//}
//
//#endif // vtkStringScanner_h
//// VTK-HeaderTest-Exclude: vtkStringScanner.h