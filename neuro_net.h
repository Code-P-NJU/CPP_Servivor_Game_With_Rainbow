#pragma once
#undef slots
#define NOMINMAX
#undef min
#undef max
#include <torch/torch.h>
#include "ResNet.h"
#define slots Q_SLOTS
extern c10::Device device;
extern int batch_size;
extern int atoms;
//噪声网络的构建参考https://zhuanlan.zhihu.com/p/490357062
class NoisyLinearImpl :public torch::nn::Module
{
	int in_dim, out_dim;
	double sigma_zero;
	torch::Tensor weight_mu, weight_sigma, bias_mu, bias_sigma, weight_eps, bias_eps;
public:
	NoisyLinearImpl(int in_dim, int out_dim, double sigma_zero = 0.5, bool bias = true) :in_dim(in_dim), out_dim(out_dim), sigma_zero(sigma_zero)
	{
		weight_mu = register_parameter("weight_mu", torch::zeros({ out_dim,in_dim }).to(torch::kFloat));
		weight_sigma = register_parameter("weight_sigma", torch::zeros({ out_dim,in_dim }).to(torch::kFloat));
		bias_mu = register_parameter("bias_mu", torch::zeros({ out_dim }).to(torch::kFloat));
		bias_sigma = register_parameter("bias_sigma", torch::zeros({ out_dim }).to(torch::kFloat));
		weight_eps = register_buffer("weight_eps", torch::zeros({ out_dim,in_dim }).to(torch::kFloat));
		bias_eps = register_buffer("bias_eps", torch::zeros({ out_dim }).to(torch::kFloat));
		reset_paramenters();
		reset_noise();
	}
	torch::Tensor forward(torch::Tensor x)
	{
		torch::Tensor weight, bias;
		if (is_training())
		{
			weight = weight_mu+(weight_sigma.mul(weight_eps));
			bias = bias_mu+(bias_sigma.mul(bias_eps));
			//std::cout << bias_sigma;
		}
		else
		{
			weight = weight_mu;
			bias = bias_mu;
		}
		//std::cout << weight.sizes() << bias.sizes()<<std::endl;
		return torch::nn::functional::linear(x, weight, bias);
	}
	void reset_sigma(double bias=1) 
	{
		weight_sigma.data().fill_(sigma_zero / sqrt(bias_sigma.size(0))*bias);
		bias_sigma.data().fill_(sigma_zero / sqrt(bias_sigma.size(0))*bias);
	}
	void reset_paramenters()
	{
		auto mu_range = 1 / sqrt(weight_mu.size(1));
		weight_mu.data().uniform_(-mu_range, mu_range);
		weight_sigma.data().fill_(sigma_zero / sqrt(bias_sigma.size(0)));
		bias_mu.data().uniform_(-mu_range, mu_range);
		bias_sigma.data().fill_(sigma_zero / sqrt(bias_sigma.size(0)));
	}
	void reset_noise()
	{
		auto eps_in = _scale_noise(in_dim);
		auto eps_out = _scale_noise(out_dim);
		weight_eps.copy_(eps_out.ger(eps_in));
		bias_eps.copy_(_scale_noise(out_dim));
	}
	torch::Tensor _scale_noise(int size)
	{
		auto x = torch::randn({ size });
		x = x.sign().mul(x.abs().sqrt());
		return x;
	}
};
TORCH_MODULE(NoisyLinear);
//引入distributional dqn,参考https://github.com/Kaixhin/Rainbow/blob/master/
class neuro_netImpl :public torch::nn::Module
{
	int actions=1;
	ResNet_ocr ResNet = nullptr;
	//
	torch::nn::AvgPool2d pool = nullptr;//池化层
	NoisyLinear Linear1 = nullptr;//线性层1
	torch::nn::Linear Linear2 = nullptr;//线性层2
	NoisyLinear Linear3 = nullptr;//线性层3
	//用来预测state_value
	NoisyLinear Linear4 = nullptr;//线性层4
	torch::nn::Linear Linear5 = nullptr;//线性层5
	NoisyLinear Linear6 = nullptr;//线性层6
	//用来预测action_value
	//
	torch::nn::LeakyReLU acti;//激活层使用leakyRelu
	torch::nn::LayerNorm LN1 = nullptr;
	torch::nn::LayerNorm LN2 = nullptr;
	torch::nn::LayerNorm LN3 = nullptr;
	torch::nn::LayerNorm LN4 = nullptr;
	torch::nn::LayerNorm LN5 = nullptr;
public:
	neuro_netImpl(int observations, int actions):actions(actions)
	{
		//  /*  
		ResNet = register_module("ResNet", ResNet_ocr());
		Linear1 = register_module(std::string("Linear1"), NoisyLinear(512, 512));
		Linear2 = register_module(std::string("Linear2"), torch::nn::Linear(512, 512));
		Linear3 = register_module(std::string("Linear3"), NoisyLinear(512, atoms));
		Linear4 = register_module(std::string("Linear4"), NoisyLinear(512,512));
		Linear5 = register_module(std::string("Linear5"), torch::nn::Linear(512, 512));
		Linear6 = register_module(std::string("Linear6"), NoisyLinear(512, actions*atoms));
		acti = register_module(std::string("acti"), torch::nn::LeakyReLU());
		pool = register_module(std::string("pool"), torch::nn::AvgPool2d(torch::nn::AvgPool2dOptions({4,6})));
		LN1 = register_module(std::string("LN1"), torch::nn::LayerNorm(torch::nn::LayerNormOptions(std::vector<int64_t>{ 512 }).elementwise_affine(1)));
		LN2 = register_module(std::string("LN2"), torch::nn::LayerNorm(torch::nn::LayerNormOptions(std::vector<int64_t>{ 512 }).elementwise_affine(1)));
		LN3 = register_module(std::string("LN3"), torch::nn::LayerNorm(torch::nn::LayerNormOptions(std::vector<int64_t>{ 512 }).elementwise_affine(1)));
		LN4 = register_module(std::string("LN4"), torch::nn::LayerNorm(torch::nn::LayerNormOptions(std::vector<int64_t>{ 512 }).elementwise_affine(1)));
		LN5 = register_module(std::string("LN5"), torch::nn::LayerNorm(torch::nn::LayerNormOptions(std::vector<int64_t>{ 512 }).elementwise_affine(1)));
		//  */

		/*
		conv4->to(device);
		conv4->weight.data().normal_(0, 0.1);
		*/

		Linear1->to(device);
		//Linear1->weight.data().normal_(0, 0.1);

		Linear2->to(device);
		Linear2->weight.data().normal_(0, 0.1);

		Linear3->to(device);
		//Linear3->weight.data().normal_(0, 0.1);

		Linear4->to(device);
		//Linear4->weight.data().normal_(0, 0.1);

		Linear5->to(device);
		Linear5->weight.data().normal_(0, 0.1);

		Linear6->to(device);
		//Linear6->weight.data().normal_(0, 0.1);
		acti->to(device);
		LN1->to(device);
		LN2->to(device);
		LN3->to(device);
		LN4->to(device);
		LN5->to(device);
	};//初始化
	void reset_noise()
	{
		Linear1->reset_noise();
		Linear3->reset_noise();
		Linear4->reset_noise();
		Linear6->reset_noise();
	}
	void reset_sigma(double bias =1) 
	{
		Linear1->reset_sigma(bias);
		Linear3->reset_sigma(bias);
		Linear4->reset_sigma(bias);
		Linear6->reset_sigma(bias);
	}
	torch::Tensor forward(torch::Tensor x)
	{
		x = x.to(device);
		x = x / 255.0;
		x = ResNet->forward(x);
		//std::cout << x.sizes();
		x = pool->forward(x);

		auto state_v = LN1(x.view({ x.size(0), -1 }));
		//auto state_v = x.view({ x.size(0), -1 });
		auto action_v = state_v.clone();

		state_v = Linear2->forward(state_v);
		state_v = acti(state_v);
		state_v = LN2(state_v);

		state_v = Linear1->forward(state_v);
		state_v = acti(state_v);
		state_v = LN3(state_v);

		state_v = Linear3->forward(state_v);

		action_v = Linear5->forward(action_v);
		action_v = acti(action_v);
		action_v = LN4(action_v);

		action_v = Linear4->forward(action_v);
		action_v = acti(action_v);
		action_v = LN5(action_v);

		action_v = Linear6->forward(action_v);

		state_v = state_v.view({ -1,1,atoms });
		action_v = action_v.view({-1, actions, atoms});

		auto action_mean = action_v.mean(1, 1);
		x = action_v + state_v - action_mean;
		x = torch::nn::functional::softmax(x, torch::nn::functional::SoftmaxFuncOptions(2));
		//std::cout << "action_v:" << action_v <<std::endl<<"state_v:" << state_v <<std::endl<<"action_mean:"<< action_mean <<std::endl<<"action:" << x;
		return x;
	};//前向传播函数
};
TORCH_MODULE(neuro_net);

