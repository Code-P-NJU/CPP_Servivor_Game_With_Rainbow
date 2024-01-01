//此段代码参考自https://zhuanlan.zhihu.com/p/225597229和https://blog.csdn.net/weixin_52646021/article/details/122323828,结构做了一些修改
#pragma once
#include "torch/torch.h"
inline torch::nn::Conv2dOptions conv_options(int64_t in_planes, int64_t out_planes, int64_t kerner_size,
	int64_t stride = 1, int64_t padding = 0, int groups = 1, bool with_bias = true) {
	torch::nn::Conv2dOptions conv_options = torch::nn::Conv2dOptions(in_planes, out_planes, kerner_size);
	conv_options.stride(stride);
	conv_options.padding(padding);
	conv_options.bias(with_bias);
	conv_options.groups(groups);
	return conv_options;
}
//残差模块
class Block_ocrImpl : public torch::nn::Module 
{
private:
	bool is_basic = true;
	int64_t stride = 1;
	torch::nn::Conv2d conv1=nullptr;
	torch::nn::BatchNorm2d bn1= nullptr;
	torch::nn::Conv2d conv2= nullptr;
	torch::nn::BatchNorm2d bn2=nullptr;
	torch::nn::Conv2d conv3=nullptr;
	torch::nn::BatchNorm2d bn3=nullptr;
public:
	Block_ocrImpl(int64_t inplanes, int64_t planes, int64_t stride_ = 1,torch::nn::Sequential downsample_ = nullptr, int groups = 1, int base_width = 64, bool _is_basic = true)
    {
        downsample = downsample_;
        stride = stride_;
        int width = int(planes * (base_width / 64.0)) * groups;
        is_basic = _is_basic;
        bn1 = register_module("bn1", torch::nn::BatchNorm2d(torch::nn::BatchNorm2dOptions(width).affine(1)));
        bn2 = register_module("bn2", torch::nn::BatchNorm2d(torch::nn::BatchNorm2dOptions(width).affine(1)));
        if (is_basic)
        {
            conv1 = register_module("conv1", torch::nn::Conv2d(conv_options(inplanes, width, 3, stride_, 1, groups, false)));
            conv2 = register_module("conv2", torch::nn::Conv2d(conv_options(width, width, 3, 1, 1, groups, false)));
        }
        else {
            conv1 = register_module("conv1", torch::nn::Conv2d(conv_options(inplanes, width, 1, 1, 0, 1, false)));
            conv2 = register_module("conv2", torch::nn::Conv2d(conv_options(width, width, 3, stride_, 1, groups, false)));
            conv3 = register_module("conv3", torch::nn::Conv2d(conv_options(width, planes * 4, 1, 1, 0, 1, false)));
            bn3 = register_module("bn3", torch::nn::BatchNorm2d(torch::nn::BatchNorm2dOptions(planes * 4).affine(1)));
        }
        if (!downsample->is_empty()) {
            register_module("downsample", downsample);
        }
    }
	torch::Tensor forward(torch::Tensor x)
    {
        torch::Tensor residual = x.clone();

        x = conv1->forward(x);
        x = bn1->forward(x);
        x = torch::relu(x);

        x = conv2->forward(x);
        x = bn2->forward(x);

        if (!is_basic) {
            x = torch::relu(x);
            x = conv3->forward(x);
            x = bn3->forward(x);
        }

        if (!downsample->is_empty()) {
            residual = downsample->forward(residual);
        }

        x += residual;
        x = torch::relu(x);

        return x;
    }
	torch::nn::Sequential downsample=nullptr;
};
TORCH_MODULE(Block_ocr);
//残差网络
class ResNet_ocrImpl : public torch::nn::Module 
{
private:
    int expansion = 1; bool is_basic = true;
    int64_t inplanes = 64; 
    int groups = 1; 
    int base_width = 64;
    torch::nn::Conv2d conv1=nullptr;
    torch::nn::BatchNorm2d bn1=nullptr;
    torch::nn::Sequential layer1=nullptr;
    torch::nn::Sequential layer2=nullptr;
    torch::nn::Sequential layer3=nullptr;
    torch::nn::Sequential layer4=nullptr;
public:
    ResNet_ocrImpl(bool isbasic=1,int _groups = 1, int _width_per_group = 64)
    {
        if (!isbasic)
        {
            expansion = 4;
            is_basic = false;
        }
        groups = _groups;
        base_width = _width_per_group;
        conv1 = register_module("conv1", torch::nn::Conv2d(conv_options(3, 64, 7, 2, 3, 1, false)));
        bn1 = register_module("bn1", torch::nn::BatchNorm2d(torch::nn::BatchNorm2dOptions(64).affine(1)));
        layer1 = register_module("layer1", torch::nn::Sequential(_make_layer(64, 1,1)));
        layer2 = register_module("layer2", torch::nn::Sequential(_make_layer(128, 1, 2)));
        layer3 = register_module("layer3", torch::nn::Sequential(_make_layer(256, 1, 2)));
        layer4 = register_module("layer4", torch::nn::Sequential(_make_layer(512, 1, 2)));
        for (auto& module : modules(0))
        {
            if (auto M = dynamic_cast<torch::nn::Conv2dImpl*>(module.get()))
            {
                torch::nn::init::kaiming_normal_(M->weight, 0, torch::kFanOut, torch::kReLU);
            }
            else if (auto M = dynamic_cast<torch::nn::BatchNorm2dImpl*>(module.get()))
            {
                torch::nn::init::constant_(M->weight, 1);
                torch::nn::init::constant_(M->bias, 0);
            }
        }
    }

    torch::nn::Sequential _make_layer(int64_t planes, int64_t blocks, int64_t stride = 1)
    {
        torch::nn::Sequential downsample;
        if (stride != 1 || inplanes != planes * expansion) {
            downsample = torch::nn::Sequential(
                torch::nn::Conv2d(conv_options(inplanes, planes * expansion, 1, stride, 0, 1, false)),
                torch::nn::BatchNorm2d(torch::nn::BatchNorm2dOptions(planes * expansion).affine(1))
            );
        }
        torch::nn::Sequential layers;
        layers->push_back(Block_ocr(inplanes, planes, stride, downsample, groups, base_width, is_basic));
        inplanes = planes * expansion;
        for (int64_t i = 1; i < blocks; i++) {
            layers->push_back(Block_ocr(inplanes, planes, 1, torch::nn::Sequential(), groups, base_width, is_basic));
        }
        return layers;
    }
    torch::Tensor forward(torch::Tensor x)
    {
        x = conv1->forward(x);
        x = bn1->forward(x);
        x = torch::max_pool2d(x, 3, 2, 1);
        x = torch::relu(x);

        x = layer1->forward(x);
        x = layer2->forward(x);
        x = layer3->forward(x);
        x = layer4->forward(x);
        return x;
    }
};
TORCH_MODULE(ResNet_ocr);
