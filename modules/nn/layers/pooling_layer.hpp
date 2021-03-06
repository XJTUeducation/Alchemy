#include "math/math_op.h"

namespace alchemy {

template <typename Device, typename T>
void PoolingLayer<Device, T>::setup(const vector<container *> &input,
                            const vector<container *> &output)
{
    assert((size_t)input[0]->shape(2) >= pooling_param_.kernel_size());
    assert((size_t)input[0]->shape(3) >= pooling_param_.kernel_size());

    auto ksize = pooling_param_.kernel_size();
    auto num_in = input[0]->shape(0);
    auto chs_in = input[0]->shape(1);
    auto row_in = input[0]->shape(2);
    auto col_in = input[0]->shape(3);

    auto row_out = (row_in - ksize) / pooling_param_.stride() + 1;
    auto col_out = (col_in - ksize) / pooling_param_.stride() + 1;

    output[0]->reset({ num_in, chs_in, row_out, col_out });
    max_idx_.reset(output[0]->shape());
}

template <typename Device, typename T>
void PoolingLayer<Device, T>::ForwardCPU(const vector<container *> &input,
                                 const vector<container *> &output)
{
    const size_t batch_size = input[0]->shape(0);
    const size_t channels = input[0]->shape(1);
    const size_t in_cols = input[0]->shape(3);
    const size_t out_rows = output[0]->shape(2);
    const size_t out_cols = output[0]->shape(3);
    const size_t stride = pooling_param_.stride();
    const size_t ksize = pooling_param_.kernel_size();

    auto input_data = input[0]->data_cptr();
    auto output_data = output[0]->mutable_data_cptr();
    auto max_idx = max_idx_.mutable_cptr();

    //
//    vector_set(output[0]->size(), -std::numeric_limits<T>::max(), output_data);
    Filler<Device, T>::constant_fill(output[0]->size(), output_data, -std::numeric_limits<T>::max());

    switch(pooling_param_.type()) {
        case MAX:
            for(size_t batch_idx = 0; batch_idx < batch_size; ++batch_idx) {
                for(size_t chs_idx = 0; chs_idx < channels; ++chs_idx) {

                    for(size_t row_idx = 0; row_idx < out_rows; ++row_idx) {
                        for(size_t col_idx = 0; col_idx < out_cols; ++col_idx) {

                            size_t r_s = row_idx * stride;
                            size_t c_s = col_idx * stride;
                            size_t r_e = r_s + ksize;
                            size_t c_e = c_s + ksize;

                            size_t out_idx = row_idx * out_cols + col_idx;
                            for(size_t i = r_s; i < r_e; ++i) {
                                for(size_t j = c_s; j < c_e; ++j) {

                                    size_t in_idx = i * in_cols + j;

                                    if(input_data[in_idx] > output_data[out_idx]) {
                                        output_data[out_idx] = input_data[in_idx];
                                        max_idx[out_idx] = in_idx;
                                    }
                                }
                            }
                        }
                    }

                    input_data += input[0]->size(2, 4);
                    output_data += output[0]->size(2, 4);
                    max_idx += max_idx_.size(2, 4);
                }
            }

            break;

        case AVERAGE: LOG(FATAL) << "Not implement!"; break;

        default: LOG(FATAL) << "Unknown Pooling type!"; break;
    }
}

template <typename Device, typename T>
void PoolingLayer<Device, T>::BackwardCPU(const vector<container *> &input,
                                  const vector<container *> &output)
{
    const size_t batch_size = input[0]->shape(0);
    const size_t channels = input[0]->shape(1);
    const size_t out_rows = output[0]->shape(2);
    const size_t out_cols = output[0]->shape(3);

    auto input_diff = input[0]->mutable_diff_cptr();
    auto output_diff = output[0]->diff_cptr();
    auto max_idx = max_idx_.cptr();

    Filler<Device, T>::constant_fill(input[0]->size(), input[0]->mutable_diff_cptr(), (T)0.0);
//    vector_set(input[0]->size(), (T)0.0, input[0]->mutable_diff_cptr());

    switch(pooling_param_.type()) {
        case MAX:
            for(size_t batch_idx = 0; batch_idx < batch_size; ++batch_idx) {
                for(size_t chs_idx = 0; chs_idx < channels; ++chs_idx) {

                    for(size_t row_idx = 0; row_idx < out_rows; ++row_idx) {
                        for(size_t col_idx = 0; col_idx < out_cols; ++col_idx) {

                            size_t out_idx = row_idx * out_cols + col_idx;

                            input_diff[max_idx[out_idx]] += output_diff[out_idx];
                        }
                    }

                    input_diff += input[0]->size(2, 4);
                    output_diff += output[0]->size(2, 4);
                    max_idx += max_idx_.size(2, 4);
                }
            }
            break;

        case AVERAGE: LOG(FATAL) << "Not implement!"; break;
        default: LOG(FATAL) << "Unknown Pooling type!"; break;
    }
}
}