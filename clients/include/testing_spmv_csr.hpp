/* ************************************************************************
 * Copyright (c) 2022 Advanced Micro Devices, Inc.
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
#ifndef TESTING_SPMV_CSR_HPP
#define TESTING_SPMV_CSR_HPP

#include "hipsparse_test_unique_ptr.hpp"
#include "unit.hpp"
#include "utility.hpp"

#include <hipsparse.h>
#include <string>
#include <typeinfo>

using namespace hipsparse_test;

void testing_spmv_csr_bad_arg(void)
{
#if(!defined(CUDART_VERSION) || CUDART_VERSION >= 10010)
    int64_t              m         = 100;
    int64_t              n         = 100;
    int64_t              nnz       = 100;
    int64_t              safe_size = 100;
    float                alpha     = 0.6;
    float                beta      = 0.2;
    hipsparseOperation_t transA    = HIPSPARSE_OPERATION_NON_TRANSPOSE;
    hipsparseIndexBase_t idxBase   = HIPSPARSE_INDEX_BASE_ZERO;
    hipsparseIndexType_t idxType   = HIPSPARSE_INDEX_32I;
    hipDataType          dataType  = HIP_R_32F;
    hipsparseSpMVAlg_t   alg       = HIPSPARSE_MV_ALG_DEFAULT;

    std::unique_ptr<handle_struct> unique_ptr_handle(new handle_struct);
    hipsparseHandle_t              handle = unique_ptr_handle->handle;

    auto dptr_managed = hipsparse_unique_ptr{device_malloc(sizeof(int) * safe_size), device_free};
    auto dcol_managed = hipsparse_unique_ptr{device_malloc(sizeof(int) * safe_size), device_free};
    auto dval_managed = hipsparse_unique_ptr{device_malloc(sizeof(float) * safe_size), device_free};
    auto dx_managed   = hipsparse_unique_ptr{device_malloc(sizeof(float) * safe_size), device_free};
    auto dy_managed   = hipsparse_unique_ptr{device_malloc(sizeof(float) * safe_size), device_free};
    auto dbuf_managed = hipsparse_unique_ptr{device_malloc(sizeof(char) * safe_size), device_free};

    int*   dptr = (int*)dptr_managed.get();
    int*   dcol = (int*)dcol_managed.get();
    float* dval = (float*)dval_managed.get();
    float* dx   = (float*)dx_managed.get();
    float* dy   = (float*)dy_managed.get();
    void*  dbuf = (void*)dbuf_managed.get();

    if(!dval || !dptr || !dcol || !dx || !dy || !dbuf)
    {
        PRINT_IF_HIP_ERROR(hipErrorOutOfMemory);
        return;
    }

    // SpMV structures
    hipsparseSpMatDescr_t A;
    hipsparseDnVecDescr_t x, y;

    size_t bsize;

    // Create SpMV structures
    verify_hipsparse_status_success(
        hipsparseCreateCsr(&A, m, n, nnz, dptr, dcol, dval, idxType, idxType, idxBase, dataType),
        "success");
    verify_hipsparse_status_success(hipsparseCreateDnVec(&x, n, dx, dataType), "success");
    verify_hipsparse_status_success(hipsparseCreateDnVec(&y, m, dy, dataType), "success");

    // SpMV buffer
    verify_hipsparse_status_invalid_handle(
        hipsparseSpMV_bufferSize(nullptr, transA, &alpha, A, x, &beta, y, dataType, alg, &bsize));
    verify_hipsparse_status_invalid_pointer(
        hipsparseSpMV_bufferSize(handle, transA, nullptr, A, x, &beta, y, dataType, alg, &bsize),
        "Error: alpha is nullptr");
    verify_hipsparse_status_invalid_pointer(
        hipsparseSpMV_bufferSize(
            handle, transA, &alpha, nullptr, x, &beta, y, dataType, alg, &bsize),
        "Error: A is nullptr");
    verify_hipsparse_status_invalid_pointer(
        hipsparseSpMV_bufferSize(
            handle, transA, &alpha, A, nullptr, &beta, y, dataType, alg, &bsize),
        "Error: x is nullptr");
    verify_hipsparse_status_invalid_pointer(
        hipsparseSpMV_bufferSize(handle, transA, &alpha, A, x, nullptr, y, dataType, alg, &bsize),
        "Error: beta is nullptr");
    verify_hipsparse_status_invalid_pointer(
        hipsparseSpMV_bufferSize(
            handle, transA, &alpha, A, x, &beta, nullptr, dataType, alg, &bsize),
        "Error: y is nullptr");
    verify_hipsparse_status_invalid_pointer(
        hipsparseSpMV_bufferSize(handle, transA, &alpha, A, x, &beta, y, dataType, alg, nullptr),
        "Error: bsize is nullptr");

    // SpMV preprocess (optional)
    verify_hipsparse_status_invalid_handle(
        hipsparseSpMV_preprocess(nullptr, transA, &alpha, A, x, &beta, y, dataType, alg, dbuf));
    verify_hipsparse_status_invalid_pointer(
        hipsparseSpMV_preprocess(handle, transA, nullptr, A, x, &beta, y, dataType, alg, dbuf),
        "Error: alpha is nullptr");
    verify_hipsparse_status_invalid_pointer(
        hipsparseSpMV_preprocess(handle, transA, &alpha, nullptr, x, &beta, y, dataType, alg, dbuf),
        "Error: A is nullptr");
    verify_hipsparse_status_invalid_pointer(
        hipsparseSpMV_preprocess(handle, transA, &alpha, A, nullptr, &beta, y, dataType, alg, dbuf),
        "Error: x is nullptr");
    verify_hipsparse_status_invalid_pointer(
        hipsparseSpMV_preprocess(handle, transA, &alpha, A, x, nullptr, y, dataType, alg, dbuf),
        "Error: beta is nullptr");
    verify_hipsparse_status_invalid_pointer(
        hipsparseSpMV_preprocess(handle, transA, &alpha, A, x, &beta, nullptr, dataType, alg, dbuf),
        "Error: y is nullptr");
    verify_hipsparse_status_invalid_pointer(
        hipsparseSpMV_preprocess(
            handle, transA, &alpha, A, x, &beta, nullptr, dataType, alg, nullptr),
        "Error: dbuf is nullptr");

    // SpMV
    verify_hipsparse_status_invalid_handle(
        hipsparseSpMV(nullptr, transA, &alpha, A, x, &beta, y, dataType, alg, dbuf));
    verify_hipsparse_status_invalid_pointer(
        hipsparseSpMV(handle, transA, nullptr, A, x, &beta, y, dataType, alg, dbuf),
        "Error: alpha is nullptr");
    verify_hipsparse_status_invalid_pointer(
        hipsparseSpMV(handle, transA, &alpha, nullptr, x, &beta, y, dataType, alg, dbuf),
        "Error: A is nullptr");
    verify_hipsparse_status_invalid_pointer(
        hipsparseSpMV(handle, transA, &alpha, A, nullptr, &beta, y, dataType, alg, dbuf),
        "Error: x is nullptr");
    verify_hipsparse_status_invalid_pointer(
        hipsparseSpMV(handle, transA, &alpha, A, x, nullptr, y, dataType, alg, dbuf),
        "Error: beta is nullptr");
    verify_hipsparse_status_invalid_pointer(
        hipsparseSpMV(handle, transA, &alpha, A, x, &beta, nullptr, dataType, alg, dbuf),
        "Error: y is nullptr");
    verify_hipsparse_status_invalid_pointer(
        hipsparseSpMV(handle, transA, &alpha, A, x, &beta, nullptr, dataType, alg, nullptr),
        "Error: dbuf is nullptr");

    // Destruct
    verify_hipsparse_status_success(hipsparseDestroySpMat(A), "success");
    verify_hipsparse_status_success(hipsparseDestroyDnVec(x), "success");
    verify_hipsparse_status_success(hipsparseDestroyDnVec(y), "success");
#endif
}

template <typename I, typename J, typename T>
hipsparseStatus_t testing_spmv_csr(void)
{
#if(!defined(CUDART_VERSION) || CUDART_VERSION >= 10010)
    T                    h_alpha  = make_DataType<T>(2.0);
    T                    h_beta   = make_DataType<T>(1.0);
    hipsparseOperation_t transA   = HIPSPARSE_OPERATION_NON_TRANSPOSE;
    hipsparseIndexBase_t idx_base = HIPSPARSE_INDEX_BASE_ZERO;
    hipsparseSpMVAlg_t   alg      = HIPSPARSE_CSRMV_ALG2;

    // Matrices are stored at the same path in matrices directory
    std::string filename = hipsparse_exepath() + "../matrices/nos3.bin";

    // Index and data type
    hipsparseIndexType_t typeI
        = (typeid(I) == typeid(int32_t)) ? HIPSPARSE_INDEX_32I : HIPSPARSE_INDEX_64I;
    hipsparseIndexType_t typeJ
        = (typeid(J) == typeid(int32_t)) ? HIPSPARSE_INDEX_32I : HIPSPARSE_INDEX_64I;
    hipDataType typeT = (typeid(T) == typeid(float))
                            ? HIP_R_32F
                            : ((typeid(T) == typeid(double))
                                   ? HIP_R_64F
                                   : ((typeid(T) == typeid(hipComplex) ? HIP_C_32F : HIP_C_64F)));

    // hipSPARSE handle
    std::unique_ptr<handle_struct> test_handle(new handle_struct);
    hipsparseHandle_t              handle = test_handle->handle;

    // Host structures
    std::vector<I> hcsr_row_ptr;
    std::vector<J> hcol_ind;
    std::vector<T> hval;

    // Initial Data on CPU
    srand(12345ULL);

    J m;
    J n;
    I nnz;

    if(read_bin_matrix(filename.c_str(), m, n, nnz, hcsr_row_ptr, hcol_ind, hval, idx_base) != 0)
    {
        fprintf(stderr, "Cannot open [read] %s\n", filename.c_str());
        return HIPSPARSE_STATUS_INTERNAL_ERROR;
    }

    std::vector<T> hx(n);
    std::vector<T> hy_1(m);
    std::vector<T> hy_2(m);
    std::vector<T> hy_gold(m);

    hipsparseInit<T>(hx, 1, n);
    hipsparseInit<T>(hy_1, 1, m);

    // copy vector is easy in STL; hy_gold = hx: save a copy in hy_gold which will be output of CPU
    hy_2    = hy_1;
    hy_gold = hy_1;

    // allocate memory on device
    auto dptr_managed    = hipsparse_unique_ptr{device_malloc(sizeof(I) * (m + 1)), device_free};
    auto dcol_managed    = hipsparse_unique_ptr{device_malloc(sizeof(J) * nnz), device_free};
    auto dval_managed    = hipsparse_unique_ptr{device_malloc(sizeof(T) * nnz), device_free};
    auto dx_managed      = hipsparse_unique_ptr{device_malloc(sizeof(T) * n), device_free};
    auto dy_1_managed    = hipsparse_unique_ptr{device_malloc(sizeof(T) * m), device_free};
    auto dy_2_managed    = hipsparse_unique_ptr{device_malloc(sizeof(T) * m), device_free};
    auto d_alpha_managed = hipsparse_unique_ptr{device_malloc(sizeof(T)), device_free};
    auto d_beta_managed  = hipsparse_unique_ptr{device_malloc(sizeof(T)), device_free};

    I* dptr    = (I*)dptr_managed.get();
    J* dcol    = (J*)dcol_managed.get();
    T* dval    = (T*)dval_managed.get();
    T* dx      = (T*)dx_managed.get();
    T* dy_1    = (T*)dy_1_managed.get();
    T* dy_2    = (T*)dy_2_managed.get();
    T* d_alpha = (T*)d_alpha_managed.get();
    T* d_beta  = (T*)d_beta_managed.get();

    if(!dval || !dptr || !dcol || !dx || !dy_1 || !dy_2 || !d_alpha || !d_beta)
    {
        verify_hipsparse_status_success(HIPSPARSE_STATUS_ALLOC_FAILED,
                                        "!dval || !dptr || !dcol || !dx || "
                                        "!dy_1 || !dy_2 || !d_alpha || !d_beta");
        return HIPSPARSE_STATUS_ALLOC_FAILED;
    }

    // copy data from CPU to device
    CHECK_HIP_ERROR(
        hipMemcpy(dptr, hcsr_row_ptr.data(), sizeof(I) * (m + 1), hipMemcpyHostToDevice));
    CHECK_HIP_ERROR(hipMemcpy(dcol, hcol_ind.data(), sizeof(J) * nnz, hipMemcpyHostToDevice));
    CHECK_HIP_ERROR(hipMemcpy(dval, hval.data(), sizeof(T) * nnz, hipMemcpyHostToDevice));
    CHECK_HIP_ERROR(hipMemcpy(dx, hx.data(), sizeof(T) * n, hipMemcpyHostToDevice));
    CHECK_HIP_ERROR(hipMemcpy(dy_1, hy_1.data(), sizeof(T) * m, hipMemcpyHostToDevice));
    CHECK_HIP_ERROR(hipMemcpy(dy_2, hy_2.data(), sizeof(T) * m, hipMemcpyHostToDevice));
    CHECK_HIP_ERROR(hipMemcpy(d_alpha, &h_alpha, sizeof(T), hipMemcpyHostToDevice));
    CHECK_HIP_ERROR(hipMemcpy(d_beta, &h_beta, sizeof(T), hipMemcpyHostToDevice));

    // Create matrices
    hipsparseSpMatDescr_t A;
    CHECK_HIPSPARSE_ERROR(
        hipsparseCreateCsr(&A, m, n, nnz, dptr, dcol, dval, typeI, typeJ, idx_base, typeT));

    // Create dense vectors
    hipsparseDnVecDescr_t x, y1, y2;
    CHECK_HIPSPARSE_ERROR(hipsparseCreateDnVec(&x, n, dx, typeT));
    CHECK_HIPSPARSE_ERROR(hipsparseCreateDnVec(&y1, m, dy_1, typeT));
    CHECK_HIPSPARSE_ERROR(hipsparseCreateDnVec(&y2, m, dy_2, typeT));

    // Query SpMV buffer
    size_t bufferSize;
    CHECK_HIPSPARSE_ERROR(hipsparseSpMV_bufferSize(
        handle, transA, &h_alpha, A, x, &h_beta, y1, typeT, alg, &bufferSize));

    void* buffer;
    CHECK_HIP_ERROR(hipMalloc(&buffer, bufferSize));

    // Preprocess (optional)
    CHECK_HIPSPARSE_ERROR(
        hipsparseSpMV_preprocess(handle, transA, &h_alpha, A, x, &h_beta, y1, typeT, alg, buffer));

    // ROCSPARSE pointer mode host
    CHECK_HIPSPARSE_ERROR(hipsparseSetPointerMode(handle, HIPSPARSE_POINTER_MODE_HOST));
    CHECK_HIPSPARSE_ERROR(
        hipsparseSpMV(handle, transA, &h_alpha, A, x, &h_beta, y1, typeT, alg, buffer));

    // ROCSPARSE pointer mode device
    CHECK_HIPSPARSE_ERROR(hipsparseSetPointerMode(handle, HIPSPARSE_POINTER_MODE_DEVICE));
    CHECK_HIPSPARSE_ERROR(
        hipsparseSpMV(handle, transA, d_alpha, A, x, d_beta, y2, typeT, alg, buffer));

    // copy output from device to CPU
    CHECK_HIP_ERROR(hipMemcpy(hy_1.data(), dy_1, sizeof(T) * m, hipMemcpyDeviceToHost));
    CHECK_HIP_ERROR(hipMemcpy(hy_2.data(), dy_2, sizeof(T) * m, hipMemcpyDeviceToHost));

    // Query for warpSize
    hipDeviceProp_t prop;
    hipGetDeviceProperties(&prop, 0);

    int WF_SIZE;
    I   nnz_per_row = nnz / m;

    if(prop.warpSize == 32)
    {
        if(nnz_per_row < 4)
            WF_SIZE = 2;
        else if(nnz_per_row < 8)
            WF_SIZE = 4;
        else if(nnz_per_row < 16)
            WF_SIZE = 8;
        else if(nnz_per_row < 32)
            WF_SIZE = 16;
        else
            WF_SIZE = 32;
    }
    else if(prop.warpSize == 64)
    {
        if(nnz_per_row < 4)
            WF_SIZE = 2;
        else if(nnz_per_row < 8)
            WF_SIZE = 4;
        else if(nnz_per_row < 16)
            WF_SIZE = 8;
        else if(nnz_per_row < 32)
            WF_SIZE = 16;
        else if(nnz_per_row < 64)
            WF_SIZE = 32;
        else
            WF_SIZE = 64;
    }
    else
    {
        return HIPSPARSE_STATUS_INTERNAL_ERROR;
    }

    for(J i = 0; i < m; ++i)
    {
        std::vector<T> sum(WF_SIZE, make_DataType<T>(0.0));

        for(I j = hcsr_row_ptr[i] - idx_base; j < hcsr_row_ptr[i + 1] - idx_base; j += WF_SIZE)
        {
            for(int k = 0; k < WF_SIZE; ++k)
            {
                if(j + k < hcsr_row_ptr[i + 1] - idx_base)
                {
                    sum[k] = testing_fma(
                        h_alpha * hval[j + k], hx[hcol_ind[j + k] - idx_base], sum[k]);
                }
            }
        }

        for(int j = 1; j < WF_SIZE; j <<= 1)
        {
            for(int k = 0; k < WF_SIZE - j; ++k)
            {
                sum[k] = sum[k] + sum[k + j];
            }
        }

        if(h_beta == make_DataType<T>(0.0))
        {
            hy_gold[i] = sum[0];
        }
        else
        {
            hy_gold[i] = testing_fma(h_beta, hy_gold[i], sum[0]);
        }
    }

    unit_check_near(1, m, 1, hy_gold.data(), hy_1.data());
    unit_check_near(1, m, 1, hy_gold.data(), hy_2.data());

    CHECK_HIP_ERROR(hipFree(buffer));
    CHECK_HIPSPARSE_ERROR(hipsparseDestroySpMat(A));
    CHECK_HIPSPARSE_ERROR(hipsparseDestroyDnVec(x));
    CHECK_HIPSPARSE_ERROR(hipsparseDestroyDnVec(y1));
    CHECK_HIPSPARSE_ERROR(hipsparseDestroyDnVec(y2));
#endif

    return HIPSPARSE_STATUS_SUCCESS;
}

#endif // TESTING_SPMV_CSR_HPP
