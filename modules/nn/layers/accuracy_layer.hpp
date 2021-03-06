#include <algorithm>
#include <math/math_op.h>
#include "util/filler.h"

namespace alchemy {

template <typename Device, typename T>
void AccuracyLayer<Device, T>::ForwardCPU(const vector<container *> &input,
                                  const vector<container *> &output)
{
    auto size = input[0]->size(2, 4);
    auto o_ptr = input[0]->data_cptr();
    auto g_ptr = input[1]->data_cptr();
    int result_ = 0;
    for(size_t i = 0; i < input[0]->shape(0); ++i) {
        // simples
        auto o_iter = std::max_element(o_ptr + i * size, o_ptr + i * size + size);
        auto g_iter = std::max_element(g_ptr + i * size, g_ptr + i * size + size);
        if(std::distance(o_ptr + i * size, o_iter) == std::distance(g_ptr + i * size, g_iter)) {
            result_++;
        }
    }

    output[0]->mutable_data_cptr()[0] = (T)result_ / input[0]->shape(0);
}
}
