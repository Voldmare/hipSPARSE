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
#ifndef TESTING_HYBMV_HPP
#define TESTING_HYBMV_HPP

#include "hipsparse.hpp"
#include "hipsparse_test_unique_ptr.hpp"
#include "unit.hpp"
#include "utility.hpp"

#include <hipsparse.h>
#include <string>

using namespace hipsparse;
using namespace hipsparse_test;

#define ELL_IND_ROW(i, el, m, width) (el) * (m) + (i)
#define ELL_IND_EL(i, el, m, width) (el) + (width) * (i)
#define ELL_IND(i, el, m, width) ELL_IND_ROW(i, el, m, width)

struct testhyb
{
    int                     m;
    int                     n;
    hipsparseHybPartition_t partition;
    int                     ell_nnz;
    int                     ell_width;
    int*                    ell_col_ind;
    void*                   ell_val;
    int                     coo_nnz;
    int*                    coo_row_ind;
    int*                    coo_col_ind;
    void*                   coo_val;
};

template <typename T>
void testing_hybmv_bad_arg(void)
{
    int                  safe_size = 100;
    T                    alpha     = 0.6;
    T                    beta      = 0.2;
    hipsparseOperation_t transA    = HIPSPARSE_OPERATION_NON_TRANSPOSE;
    hipsparseStatus_t    status;

    std::unique_ptr<handle_struct> unique_ptr_handle(new handle_struct);
    hipsparseHandle_t              handle = unique_ptr_handle->handle;

    std::unique_ptr<descr_struct> unique_ptr_descr(new descr_struct);
    hipsparseMatDescr_t           descr = unique_ptr_descr->descr;

    std::unique_ptr<hyb_struct> unique_ptr_hyb(new hyb_struct);
    hipsparseHybMat_t           hyb = unique_ptr_hyb->hyb;

    auto dx_managed = hipsparse_unique_ptr{device_malloc(sizeof(T) * safe_size), device_free};
    auto dy_managed = hipsparse_unique_ptr{device_malloc(sizeof(T) * safe_size), device_free};

    T* dx = (T*)dx_managed.get();
    T* dy = (T*)dy_managed.get();

    if(!dx || !dy)
    {
        PRINT_IF_HIP_ERROR(hipErrorOutOfMemory);
        return;
    }

    // testing for(nullptr == dx)
    {
        T* dx_null = nullptr;

        status = hipsparseXhybmv(handle, transA, &alpha, descr, hyb, dx_null, &beta, dy);
        verify_hipsparse_status_invalid_pointer(status, "Error: dx is nullptr");
    }
    // testing for(nullptr == dy)
    {
        T* dy_null = nullptr;

        status = hipsparseXhybmv(handle, transA, &alpha, descr, hyb, dx, &beta, dy_null);
        verify_hipsparse_status_invalid_pointer(status, "Error: dy is nullptr");
    }
    // testing for(nullptr == d_alpha)
    {
        T* d_alpha_null = nullptr;

        status = hipsparseXhybmv(handle, transA, d_alpha_null, descr, hyb, dx, &beta, dy);
        verify_hipsparse_status_invalid_pointer(status, "Error: alpha is nullptr");
    }
    // testing for(nullptr == d_beta)
    {
        T* d_beta_null = nullptr;

        status = hipsparseXhybmv(handle, transA, &alpha, descr, hyb, dx, d_beta_null, dy);
        verify_hipsparse_status_invalid_pointer(status, "Error: beta is nullptr");
    }
    // testing for(nullptr == hyb)
    {
        hipsparseHybMat_t hyb_null = nullptr;

        status = hipsparseXhybmv(handle, transA, &alpha, descr, hyb_null, dx, &beta, dy);
        verify_hipsparse_status_invalid_pointer(status, "Error: descr is nullptr");
    }
    // testing for(nullptr == descr)
    {
        hipsparseMatDescr_t descr_null = nullptr;

        status = hipsparseXhybmv(handle, transA, &alpha, descr_null, hyb, dx, &beta, dy);
        verify_hipsparse_status_invalid_pointer(status, "Error: descr is nullptr");
    }
    // testing for(nullptr == handle)
    {
        hipsparseHandle_t handle_null = nullptr;

        status = hipsparseXhybmv(handle_null, transA, &alpha, descr, hyb, dx, &beta, dy);
        verify_hipsparse_status_invalid_handle(status);
    }
}

