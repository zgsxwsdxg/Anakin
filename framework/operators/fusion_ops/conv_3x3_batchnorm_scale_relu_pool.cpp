#include "framework/operators/fusion_ops/conv_3x3_batchnorm_scale_relu_pool.h"

namespace anakin {

namespace ops {

#ifdef USE_CUDA
template<>
void SassConvBatchnormScaleReluPool<NV, Precision::FP32>::operator() (
	OpContext<NV> &ctx, 
	const std::vector<Tensor4dPtr<NV> >& ins, 
	std::vector<Tensor4dPtr<NV> >& outs) {
    /*LOG(ERROR) << " compute of SassConvBatchnormScaleReluPool ";
    float * h_data = new float[outs[0]->size()];//valid_size()];
    LOG(ERROR) << " outs[0]->valid_size() : " << outs[0]->size();
    cudaMemcpy(h_data, outs[0]->mutable_data(), outs[0]->size()*sizeof(float), cudaMemcpyDeviceToHost);
    cudaDeviceSynchronize();
    CUDA_CHECK(cudaPeekAtLastError()); 
    LOG(ERROR) << "over "; */
    auto* impl = static_cast<SassConvBatchnormScaleReluPoolHelper<NV, Precision::FP32>*>(this->_helper);
    auto& param = static_cast<SassConvBatchnormScaleReluPoolHelper<NV, Precision::FP32>*>(this->_helper)->_param_conv_batchnorm_scale_relu_pooling;
    impl->_funcs_conv_batchnorm_scale_relu_pooling(ins, outs, param, ctx);
}
#endif

/// TODO ... specialization other type of operator


/// set helper
template<typename Ttype, Precision Ptype>
SassConvBatchnormScaleReluPoolHelper<Ttype, Ptype>::~SassConvBatchnormScaleReluPoolHelper() {
}

template<typename Ttype, Precision Ptype>
Status SassConvBatchnormScaleReluPoolHelper<Ttype, Ptype>::InitParam() {
    DLOG(WARNING) << "Parsing SassConvBatchnormScaleReluPool op parameter.";
    saber::ConvParam<Ttype> _conv_param;
    PoolingParam<Ttype> _pooling_param;
    // get conv param
    auto group = GET_PARAMETER(int, group);
    auto bias_term = GET_PARAMETER(bool, bias_term);
    auto padding = GET_PARAMETER(PTuple<int>, padding);
    auto strides = GET_PARAMETER(PTuple<int>, strides);
    auto dilation_rate = GET_PARAMETER(PTuple<int>, dilation_rate);
    auto filter_num = GET_PARAMETER(int, filter_num);
    auto kernel_size = GET_PARAMETER(PTuple<int>, kernel_size);
    auto axis = GET_PARAMETER(int, axis);

	using pblock_type = PBlock<Ttype>;
    auto weights = GET_PARAMETER(pblock_type, weight_1);
    auto weight_vec = weights.vector();


    if (bias_term) {
        auto bias = GET_PARAMETER(pblock_type, weight_2);
        saber::ConvParam<Ttype> conv_param(group, padding[0], padding[1],
                                                            strides[0], strides[1],
                                                            dilation_rate[0], dilation_rate[1],
                                                            &(weights.d_tensor()), &(bias.d_tensor()));
        _conv_param = conv_param;
    } else {
        Tensor4d<Ttype>* bias = new Tensor4d<Ttype>();

        saber::ConvParam<Ttype> conv_param(group, padding[0], padding[1],
                                                            strides[0], strides[1],
                                                            dilation_rate[0], dilation_rate[1],
                                                            &(weights.d_tensor()), bias);
        _conv_param = conv_param;
    }


    // get batchnorm param
    auto epsilon = GET_PARAMETER(float, batchnorm_0_epsilon);
    auto momentum = GET_PARAMETER(float, batchnorm_0_momentum);
    auto batch_norm_weight_1 = GET_PARAMETER(pblock_type, batchnorm_0_weight_1);
    auto batch_norm_weight_1_vector = batch_norm_weight_1.vector();
    auto batch_norm_weight_2 = GET_PARAMETER(pblock_type, batchnorm_0_weight_2);
    auto batch_norm_weight_2_vector = batch_norm_weight_2.vector();
    auto batch_norm_weight_3 = GET_PARAMETER(pblock_type, batchnorm_0_weight_3);
    auto batch_norm_weight_3_vector = batch_norm_weight_3.vector();
    BatchnormParam<Ttype> batchnorm_param(batch_norm_weight_1_vector, 
                                                           batch_norm_weight_2_vector, 
                                                           batch_norm_weight_3_vector[0], 
                                                           momentum, epsilon);
    // get scale param
    auto scale_num_axes = GET_PARAMETER(int, scale_0_num_axes);
    auto scale_bias_term = GET_PARAMETER(bool, scale_0_bias_term);
    auto scale_axis = GET_PARAMETER(int, scale_0_axis);
    auto scale_weight_1 = GET_PARAMETER(pblock_type, scale_0_weight_1);
    auto scale_weight_1_vector = scale_weight_1.vector();
    auto scale_weight_2 = GET_PARAMETER(pblock_type, scale_0_weight_2);
    auto  scale_weight_2_vector = scale_weight_2.vector();
    saber::ScaleParam<Ttype> scale_param(scale_weight_1_vector,  scale_weight_2_vector, 
                                                          scale_bias_term, scale_axis, scale_num_axes);

    // get relu param
    auto alpha = GET_PARAMETER(float, relu_0_alpha);
    ActivationParam<Ttype> active_param(Active_relu);//, alpha); // Temp

    // get pooling param
    auto global_pooling = GET_PARAMETER(bool, pooling_0_global_pooling);
    auto pool_padding = GET_PARAMETER(PTuple<int>, pooling_0_padding);
    auto pool_strides = GET_PARAMETER(PTuple<int>, pooling_0_strides);
    auto pool_size = GET_PARAMETER(PTuple<int>, pooling_0_pool_size);
    auto pool_method = GET_PARAMETER(std::string, pooling_0_method);
    auto cmp_out_shape_floor_as_conv = GET_PARAMETER(bool, pooling_0_cmp_out_shape_floor_as_conv);
    if (pool_method == "MAX") {
        PoolingParam<Ttype> pooling_param(pool_size[0], pool_size[1],
                                                           pool_padding[0], pool_padding[1],
                                                           pool_strides[0], pool_strides[1],
                                                           Pooling_max, global_pooling,
                                                           cmp_out_shape_floor_as_conv);
        _pooling_param = pooling_param;
    } else if (pool_method == "AVG") {
        PoolingParam<Ttype> pooling_param(pool_size[0], pool_size[1],
                                                           pool_padding[0], pool_padding[1],
                                                           pool_strides[0], pool_strides[1],
                                                           Pooling_average_include_padding, global_pooling,
                                                           cmp_out_shape_floor_as_conv);
        _pooling_param = pooling_param;
    } else {
        LOG(FATAL) << " SassConvBatchnormScaleReluPool fusion op doesn't support : " << pool_method << " pooling.";
    }

    ConvPoolingParam<Ttype> conv_act_pooling_param;/*(_conv_param, 
                                                   batchnorm_param, 
                                                   scale_param, 
                                                   active_param, 
                                                   _pooling_param);*/
    _param_conv_batchnorm_scale_relu_pooling = conv_act_pooling_param;
    return Status::OK();
}

template<typename Ttype, Precision Ptype>
Status SassConvBatchnormScaleReluPoolHelper<Ttype, Ptype>::Init(OpContext<Ttype> &ctx, 
                                                                   const std::vector<Tensor4dPtr<Ttype> >& ins,
                                                                   std::vector<Tensor4dPtr<Ttype> >& outs) {
    _funcs_conv_batchnorm_scale_relu_pooling.init(ins, outs, _param_conv_batchnorm_scale_relu_pooling, SPECIFY, SABER_IMPL/*VENDER_IMPL*/, ctx);
    return Status::OK();
}

template<typename Ttype, Precision Ptype>
Status SassConvBatchnormScaleReluPoolHelper<Ttype, Ptype>::InferShape(const std::vector<Tensor4dPtr<Ttype> >& ins,
                                                                         std::vector<Tensor4dPtr<Ttype> >& outs) {
   SABER_CHECK(_funcs_conv_batchnorm_scale_relu_pooling.compute_output_shape(ins, outs, _param_conv_batchnorm_scale_relu_pooling));
   return Status::OK();
}

#ifdef USE_CUDA
template class SassConvBatchnormScaleReluPoolHelper<NV, Precision::FP32>;
template class SassConvBatchnormScaleReluPoolHelper<NV, Precision::FP16>;
template class SassConvBatchnormScaleReluPoolHelper<NV, Precision::INT8>;
#endif

#ifdef USE_ARM_PLACE
template class SassConvBatchnormScaleReluPoolHelper<ARM, Precision::FP32>;
template class SassConvBatchnormScaleReluPoolHelper<ARM, Precision::FP16>;
template class SassConvBatchnormScaleReluPoolHelper<ARM, Precision::INT8>;
#endif

// register helper 
#ifdef USE_CUDA
ANAKIN_REGISTER_OP_HELPER(SassConvBatchnormScaleReluPool, SassConvBatchnormScaleReluPoolHelper, NV, Precision::FP32);
#endif 

#ifdef USE_ARM_PLACE
ANAKIN_REGISTER_OP_HELPER(SassConvBatchnormScaleReluPool, SassConvBatchnormScaleReluPoolHelper, ARM, Precision::FP32);
#endif

//! register op
ANAKIN_REGISTER_OP(SassConvBatchnormScaleReluPool)
    .Doc("SassConvBatchnormScaleReluPool fusion operator")
#ifdef USE_CUDA
    .__alias__<NV, Precision::FP32>("convolution_batchnorm_scale_relu_pooling")
#endif
#ifdef USE_ARM_PLACE
    .__alias__<ARM, Precision::FP32>("convolution_batchnorm_scale_relu_pooling")
#endif
    .num_in(1)
    .num_out(1)
    .Args<int>("group", " group of conv ")
    .Args<bool>("bias_term", " whether conv weights have bias")
    .Args<PTuple<int>>("padding", "padding of conv (x, y)")
    .Args<PTuple<int>>("strides", "strides of conv (x)")
    .Args<PTuple<int>>("dilation_rate", "dilation rate of conv (x)")
    .Args<int>("filter_num", "filter(kernel) number of weights")
    .Args<PTuple<int>>("kernel_size", "kernel size of kernel (x, y)")
    .Args<int>("axis", "axis of conv")
    .Args<bool>("pooling_0_global_pooling", " whether use pooling for all input area.")
    .Args<PTuple<int>>("pooling_0_padding", " paddding of pooling ")
    .Args<PTuple<int>>("pooling_0_strides", " strides of pooling ")
    .Args<PTuple<int>>("pooling_0_pool_size", "pooling size of pooling")
    .Args<std::string>("pooling_0_method", " pooling methods")
    .Args<bool>("pooling_0_cmp_out_shape_floor_as_conv", "cmp_out_shape_floor_as_conv")
    .Args<float>("relu_0_alpha", " alpha for relu")
    .Args<int>("scale_0_num_axes", " num axes for scale")
    .Args<bool>("scale_0_bias_term", "whether scale has bias")
    .Args<int>("scale_0_axis", "axis for scale")
    .Args<float>("batchnorm_0_epsilon", "epsilon for batchnorm")
    .Args<float>("batchnorm_0_momentum", "momentum for batchnorm");

} /* namespace ops */

} /* namespace anakin */


