/* ************************************************************************
 * Copyright (c) 2021 Advanced Micro Devices, Inc.
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

#include "testing_spgemmreuse_csr.hpp"

#include <hipsparse.h>

#if(!defined(CUDART_VERSION) || CUDART_VERSION >= 11031)
TEST(spgemmreuse_csr_bad_arg, spgemmreuse_csr_float)
{
    testing_spgemmreuse_csr_bad_arg();
}

TEST(spgemmreuse_csr, spgemmreuse_csr_i32_i32_float)
{
    hipsparseStatus_t status = testing_spgemmreuse_csr<int32_t, int32_t, float>();
    EXPECT_EQ(status, HIPSPARSE_STATUS_SUCCESS);
}

TEST(spgemmreuse_csr, spgemmreuse_csr_i32_i32_double)
{
    hipsparseStatus_t status = testing_spgemmreuse_csr<int32_t, int32_t, double>();
    EXPECT_EQ(status, HIPSPARSE_STATUS_SUCCESS);
}

// 64 bit indices not supported in cusparse
#if(!defined(CUDART_VERSION))
TEST(spgemmreuse_csr, spgemmreuse_csr_i64_i32_double)
{
    hipsparseStatus_t status = testing_spgemmreuse_csr<int64_t, int32_t, double>();
    EXPECT_EQ(status, HIPSPARSE_STATUS_SUCCESS);
}

TEST(spgemmreuse_csr, spgemmreuse_csr_i64_i64_hipComplex)
{
    hipsparseStatus_t status = testing_spgemmreuse_csr<int64_t, int64_t, hipComplex>();
    EXPECT_EQ(status, HIPSPARSE_STATUS_SUCCESS);
}

TEST(spgemmreuse_csr, spgemmreuse_csr_i64_i64_hipDoubleComplex)
{
    hipsparseStatus_t status = testing_spgemmreuse_csr<int64_t, int64_t, hipDoubleComplex>();
    EXPECT_EQ(status, HIPSPARSE_STATUS_SUCCESS);
}
#endif
#endif
