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

#pragma once
#ifndef TESTING_GATHER_HPP
#define TESTING_GATHER_HPP

#include "hipsparse_test_unique_ptr.hpp"
#include "unit.hpp"
#include "utility.hpp"

#include <hipsparse.h>
#include <typeinfo>

using namespace hipsparse_test;

void testing_gather_bad_arg(void)
{
#if(!defined(CUDART_VERSION) || CUDART_VERSION >= 11000)
    int64_t size = 100;
    int64_t nnz  = 100;

    hipsparseIndexType_t idxType  = HIPSPARSE_INDEX_32I;
    hipsparseIndexBase_t idxBase  = HIPSPARSE_INDEX_BASE_ZERO;
    hipDataType          dataType = HIP_R_32F;

    std::unique_ptr<handle_struct> unique_ptr_handle(new handle_struct);
    hipsparseHandle_t              handle = unique_ptr_handle->handle;

    auto dx_val_managed = hipsparse_unique_ptr{device_malloc(sizeof(float) * nnz), device_free};
    auto dx_ind_managed = hipsparse_unique_ptr{device_malloc(sizeof(int) * nnz), device_free};
    auto dy_managed     = hipsparse_unique_ptr{device_malloc(sizeof(float) * size), device_free};

    float* dx_val = (float*)dx_val_managed.get();
    int*   dx_ind = (int*)dx_ind_managed.get();
    float* dy     = (float*)dy_managed.get();

    if(!dx_ind || !dx_val || !dy)
    {
        PRINT_IF_HIP_ERROR(hipErrorOutOfMemory);
        return;
    }

    // Structures
    hipsparseSpVecDescr_t x;
    hipsparseDnVecDescr_t y;

    verify_hipsparse_status_success(
        hipsparseCreateSpVec(&x, size, nnz, dx_ind, dx_val, idxType, idxBase, dataType), "Success");
    verify_hipsparse_status_success(hipsparseCreateDnVec(&y, size, dy, dataType), "Success");

    // Gather
    verify_hipsparse_status_invalid_handle(hipsparseGather(nullptr, y, x));
    verify_hipsparse_status_invalid_pointer(hipsparseGather(handle, nullptr, x),
                                            "Error: y is nullptr");
    verify_hipsparse_status_invalid_pointer(hipsparseGather(handle, y, nullptr),
                                            "Error: x is nullptr");

    // Destruct
    verify_hipsparse_status_success(hipsparseDestroySpVec(x), "Success");
    verify_hipsparse_status_success(hipsparseDestroyDnVec(y), "Success");
#endif
}

template <typename I, typename T>
hipsparseStatus_t testing_gather(void)
{
#if(!defined(CUDART_VERSION) || CUDART_VERSION >= 11000)
    int64_t size = 15332;
    int64_t nnz  = 500;

    hipsparseIndexBase_t idxBase = HIPSPARSE_INDEX_BASE_ZERO;

    // Index and data type
    hipsparseIndexType_t idxType
        = (typeid(I) == typeid(int32_t)) ? HIPSPARSE_INDEX_32I : HIPSPARSE_INDEX_64I;
    hipDataType dataType
        = (typeid(T) == typeid(float))
              ? HIP_R_32F
              : ((typeid(T) == typeid(double))
                     ? HIP_R_64F
                     : ((typeid(T) == typeid(hipComplex) ? HIP_C_32F : HIP_C_64F)));

    // hipSPARSE handle
    std::unique_ptr<handle_struct> test_handle(new handle_struct);
    hipsparseHandle_t              handle = test_handle->handle;

    // Host structures
    std::vector<I> hx_ind(nnz);
    std::vector<T> hx_val(nnz);
    std::vector<T> hx_val_gold(nnz);
    std::vector<T> hy(size);

    // Initial Data on CPU
    srand(12345ULL);
    hipsparseInitIndex(hx_ind.data(), nnz, 1, size);
    hipsparseInit<T>(hy, 1, size);

    // Allocate memory on device
    auto dx_ind_managed = hipsparse_unique_ptr{device_malloc(sizeof(I) * nnz), device_free};
    auto dx_val_managed = hipsparse_unique_ptr{device_malloc(sizeof(T) * nnz), device_free};
    auto dy_managed     = hipsparse_unique_ptr{device_malloc(sizeof(T) * size), device_free};

    I* dx_ind = (I*)dx_ind_managed.get();
    T* dx_val = (T*)dx_val_managed.get();
    T* dy     = (T*)dy_managed.get();

    if(!dx_ind || !dx_val || !dy)
    {
        verify_hipsparse_status_success(HIPSPARSE_STATUS_ALLOC_FAILED, "!dx_ind || !dx_val || !dy");
        return HIPSPARSE_STATUS_ALLOC_FAILED;
    }

    // copy data from CPU to device
    CHECK_HIP_ERROR(hipMemcpy(dx_ind, hx_ind.data(), sizeof(I) * nnz, hipMemcpyHostToDevice));
    CHECK_HIP_ERROR(hipMemcpy(dy, hy.data(), sizeof(T) * size, hipMemcpyHostToDevice));

    // Create structures
    hipsparseSpVecDescr_t x;
    hipsparseDnVecDescr_t y;

    CHECK_HIPSPARSE_ERROR(
        hipsparseCreateSpVec(&x, size, nnz, dx_ind, dx_val, idxType, idxBase, dataType));
    CHECK_HIPSPARSE_ERROR(hipsparseCreateDnVec(&y, size, dy, dataType));

    // Gather
    CHECK_HIPSPARSE_ERROR(hipsparseGather(handle, y, x));

    // Copy output from device to CPU
    CHECK_HIP_ERROR(hipMemcpy(hx_val.data(), dx_val, sizeof(T) * nnz, hipMemcpyDeviceToHost));

    // CPU
    for(int64_t i = 0; i < nnz; ++i)
    {
        hx_val_gold[i] = hy[hx_ind[i] - idxBase];
    }

    // Verify results against host
    unit_check_general(1, nnz, 1, hx_val_gold.data(), hx_val.data());

    CHECK_HIPSPARSE_ERROR(hipsparseDestroySpVec(x));
    CHECK_HIPSPARSE_ERROR(hipsparseDestroyDnVec(y));
#endif

    return HIPSPARSE_STATUS_SUCCESS;
}

#endif // TESTING_GATHER_HPP
