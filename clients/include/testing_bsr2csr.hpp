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
#ifndef TESTING_BSR2CSR_HPP
#define TESTING_BSR2CSR_HPP

#include "hipsparse.hpp"
#include "hipsparse_test_unique_ptr.hpp"
#include "unit.hpp"
#include "utility.hpp"

#include <algorithm>
#include <hipsparse.h>
#include <string>

using namespace hipsparse;
using namespace hipsparse_test;

template <typename T>
void testing_bsr2csr_bad_arg(void)
{
#ifdef __HIP_PLATFORM_NVIDIA__
    // do not test for bad args
    return;
#endif
    int                  m            = 100;
    int                  n            = 100;
    int                  safe_size    = 100;
    int                  block_dim    = 2;
    hipsparseIndexBase_t csr_idx_base = HIPSPARSE_INDEX_BASE_ZERO;
    hipsparseIndexBase_t bsr_idx_base = HIPSPARSE_INDEX_BASE_ZERO;
    hipsparseDirection_t dir          = HIPSPARSE_DIRECTION_ROW;
    hipsparseStatus_t    status;

    std::unique_ptr<handle_struct> unique_ptr_handle(new handle_struct);
    hipsparseHandle_t              handle = unique_ptr_handle->handle;
    std::unique_ptr<descr_struct>  unique_ptr_csr_descr(new descr_struct);
    hipsparseMatDescr_t            csr_descr = unique_ptr_csr_descr->descr;
    std::unique_ptr<descr_struct>  unique_ptr_bsr_descr(new descr_struct);
    hipsparseMatDescr_t            bsr_descr = unique_ptr_bsr_descr->descr;

    hipsparseSetMatIndexBase(csr_descr, csr_idx_base);
    hipsparseSetMatIndexBase(bsr_descr, bsr_idx_base);

    auto bsr_row_ptr_managed
        = hipsparse_unique_ptr{device_malloc(sizeof(int) * safe_size), device_free};
    auto bsr_col_ind_managed
        = hipsparse_unique_ptr{device_malloc(sizeof(int) * safe_size), device_free};
    auto bsr_val_managed = hipsparse_unique_ptr{device_malloc(sizeof(T) * safe_size), device_free};
    auto csr_row_ptr_managed
        = hipsparse_unique_ptr{device_malloc(sizeof(int) * safe_size), device_free};
    auto csr_col_ind_managed
        = hipsparse_unique_ptr{device_malloc(sizeof(int) * safe_size), device_free};
    auto csr_val_managed = hipsparse_unique_ptr{device_malloc(sizeof(T) * safe_size), device_free};

    int* bsr_row_ptr = (int*)bsr_row_ptr_managed.get();
    int* bsr_col_ind = (int*)bsr_col_ind_managed.get();
    T*   bsr_val     = (T*)bsr_val_managed.get();
    int* csr_row_ptr = (int*)csr_row_ptr_managed.get();
    int* csr_col_ind = (int*)csr_col_ind_managed.get();
    T*   csr_val     = (T*)csr_val_managed.get();

    if(!bsr_row_ptr || !bsr_col_ind || !bsr_val || !csr_row_ptr || !csr_col_ind || !csr_val)
    {
        PRINT_IF_HIP_ERROR(hipErrorOutOfMemory);
        return;
    }

    // Testing hipsparseXbsr2csr()

    // Test invalid handle
    status = hipsparseXbsr2csr(nullptr,
                               dir,
                               m,
                               n,
                               bsr_descr,
                               bsr_val,
                               bsr_row_ptr,
                               bsr_col_ind,
                               block_dim,
                               csr_descr,
                               csr_val,
                               csr_row_ptr,
                               csr_col_ind);
    verify_hipsparse_status_invalid_handle(status);

    // Test invalid pointers
    status = hipsparseXbsr2csr(handle,
                               dir,
                               m,
                               n,
                               nullptr,
                               bsr_val,
                               bsr_row_ptr,
                               bsr_col_ind,
                               block_dim,
                               csr_descr,
                               csr_val,
                               csr_row_ptr,
                               csr_col_ind);
    verify_hipsparse_status_invalid_pointer(status, "Error: bsr_descr is nullptr");

    status = hipsparseXbsr2csr(handle,
                               dir,
                               m,
                               n,
                               bsr_descr,
                               (T*)nullptr,
                               bsr_row_ptr,
                               bsr_col_ind,
                               block_dim,
                               csr_descr,
                               csr_val,
                               csr_row_ptr,
                               csr_col_ind);
    verify_hipsparse_status_invalid_pointer(status, "Error: bsr_val is nullptr");

    status = hipsparseXbsr2csr(handle,
                               dir,
                               m,
                               n,
                               bsr_descr,
                               bsr_val,
                               nullptr,
                               bsr_col_ind,
                               block_dim,
                               csr_descr,
                               csr_val,
                               csr_row_ptr,
                               csr_col_ind);
    verify_hipsparse_status_invalid_pointer(status, "Error: bsr_row_ptr is nullptr");

    status = hipsparseXbsr2csr(handle,
                               dir,
                               m,
                               n,
                               bsr_descr,
                               bsr_val,
                               bsr_row_ptr,
                               nullptr,
                               block_dim,
                               csr_descr,
                               csr_val,
                               csr_row_ptr,
                               csr_col_ind);
    verify_hipsparse_status_invalid_pointer(status, "Error: bsr_col_ind is nullptr");

    status = hipsparseXbsr2csr(handle,
                               dir,
                               m,
                               n,
                               bsr_descr,
                               bsr_val,
                               bsr_row_ptr,
                               bsr_col_ind,
                               block_dim,
                               nullptr,
                               csr_val,
                               csr_row_ptr,
                               csr_col_ind);
    verify_hipsparse_status_invalid_pointer(status, "Error: csr_descr is nullptr");

    status = hipsparseXbsr2csr(handle,
                               dir,
                               m,
                               n,
                               bsr_descr,
                               bsr_val,
                               bsr_row_ptr,
                               bsr_col_ind,
                               block_dim,
                               csr_descr,
                               (T*)nullptr,
                               csr_row_ptr,
                               csr_col_ind);
    verify_hipsparse_status_invalid_pointer(status, "Error: csr_val is nullptr");

    status = hipsparseXbsr2csr(handle,
                               dir,
                               m,
                               n,
                               bsr_descr,
                               bsr_val,
                               bsr_row_ptr,
                               bsr_col_ind,
                               block_dim,
                               csr_descr,
                               csr_val,
                               nullptr,
                               csr_col_ind);
    verify_hipsparse_status_invalid_pointer(status, "Error: csr_row_ptr is nullptr");

    status = hipsparseXbsr2csr(handle,
                               dir,
                               m,
                               n,
                               bsr_descr,
                               bsr_val,
                               bsr_row_ptr,
                               bsr_col_ind,
                               block_dim,
                               csr_descr,
                               csr_val,
                               csr_row_ptr,
                               nullptr);
    verify_hipsparse_status_invalid_pointer(status, "Error: csr_col_ind is nullptr");

    // Test invalid sizes
    status = hipsparseXbsr2csr(handle,
                               dir,
                               -1,
                               n,
                               bsr_descr,
                               bsr_val,
                               bsr_row_ptr,
                               bsr_col_ind,
                               block_dim,
                               csr_descr,
                               csr_val,
                               csr_row_ptr,
                               csr_col_ind);
    verify_hipsparse_status_invalid_size(status, "Error: m is invalid");

    status = hipsparseXbsr2csr(handle,
                               dir,
                               m,
                               -1,
                               bsr_descr,
                               bsr_val,
                               bsr_row_ptr,
                               bsr_col_ind,
                               block_dim,
                               csr_descr,
                               csr_val,
                               csr_row_ptr,
                               csr_col_ind);
    verify_hipsparse_status_invalid_size(status, "Error: n is invalid");

    status = hipsparseXbsr2csr(handle,
                               dir,
                               m,
                               n,
                               bsr_descr,
                               bsr_val,
                               bsr_row_ptr,
                               bsr_col_ind,
                               -1,
                               csr_descr,
                               csr_val,
                               csr_row_ptr,
                               csr_col_ind);
    verify_hipsparse_status_invalid_size(status, "Error: block_dim is invalid");
}

