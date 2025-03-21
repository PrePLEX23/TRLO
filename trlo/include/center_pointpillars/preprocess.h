/****************************************************************************************
 *
 * Copyright (c) 2024, Shenyang Institute of Automation, Chinese Academy of Sciences
 *
 * Authors: Yanpeng Jia
 * Contact: jiayanpeng@sia.cn
 *
 ****************************************************************************************/
/*
 * SPDX-FileCopyrightText: Copyright (c) 2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
 
#include "kernel.h"

class PreProcessCuda {
  private:
    Params params_;
    unsigned int *point2voxel_offset_;
    unsigned int *hash_table_;
    float *voxels_temp_;

    unsigned int *d_real_num_voxels_;
    unsigned int *h_real_num_voxels_;
    half *d_voxel_features_;
    unsigned int *d_voxel_num_;
    unsigned int *d_voxel_indices_;

    unsigned int hash_table_size_;
    unsigned int voxels_temp_size_;
    unsigned int voxel_features_size_;
    unsigned int voxel_idxs_size_;
    unsigned int voxel_num_size_;

  public:
    PreProcessCuda();
    ~PreProcessCuda();

    int alloc_resource();
    int generateVoxels(const float *points, size_t points_size, cudaStream_t stream);
    unsigned int getOutput(half** d_voxel_features, unsigned int** d_voxel_indices, std::vector<int>& sparse_shape);
};