class neuro_net_v1Impl :public torch::nn::Module
{
	int actions = 1;
	torch::nn::Conv2d conv1 = nullptr;//卷积层1
	torch::nn::Conv2d conv2 = nullptr;//卷积层2
	torch::nn::Conv2d conv3 = nullptr;//卷积层3
	torch::nn::Conv2d conv4 = nullptr;//卷积层4
	torch::nn::MaxPool2d pool = nullptr;//池化层
	torch::nn::Dropout dropout = nullptr;//dropout层减轻过拟合
	//
	NoisyLinear Linear1 = nullptr;//线性层1
	torch::nn::Linear Linear2 = nullptr;//线性层2
	NoisyLinear Linear3 = nullptr;//线性层3
	//用来预测state_value
	NoisyLinear Linear4 = nullptr;//线性层4
	torch::nn::Linear Linear5 = nullptr;//线性层5
	NoisyLinear Linear6 = nullptr;//线性层6
	//用来预测action_value
	//
	torch::nn::LeakyReLU acti;//激活层使用leakyRelu
	torch::nn::LayerNorm LN1 = nullptr;
	torch::nn::LayerNorm LN2 = nullptr;
	torch::nn::LayerNorm LN3 = nullptr;
	torch::nn::LayerNorm LN4 = nullptr;
	torch::nn::LayerNorm LN5 = nullptr;
public:
	neuro_net_v1Impl(int observations, int actions):actions(actions)
	{
		//  /*  
		conv1 = register_module(std::string("conv1"), torch::nn::Conv2d(torch::nn::Conv2dOptions(3, 64, 8).stride(4).padding(3)));
		conv2 = register_module(std::string("conv2"), torch::nn::Conv2d(torch::nn::Conv2dOptions(64, 128, 4).stride(2).padding(2)));
		conv3 = register_module(std::string("conv3"), torch::nn::Conv2d(torch::nn::Conv2dOptions(128, 256, 3).stride(2).padding(1)));
		conv4 = register_module(std::string("conv4"), torch::nn::Conv2d(torch::nn::Conv2dOptions(256, 512, {2,3}).stride(1)));
		pool = register_module(std::string("pool"), torch::nn::MaxPool2d(torch::nn::MaxPool2dOptions(2).stride(2)));
		dropout = register_module(std::string("dropout"), torch::nn::Dropout(torch::nn::DropoutOptions(0.3)));
		Linear1 = register_module(std::string("Linear1"), NoisyLinear(512, 512));
		Linear2 = register_module(std::string("Linear2"), torch::nn::Linear(512, 512));
		Linear3 = register_module(std::string("Linear3"), NoisyLinear(512, atoms));
		Linear4 = register_module(std::string("Linear4"), NoisyLinear(512, 512));
		Linear5 = register_module(std::string("Linear5"), torch::nn::Linear(512, 512));
		Linear6 = register_module(std::string("Linear6"), NoisyLinear(512, actions*atoms));
		acti = register_module(std::string("acti"), torch::nn::LeakyReLU());

		LN1 = register_module(std::string("LN1"), torch::nn::LayerNorm(torch::nn::LayerNormOptions(std::vector<int64_t>{ 512 }).elementwise_affine(0)));
		LN2 = register_module(std::string("LN2"), torch::nn::LayerNorm(torch::nn::LayerNormOptions(std::vector<int64_t>{ 512 }).elementwise_affine(0)));
		LN3 = register_module(std::string("LN3"), torch::nn::LayerNorm(torch::nn::LayerNormOptions(std::vector<int64_t>{ 512 }).elementwise_affine(0)));
		LN4 = register_module(std::string("LN4"), torch::nn::LayerNorm(torch::nn::LayerNormOptions(std::vector<int64_t>{ 512 }).elementwise_affine(0)));
		LN5 = register_module(std::string("LN5"), torch::nn::LayerNorm(torch::nn::LayerNormOptions(std::vector<int64_t>{ 512 }).elementwise_affine(0)));
		//  */

		conv1->to(device);
		conv1->weight.data().normal_(0, 1);

		conv2->to(device);
		conv2->weight.data().normal_(0, 1);

		conv3->to(device);
		conv3->weight.data().normal_(0, 1);
		
		conv4->to(device);
		conv4->weight.data().normal_(0, 1);
		

		Linear1->to(device);
		//Linear1->weight.data().normal_(0, 0.1);

		Linear2->to(device);
		Linear2->weight.data().normal_(0, 1);

		Linear3->to(device);
		//Linear3->weight.data().normal_(0, 0.1);

		Linear4->to(device);
		//Linear4->weight.data().normal_(0, 0.1);

		Linear5->to(device);
		Linear5->weight.data().normal_(0, 1);

		Linear6->to(device);
		//Linear6->weight.data().normal_(0, 0.1);
		pool->to(device);
		acti->to(device);
		dropout->to(device);
		LN1->to(device);
		LN2->to(device);
		LN3->to(device);
		LN4->to(device);
		LN5->to(device);
	};//初始化
	void reset_noise()
	{
		Linear1->reset_noise();
		Linear3->reset_noise();
		Linear4->reset_noise();
		Linear6->reset_noise();
	}
	torch::Tensor forward(torch::Tensor x)
	{
		x = x.to(device);
		x = x / 255.0;
		x = conv1->forward(x);
		x = acti(x);
		//x = pool(x);

		x = conv2->forward(x);
		x = acti(x);
		x = pool(x);

		x = conv3->forward(x);
		x = acti(x);
		x = pool(x);

		x = conv4->forward(x);
		x = acti(x);

		auto state_v = LN1(x.view({ x.size(0), -1 }));
		auto action_v = state_v.clone();

		state_v = Linear2->forward(state_v);
		state_v = acti(state_v);
		state_v = LN2(state_v);

		state_v = Linear1->forward(state_v);
		state_v = acti(state_v);
		state_v = LN3(state_v);

		state_v = Linear3->forward(state_v);

		action_v = Linear5->forward(action_v);
		action_v = acti(action_v);
		action_v = LN4(action_v);

		action_v = Linear4->forward(action_v);
		action_v = acti(action_v);
		action_v = LN5(action_v);

		action_v = Linear6->forward(action_v);

		state_v = state_v.view({ -1,1,atoms });
		action_v = action_v.view({ -1, actions, atoms });

		auto action_mean = action_v.mean(1, 1);
		x = action_v + state_v - action_mean;
		x = torch::nn::functional::softmax(x, torch::nn::functional::SoftmaxFuncOptions(2));
		//std::cout << "action_v:" << action_v <<std::endl<<"state_v:" << state_v <<std::endl<<"action_mean:"<< action_mean <<std::endl<<"action:" << x;
		return x;
	};//前向传播函数

};
TORCH_MODULE(neuro_net_v1);