template <typename T>
hipsparseStatus_t testing_bsr2csr(Arguments argus)
{
    int                  m            = argus.M;
    int                  n            = argus.N;
    int                  block_dim    = argus.block_dim;
    hipsparseIndexBase_t bsr_idx_base = argus.idx_base;
    hipsparseIndexBase_t csr_idx_base = argus.idx_base2;
    hipsparseDirection_t dir          = argus.dirA;
    std::string          binfile      = "";
    std::string          filename     = "";
    hipsparseStatus_t    status;

    // When in testing mode, M == N == -99 indicates that we are testing with a real
    // matrix from cise.ufl.edu
    int safe_size = 100;
    if(m == -99 && n == -99 && argus.timing == 0)
    {
        binfile = argus.filename;
        m = n = safe_size;
    }

    if(argus.timing == 1)
    {
        filename = argus.filename;
    }

    int mb = -1;
    int nb = -1;
    if(block_dim > 0)
    {
        mb = (m + block_dim - 1) / block_dim;
        nb = (n + block_dim - 1) / block_dim;
    }

    std::unique_ptr<handle_struct> unique_ptr_handle(new handle_struct);
    hipsparseHandle_t              handle = unique_ptr_handle->handle;
    std::unique_ptr<descr_struct>  unique_ptr_csr_descr(new descr_struct);
    hipsparseMatDescr_t            csr_descr = unique_ptr_csr_descr->descr;
    std::unique_ptr<descr_struct>  unique_ptr_bsr_descr(new descr_struct);
    hipsparseMatDescr_t            bsr_descr = unique_ptr_bsr_descr->descr;

    hipsparseSetMatIndexBase(csr_descr, csr_idx_base);
    hipsparseSetMatIndexBase(bsr_descr, bsr_idx_base);

    if(block_dim == 1)
    {
#ifdef __HIP_PLATFORM_NVIDIA__
        // cusparse does not support asynchronous execution if block_dim == 1
        return HIPSPARSE_STATUS_SUCCESS;
#endif
    }

    // Argument sanity check before allocating invalid memory
    if(mb <= 0 || nb <= 0 || block_dim <= 0)
    {
        auto dbsr_row_ptr_managed
            = hipsparse_unique_ptr{device_malloc(sizeof(int) * safe_size), device_free};
        auto dbsr_col_ind_managed
            = hipsparse_unique_ptr{device_malloc(sizeof(int) * safe_size), device_free};
        auto dbsr_val_managed
            = hipsparse_unique_ptr{device_malloc(sizeof(T) * safe_size), device_free};
        auto dcsr_row_ptr_managed
            = hipsparse_unique_ptr{device_malloc(sizeof(int) * safe_size), device_free};
        auto dcsr_col_ind_managed
            = hipsparse_unique_ptr{device_malloc(sizeof(int) * safe_size), device_free};
        auto dcsr_val_managed
            = hipsparse_unique_ptr{device_malloc(sizeof(T) * safe_size), device_free};

        int* dbsr_row_ptr = (int*)dbsr_row_ptr_managed.get();
        int* dbsr_col_ind = (int*)dbsr_col_ind_managed.get();
        T*   dbsr_val     = (T*)dbsr_val_managed.get();
        int* dcsr_row_ptr = (int*)dcsr_row_ptr_managed.get();
        int* dcsr_col_ind = (int*)dcsr_col_ind_managed.get();
        T*   dcsr_val     = (T*)dcsr_val_managed.get();

        if(!dbsr_row_ptr || !dbsr_col_ind || !dbsr_val || !dcsr_row_ptr || !dcsr_col_ind
           || !dcsr_val)
        {
            verify_hipsparse_status_success(HIPSPARSE_STATUS_ALLOC_FAILED,
                                            "!dbsr_row_ptr || !dbsr_col_ind || !dbsr_val || "
                                            "!dcsr_row_ptr || !dcsr_col_ind || !dcsr_val");
            return HIPSPARSE_STATUS_ALLOC_FAILED;
        }

        status = hipsparseXbsr2csr(handle,
                                   dir,
                                   mb,
                                   nb,
                                   bsr_descr,
                                   dbsr_val,
                                   dbsr_row_ptr,
                                   dbsr_col_ind,
                                   block_dim,
                                   csr_descr,
                                   dcsr_val,
                                   dcsr_row_ptr,
                                   dcsr_col_ind);

        if(mb < 0 || nb < 0 || block_dim < 0)
        {
            verify_hipsparse_status_invalid_size(status,
                                                 "Error: mb < 0 || nb < 0 || block_dim < 0");
        }
        else
        {
            verify_hipsparse_status_success(status, "mb >= 0 && nb >= 0 && block_dim >= 0");
        }

        return HIPSPARSE_STATUS_SUCCESS;
    }

    // Read or construct CSR matrix
    std::vector<int> csr_row_ptr;
    std::vector<int> csr_col_ind;
    std::vector<T>   csr_val;
    int              nnz;
    srand(12345ULL);
    if(binfile != "")
    {
        if(read_bin_matrix(
               binfile.c_str(), m, n, nnz, csr_row_ptr, csr_col_ind, csr_val, csr_idx_base)
           != 0)
        {
            fprintf(stderr, "Cannot open [read] %s\n", binfile.c_str());
            return HIPSPARSE_STATUS_INTERNAL_ERROR;
        }
    }
    else if(argus.laplacian)
    {
        m = n = gen_2d_laplacian(argus.laplacian, csr_row_ptr, csr_col_ind, csr_val, csr_idx_base);
        nnz   = csr_row_ptr[m];
    }
    else
    {
        std::vector<int> coo_row_ind;

        if(filename != "")
        {
            if(read_mtx_matrix(
                   filename.c_str(), m, n, nnz, coo_row_ind, csr_col_ind, csr_val, csr_idx_base)
               != 0)
            {
                fprintf(stderr, "Cannot open [read] %s\n", filename.c_str());
                return HIPSPARSE_STATUS_INTERNAL_ERROR;
            }
        }
        else
        {
            double scale = 0.02;
            if(m > 1000 || n > 1000)
            {
                scale = 2.0 / std::max(m, n);
            }
            nnz = m * scale * n;
            gen_matrix_coo(m, n, nnz, coo_row_ind, csr_col_ind, csr_val, csr_idx_base);
        }

        // Convert COO to CSR
        csr_row_ptr.resize(m + 1, 0);
        for(int i = 0; i < nnz; ++i)
        {
            ++csr_row_ptr[coo_row_ind[i] + 1 - csr_idx_base];
        }

        csr_row_ptr[0] = csr_idx_base;
        for(int i = 0; i < m; ++i)
        {
            csr_row_ptr[i + 1] += csr_row_ptr[i];
        }
    }

    // m and n can be modifed if we read in a matrix from a file
    mb = (m + block_dim - 1) / block_dim;
    nb = (n + block_dim - 1) / block_dim;

    // Host BSR matrix
    std::vector<int> hbsr_row_ptr;
    std::vector<int> hbsr_col_ind;
    std::vector<T>   hbsr_val;

    // Convert CSR matrix to BSR
    int nnzb;
    host_csr_to_bsr<T>(dir,
                       m,
                       n,
                       block_dim,
                       nnzb,
                       csr_idx_base,
                       csr_row_ptr,
                       csr_col_ind,
                       csr_val,
                       bsr_idx_base,
                       hbsr_row_ptr,
                       hbsr_col_ind,
                       hbsr_val);

    // Determine the size of the output CSR matrix based on the size of the input BSR matrix
    m = mb * block_dim;
    n = nb * block_dim;

    // Host CSR matrix
    std::vector<int> hcsr_row_ptr(m + 1);
    std::vector<int> hcsr_col_ind(nnzb * block_dim * block_dim);
    std::vector<T>   hcsr_val(nnzb * block_dim * block_dim);

    // Allocate memory on the device
    auto dbsr_row_ptr_managed
        = hipsparse_unique_ptr{device_malloc(sizeof(int) * (mb + 1)), device_free};
    auto dbsr_col_ind_managed
        = hipsparse_unique_ptr{device_malloc(sizeof(int) * nnzb), device_free};
    auto dbsr_val_managed = hipsparse_unique_ptr{
        device_malloc(sizeof(T) * nnzb * block_dim * block_dim), device_free};
    auto dcsr_row_ptr_managed
        = hipsparse_unique_ptr{device_malloc(sizeof(int) * (m + 1)), device_free};
    auto dcsr_col_ind_managed = hipsparse_unique_ptr{
        device_malloc(sizeof(int) * nnzb * block_dim * block_dim), device_free};
    auto dcsr_val_managed = hipsparse_unique_ptr{
        device_malloc(sizeof(T) * nnzb * block_dim * block_dim), device_free};

    int* dbsr_row_ptr = (int*)dbsr_row_ptr_managed.get();
    int* dbsr_col_ind = (int*)dbsr_col_ind_managed.get();
    T*   dbsr_val     = (T*)dbsr_val_managed.get();
    int* dcsr_row_ptr = (int*)dcsr_row_ptr_managed.get();
    int* dcsr_col_ind = (int*)dcsr_col_ind_managed.get();
    T*   dcsr_val     = (T*)dcsr_val_managed.get();

    if(!dbsr_row_ptr || !dbsr_col_ind || !dbsr_val || !dcsr_row_ptr || !dcsr_col_ind || !dcsr_val)
    {
        verify_hipsparse_status_success(HIPSPARSE_STATUS_ALLOC_FAILED,
                                        "!dbsr_row_ptr || !dbsr_col_ind || !dbsr_val || "
                                        "!dcsr_row_ptr || !dcsr_col_ind || !dcsr_val");
        return HIPSPARSE_STATUS_ALLOC_FAILED;
    }

    // Copy data from host to device
    CHECK_HIP_ERROR(hipMemcpy(
        dbsr_row_ptr, hbsr_row_ptr.data(), sizeof(int) * (mb + 1), hipMemcpyHostToDevice));
    CHECK_HIP_ERROR(
        hipMemcpy(dbsr_col_ind, hbsr_col_ind.data(), sizeof(int) * nnzb, hipMemcpyHostToDevice));
    CHECK_HIP_ERROR(hipMemcpy(dbsr_val,
                              hbsr_val.data(),
                              sizeof(T) * nnzb * block_dim * block_dim,
                              hipMemcpyHostToDevice));

    if(argus.unit_check)
    {
        CHECK_HIPSPARSE_ERROR(hipsparseXbsr2csr(handle,
                                                dir,
                                                mb,
                                                nb,
                                                bsr_descr,
                                                dbsr_val,
                                                dbsr_row_ptr,
                                                dbsr_col_ind,
                                                block_dim,
                                                csr_descr,
                                                dcsr_val,
                                                dcsr_row_ptr,
                                                dcsr_col_ind));

        // Copy output from device to host
        CHECK_HIP_ERROR(hipMemcpy(
            hcsr_row_ptr.data(), dcsr_row_ptr, sizeof(int) * (m + 1), hipMemcpyDeviceToHost));
        CHECK_HIP_ERROR(hipMemcpy(hcsr_col_ind.data(),
                                  dcsr_col_ind,
                                  sizeof(int) * nnzb * block_dim * block_dim,
                                  hipMemcpyDeviceToHost));
        CHECK_HIP_ERROR(hipMemcpy(hcsr_val.data(),
                                  dcsr_val,
                                  sizeof(T) * nnzb * block_dim * block_dim,
                                  hipMemcpyDeviceToHost));

        // Host computed bsr2csr conversion
        std::vector<int> hcsr_row_ptr_gold(m + 1);
        std::vector<int> hcsr_col_ind_gold(nnzb * block_dim * block_dim, 0);
        std::vector<T>   hcsr_val_gold(nnzb * block_dim * block_dim);

        // Host bsr2csr
        host_bsr_to_csr<T>(dir,
                           mb,
                           nb,
                           block_dim,
                           bsr_idx_base,
                           hbsr_row_ptr,
                           hbsr_col_ind,
                           hbsr_val,
                           csr_idx_base,
                           hcsr_row_ptr_gold,
                           hcsr_col_ind_gold,
                           hcsr_val_gold);

        // Unit check
        unit_check_general(1, m + 1, 1, hcsr_row_ptr_gold.data(), hcsr_row_ptr.data());
        unit_check_general(
            1, nnzb * block_dim * block_dim, 1, hcsr_col_ind_gold.data(), hcsr_col_ind.data());
        unit_check_general(
            1, nnzb * block_dim * block_dim, 1, hcsr_val_gold.data(), hcsr_val.data());
    }

    return HIPSPARSE_STATUS_SUCCESS;
}

#endif // TESTING_BSR2CSR_HPP