template <typename T>
hipsparseStatus_t testing_hybmv(Arguments argus)
{
    int                     safe_size      = 100;
    int                     m              = argus.M;
    int                     n              = argus.N;
    T                       h_alpha        = make_DataType<T>(argus.alpha);
    T                       h_beta         = make_DataType<T>(argus.beta);
    hipsparseOperation_t    transA         = argus.transA;
    hipsparseIndexBase_t    idx_base       = argus.idx_base;
    hipsparseHybPartition_t part           = argus.part;
    int                     user_ell_width = argus.ell_width;
    std::string             binfile        = "";
    std::string             filename       = "";
    hipsparseStatus_t       status;

    T zero = make_DataType<T>(0.0);
    T one  = make_DataType<T>(1.0);

    // When in testing mode, M == N == -99 indicates that we are testing with a real
    // matrix from cise.ufl.edu
    if(m == -99 && n == -99 && argus.timing == 0)
    {
        binfile = argus.filename;
        m = n = safe_size;
    }

    if(argus.timing == 1)
    {
        filename = argus.filename;
    }

    std::unique_ptr<handle_struct> test_handle(new handle_struct);
    hipsparseHandle_t              handle = test_handle->handle;

    std::unique_ptr<descr_struct> test_descr(new descr_struct);
    hipsparseMatDescr_t           descr = test_descr->descr;

    // Set matrix index base
    CHECK_HIPSPARSE_ERROR(hipsparseSetMatIndexBase(descr, idx_base));

    std::unique_ptr<hyb_struct> test_hyb(new hyb_struct);
    hipsparseHybMat_t           hyb = test_hyb->hyb;

    // Determine number of non-zero elements
    double scale = 0.02;
    if(m > 1000 || n > 1000)
    {
        scale = 2.0 / std::max(m, n);
    }
    int nnz = m * scale * n;

    // Argument sanity check before allocating invalid memory
    if(m <= 0 || n <= 0 || nnz <= 0)
    {
        auto dptr_managed
            = hipsparse_unique_ptr{device_malloc(sizeof(int) * safe_size), device_free};
        auto dcol_managed
            = hipsparse_unique_ptr{device_malloc(sizeof(int) * safe_size), device_free};
        auto dval_managed = hipsparse_unique_ptr{device_malloc(sizeof(T) * safe_size), device_free};
        auto dx_managed   = hipsparse_unique_ptr{device_malloc(sizeof(T) * safe_size), device_free};
        auto dy_managed   = hipsparse_unique_ptr{device_malloc(sizeof(T) * safe_size), device_free};

        int* dptr = (int*)dptr_managed.get();
        int* dcol = (int*)dcol_managed.get();
        T*   dval = (T*)dval_managed.get();
        T*   dx   = (T*)dx_managed.get();
        T*   dy   = (T*)dy_managed.get();

        if(!dval || !dptr || !dcol || !dx || !dy)
        {
            verify_hipsparse_status_success(HIPSPARSE_STATUS_ALLOC_FAILED,
                                            "!dptr || !dcol || !dval || !dx || !dy");
            return HIPSPARSE_STATUS_ALLOC_FAILED;
        }

        CHECK_HIPSPARSE_ERROR(hipsparseSetPointerMode(handle, HIPSPARSE_POINTER_MODE_HOST));
        status
            = hipsparseXcsr2hyb(handle, m, n, descr, dval, dptr, dcol, hyb, user_ell_width, part);

        if(m < 0 || n < 0 || nnz < 0)
        {
            verify_hipsparse_status_invalid_size(status, "Error: m < 0 || n < 0 || nnz < 0");
        }

        // hybmv should be able to deal with m <= 0 || n <= 0 || nnz <= 0 even if csr2hyb fails
        // because hyb structures is allocated with n = m = 0 - so nothing should happen
        status = hipsparseXhybmv(handle, transA, &h_alpha, descr, hyb, dx, &h_beta, dy);
        verify_hipsparse_status_success(status, "m >= 0 && n >= 0 && nnz >= 0");

        return HIPSPARSE_STATUS_SUCCESS;
    }

    // Host structures
    std::vector<int> hcsr_row_ptr;
    std::vector<int> hcoo_row_ind;
    std::vector<int> hcol_ind;
    std::vector<T>   hval;

    // Initial Data on CPU
    srand(12345ULL);
    if(binfile != "")
    {
        if(read_bin_matrix(binfile.c_str(), m, n, nnz, hcsr_row_ptr, hcol_ind, hval, idx_base) != 0)
        {
            fprintf(stderr, "Cannot open [read] %s\n", binfile.c_str());
            return HIPSPARSE_STATUS_INTERNAL_ERROR;
        }
    }
    else if(argus.laplacian)
    {
        m = n = gen_2d_laplacian(argus.laplacian, hcsr_row_ptr, hcol_ind, hval, idx_base);
        nnz   = hcsr_row_ptr[m];
    }
    else
    {
        if(filename != "")
        {
            if(read_mtx_matrix(filename.c_str(), m, n, nnz, hcoo_row_ind, hcol_ind, hval, idx_base)
               != 0)
            {
                fprintf(stderr, "Cannot open [read] %s\n", filename.c_str());
                return HIPSPARSE_STATUS_INTERNAL_ERROR;
            }
        }
        else
        {
            gen_matrix_coo(m, n, nnz, hcoo_row_ind, hcol_ind, hval, idx_base);
        }

        // Convert COO to CSR
        hcsr_row_ptr.resize(m + 1, 0);
        for(int i = 0; i < nnz; ++i)
        {
            ++hcsr_row_ptr[hcoo_row_ind[i] + 1 - idx_base];
        }

        hcsr_row_ptr[0] = idx_base;
        for(int i = 0; i < m; ++i)
        {
            hcsr_row_ptr[i + 1] += hcsr_row_ptr[i];
        }
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
    auto dptr_managed    = hipsparse_unique_ptr{device_malloc(sizeof(int) * (m + 1)), device_free};
    auto dcol_managed    = hipsparse_unique_ptr{device_malloc(sizeof(int) * nnz), device_free};
    auto dval_managed    = hipsparse_unique_ptr{device_malloc(sizeof(T) * nnz), device_free};
    auto dx_managed      = hipsparse_unique_ptr{device_malloc(sizeof(T) * n), device_free};
    auto dy_1_managed    = hipsparse_unique_ptr{device_malloc(sizeof(T) * m), device_free};
    auto dy_2_managed    = hipsparse_unique_ptr{device_malloc(sizeof(T) * m), device_free};
    auto d_alpha_managed = hipsparse_unique_ptr{device_malloc(sizeof(T)), device_free};
    auto d_beta_managed  = hipsparse_unique_ptr{device_malloc(sizeof(T)), device_free};

    int* dptr    = (int*)dptr_managed.get();
    int* dcol    = (int*)dcol_managed.get();
    T*   dval    = (T*)dval_managed.get();
    T*   dx      = (T*)dx_managed.get();
    T*   dy_1    = (T*)dy_1_managed.get();
    T*   dy_2    = (T*)dy_2_managed.get();
    T*   d_alpha = (T*)d_alpha_managed.get();
    T*   d_beta  = (T*)d_beta_managed.get();

    if(!dval || !dptr || !dcol || !dx || !dy_1 || !dy_2 || !d_alpha || !d_beta)
    {
        verify_hipsparse_status_success(HIPSPARSE_STATUS_ALLOC_FAILED,
                                        "!dval || !dptr || !dcol || !dx || "
                                        "!dy_1 || !dy_2 || !d_alpha || !d_beta");
        return HIPSPARSE_STATUS_ALLOC_FAILED;
    }

    // copy data from CPU to device
    CHECK_HIP_ERROR(
        hipMemcpy(dptr, hcsr_row_ptr.data(), sizeof(int) * (m + 1), hipMemcpyHostToDevice));
    CHECK_HIP_ERROR(hipMemcpy(dcol, hcol_ind.data(), sizeof(int) * nnz, hipMemcpyHostToDevice));
    CHECK_HIP_ERROR(hipMemcpy(dval, hval.data(), sizeof(T) * nnz, hipMemcpyHostToDevice));
    CHECK_HIP_ERROR(hipMemcpy(dx, hx.data(), sizeof(T) * n, hipMemcpyHostToDevice));
    CHECK_HIP_ERROR(hipMemcpy(dy_1, hy_1.data(), sizeof(T) * m, hipMemcpyHostToDevice));
    CHECK_HIP_ERROR(hipMemcpy(d_alpha, &h_alpha, sizeof(T), hipMemcpyHostToDevice));
    CHECK_HIP_ERROR(hipMemcpy(d_beta, &h_beta, sizeof(T), hipMemcpyHostToDevice));

    // ELL width limit
    int width_limit = (2 * nnz - 1) / m + 1;

    // Limit ELL user width
    if(part == HIPSPARSE_HYB_PARTITION_USER)
    {
        user_ell_width = user_ell_width * nnz / m;
        user_ell_width = std::min(width_limit, user_ell_width);
    }

    // Convert CSR to HYB
    status = hipsparseXcsr2hyb(handle, m, n, descr, dval, dptr, dcol, hyb, user_ell_width, part);

    if(part == HIPSPARSE_HYB_PARTITION_MAX)
    {
        // Compute max ELL width
        int ell_max_width = 0;
        for(int i = 0; i < m; ++i)
        {
            ell_max_width = std::max(hcsr_row_ptr[i + 1] - hcsr_row_ptr[i], ell_max_width);
        }

        if(ell_max_width > width_limit)
        {
            verify_hipsparse_status_invalid_value(status, "ell_max_width > width_limit");
            return HIPSPARSE_STATUS_SUCCESS;
        }
    }

    if(argus.unit_check)
    {
        // Copy HYB structure to CPU
        testhyb* dhyb = (testhyb*)hyb;

        int ell_nnz = dhyb->ell_nnz;
        int coo_nnz = dhyb->coo_nnz;

        std::vector<int> hell_col(ell_nnz);
        std::vector<T>   hell_val(ell_nnz);
        std::vector<int> hcoo_row(coo_nnz);
        std::vector<int> hcoo_col(coo_nnz);
        std::vector<T>   hcoo_val(coo_nnz);

        if(ell_nnz > 0)
        {
            CHECK_HIP_ERROR(hipMemcpy(
                hell_col.data(), dhyb->ell_col_ind, sizeof(int) * ell_nnz, hipMemcpyDeviceToHost));
            CHECK_HIP_ERROR(hipMemcpy(
                hell_val.data(), dhyb->ell_val, sizeof(T) * ell_nnz, hipMemcpyDeviceToHost));
        }

        if(coo_nnz > 0)
        {
            CHECK_HIP_ERROR(hipMemcpy(
                hcoo_row.data(), dhyb->coo_row_ind, sizeof(int) * coo_nnz, hipMemcpyDeviceToHost));
            CHECK_HIP_ERROR(hipMemcpy(
                hcoo_col.data(), dhyb->coo_col_ind, sizeof(int) * coo_nnz, hipMemcpyDeviceToHost));
            CHECK_HIP_ERROR(hipMemcpy(
                hcoo_val.data(), dhyb->coo_val, sizeof(T) * coo_nnz, hipMemcpyDeviceToHost));
        }

        CHECK_HIP_ERROR(hipMemcpy(dy_2, hy_2.data(), sizeof(T) * m, hipMemcpyHostToDevice));

        // ROCSPARSE pointer mode host
        CHECK_HIPSPARSE_ERROR(hipsparseSetPointerMode(handle, HIPSPARSE_POINTER_MODE_HOST));
        CHECK_HIPSPARSE_ERROR(
            hipsparseXhybmv(handle, transA, &h_alpha, descr, hyb, dx, &h_beta, dy_1));

        // ROCSPARSE pointer mode device
        CHECK_HIPSPARSE_ERROR(hipsparseSetPointerMode(handle, HIPSPARSE_POINTER_MODE_DEVICE));
        CHECK_HIPSPARSE_ERROR(
            hipsparseXhybmv(handle, transA, d_alpha, descr, hyb, dx, d_beta, dy_2));

        // copy output from device to CPU
        CHECK_HIP_ERROR(hipMemcpy(hy_1.data(), dy_1, sizeof(T) * m, hipMemcpyDeviceToHost));
        CHECK_HIP_ERROR(hipMemcpy(hy_2.data(), dy_2, sizeof(T) * m, hipMemcpyDeviceToHost));

        // CPU
        // ELL part
        if(ell_nnz > 0)
        {
            for(int i = 0; i < m; ++i)
            {
                T sum = zero;
                for(int p = 0; p < dhyb->ell_width; ++p)
                {
                    int idx = ELL_IND(i, p, m, dhyb->ell_width);
                    int col = hell_col[idx] - idx_base;

                    if(col >= 0 && col < n)
                    {
                        sum = sum + hell_val[idx] * hx[col];
                    }
                    else
                    {
                        break;
                    }
                }

                if(h_beta != zero)
                {
                    hy_gold[i] = h_beta * hy_gold[i] + h_alpha * sum;
                }
                else
                {
                    hy_gold[i] = h_alpha * sum;
                }
            }
        }

        // COO part
        if(coo_nnz > 0)
        {
            T coo_beta = (ell_nnz > 0) ? one : h_beta;

            for(int i = 0; i < m; ++i)
            {
                hy_gold[i] = hy_gold[i] * coo_beta;
            }

            for(int i = 0; i < coo_nnz; ++i)
            {
                int row = hcoo_row[i] - idx_base;
                int col = hcoo_col[i] - idx_base;

                hy_gold[row] = hy_gold[row] + h_alpha * hcoo_val[i] * hx[col];
            }
        }

        unit_check_near(1, m, 1, hy_gold.data(), hy_1.data());
        unit_check_near(1, m, 1, hy_gold.data(), hy_2.data());
    }

    return HIPSPARSE_STATUS_SUCCESS;
}

#endif // TESTING_HYBMV_HPP
