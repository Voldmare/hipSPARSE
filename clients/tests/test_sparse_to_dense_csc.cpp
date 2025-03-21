/* ************************************************************************
 * Copyright (c) 2020 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * ************************************************************************ */

#include "testing_sparse_to_dense_csc.hpp"

#include <hipsparse.h>

// Only run tests for CUDA 11.1 or greater
#if(!defined(CUDART_VERSION) || CUDART_VERSION >= 11010)
TEST(sparse_to_dense_csc_bad_arg, sparse_to_dense_csc_float)
{
    testing_sparse_to_dense_csc_bad_arg();
}

TEST(sparse_to_dense_csc, sparse_to_dense_csc_i32_i32_float)
{
    hipsparseStatus_t status = testing_sparse_to_dense_csc<int32_t, int32_t, float>();
    EXPECT_EQ(status, HIPSPARSE_STATUS_SUCCESS);
}

#if(!defined(CUDART_VERSION))
TEST(sparse_to_dense_csc, sparse_to_dense_csc_i64_i32_double)
{
    hipsparseStatus_t status = testing_sparse_to_dense_csc<int64_t, int32_t, double>();
    EXPECT_EQ(status, HIPSPARSE_STATUS_SUCCESS);
}
#else
TEST(sparse_to_dense_csc, sparse_to_dense_csc_i64_i64_double)
{
    hipsparseStatus_t status = testing_sparse_to_dense_csc<int64_t, int64_t, double>();
    EXPECT_EQ(status, HIPSPARSE_STATUS_SUCCESS);
}
#endif

TEST(sparse_to_dense_csc, sparse_to_dense_csc_i64_i64_hipComplex)
{
    hipsparseStatus_t status = testing_sparse_to_dense_csc<int64_t, int64_t, hipComplex>();
    EXPECT_EQ(status, HIPSPARSE_STATUS_SUCCESS);
}
#endif