/* ************************************************************************
 * Copyright (c) 2018-2019 Advanced Micro Devices, Inc.
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
#ifndef TESTING_GTHR_HPP
#define TESTING_GTHR_HPP

#include "hipsparse.hpp"
#include "hipsparse_test_unique_ptr.hpp"
#include "unit.hpp"
#include "utility.hpp"

#include <hipsparse.h>

using namespace hipsparse;
using namespace hipsparse_test;

template <typename T>
void testing_gthr_bad_arg(void)
{
    int nnz       = 100;
    int safe_size = 100;

    hipsparseIndexBase_t idx_base = HIPSPARSE_INDEX_BASE_ZERO;
    hipsparseStatus_t    status;

    std::unique_ptr<handle_struct> unique_ptr_handle(new handle_struct);
    hipsparseHandle_t              handle = unique_ptr_handle->handle;

    auto dx_val_managed = hipsparse_unique_ptr{device_malloc(sizeof(T) * safe_size), device_free};
    auto dx_ind_managed = hipsparse_unique_ptr{device_malloc(sizeof(int) * safe_size), device_free};
    auto dy_managed     = hipsparse_unique_ptr{device_malloc(sizeof(T) * safe_size), device_free};

    T*   dx_val = (T*)dx_val_managed.get();
    int* dx_ind = (int*)dx_ind_managed.get();
    T*   dy     = (T*)dy_managed.get();

    if(!dx_ind || !dx_val || !dy)
    {
        PRINT_IF_HIP_ERROR(hipErrorOutOfMemory);
        return;
    }

#if(!defined(CUDART_VERSION))
    // testing for(nullptr == dx_ind)
    {
        int* dx_ind_null = nullptr;

        status = hipsparseXgthr(handle, nnz, dy, dx_val, dx_ind_null, idx_base);
        verify_hipsparse_status_invalid_pointer(status, "Error: x_ind is nullptr");
    }
    // testing for(nullptr == dx_val)
    {
        T* dx_val_null = nullptr;

        status = hipsparseXgthr(handle, nnz, dy, dx_val_null, dx_ind, idx_base);
        verify_hipsparse_status_invalid_pointer(status, "Error: x_val is nullptr");
    }
    // testing for(nullptr == dy)
    {
        T* dy_null = nullptr;

        status = hipsparseXgthr(handle, nnz, dy_null, dx_val, dx_ind, idx_base);
        verify_hipsparse_status_invalid_pointer(status, "Error: y is nullptr");
    }
    // testing for(nullptr == handle)
    {
        hipsparseHandle_t handle_null = nullptr;

        status = hipsparseXgthr(handle_null, nnz, dy, dx_val, dx_ind, idx_base);
        verify_hipsparse_status_invalid_handle(status);
    }
#endif
}

template <typename T>
hipsparseStatus_t testing_gthr(Arguments argus)
{
    int                  N         = argus.N;
    int                  nnz       = argus.nnz;
    int                  safe_size = 100;
    hipsparseIndexBase_t idx_base  = argus.idx_base;
    hipsparseStatus_t    status;

    std::unique_ptr<handle_struct> test_handle(new handle_struct);
    hipsparseHandle_t              handle = test_handle->handle;

    // Argument sanity check before allocating invalid memory
    if(nnz <= 0)
    {
        auto dx_ind_managed
            = hipsparse_unique_ptr{device_malloc(sizeof(int) * safe_size), device_free};
        auto dx_val_managed
            = hipsparse_unique_ptr{device_malloc(sizeof(T) * safe_size), device_free};
        auto dy_managed = hipsparse_unique_ptr{device_malloc(sizeof(T) * safe_size), device_free};

        int* dx_ind = (int*)dx_ind_managed.get();
        T*   dx_val = (T*)dx_val_managed.get();
        T*   dy     = (T*)dy_managed.get();

        if(!dx_ind || !dx_val || !dy)
        {
            verify_hipsparse_status_success(HIPSPARSE_STATUS_ALLOC_FAILED,
                                            "!dx_ind || !dx_val || !dy");
            return HIPSPARSE_STATUS_ALLOC_FAILED;
        }

        CHECK_HIPSPARSE_ERROR(hipsparseSetPointerMode(handle, HIPSPARSE_POINTER_MODE_HOST));
        status = hipsparseXgthr(handle, nnz, dy, dx_val, dx_ind, idx_base);

        if(nnz < 0)
        {
            verify_hipsparse_status_invalid_size(status, "Error: nnz < 0");
        }
        else
        {
            verify_hipsparse_status_success(status, "nnz == 0");
        }

        return HIPSPARSE_STATUS_SUCCESS;
    }

    // Host structures
    std::vector<int> hx_ind(nnz);
    std::vector<T>   hx_val(nnz);
    std::vector<T>   hx_val_gold(nnz);
    std::vector<T>   hy(N);

    // Initial Data on CPU
    srand(12345ULL);
    hipsparseInitIndex(hx_ind.data(), nnz, 1, N);
    hipsparseInit<T>(hy, 1, N);

    // allocate memory on device
    auto dx_ind_managed = hipsparse_unique_ptr{device_malloc(sizeof(int) * nnz), device_free};
    auto dx_val_managed = hipsparse_unique_ptr{device_malloc(sizeof(T) * nnz), device_free};
    auto dy_managed     = hipsparse_unique_ptr{device_malloc(sizeof(T) * N), device_free};

    int* dx_ind = (int*)dx_ind_managed.get();
    T*   dx_val = (T*)dx_val_managed.get();
    T*   dy     = (T*)dy_managed.get();

    if(!dx_ind || !dx_val || !dy)
    {
        verify_hipsparse_status_success(HIPSPARSE_STATUS_ALLOC_FAILED, "!dx_ind || !dx_val || !dy");
        return HIPSPARSE_STATUS_ALLOC_FAILED;
    }

    // copy data from CPU to device
    CHECK_HIP_ERROR(hipMemcpy(dx_ind, hx_ind.data(), sizeof(int) * nnz, hipMemcpyHostToDevice));
    CHECK_HIP_ERROR(hipMemcpy(dy, hy.data(), sizeof(T) * N, hipMemcpyHostToDevice));

    if(argus.unit_check)
    {
        // ROCSPARSE pointer mode host
        CHECK_HIPSPARSE_ERROR(hipsparseSetPointerMode(handle, HIPSPARSE_POINTER_MODE_HOST));
        CHECK_HIPSPARSE_ERROR(hipsparseXgthr(handle, nnz, dy, dx_val, dx_ind, idx_base));

        // copy output from device to CPU
        CHECK_HIP_ERROR(hipMemcpy(hx_val.data(), dx_val, sizeof(T) * nnz, hipMemcpyDeviceToHost));

        // CPU
        for(int i = 0; i < nnz; ++i)
        {
            hx_val_gold[i] = hy[hx_ind[i] - idx_base];
        }

        // enable unit check, notice unit check is not invasive, but norm check is,
        // unit check and norm check can not be interchanged their order
        unit_check_general(1, nnz, 1, hx_val_gold.data(), hx_val.data());
    }

    return HIPSPARSE_STATUS_SUCCESS;
}

#endif // TESTING_GTHR_HPP
