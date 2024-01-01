#pragma once
#include <torch/script.h>
#include "neuro_net.h"
#include <random>
#include "ReplayMemory.h"
extern c10::Device device;
using transition = std::tuple<torch::Tensor, int, float, torch::Tensor, bool>;//五个tensor对应状态 动作 奖励 次态 结束
//using samples = std::tuple<std::vector<torch::Tensor>, std::vector<int>, std::vector<float>, std::vector<torch::Tensor>, std::vector<int>>;
using samples = std::tuple<std::vector<torch::Tensor>, std::vector<int>, std::vector<float>, std::vector<torch::Tensor>, std::vector<int>, std::vector<int>, std::vector<float>>;
extern double gamma, lr,max_lr,min_lr; //, epsilon;
extern int target_update;
extern int N_step_replay;
extern int steplr_step_size;
extern float steplr_gamma,alpha,beta;
extern int batch_size;
extern int V_max, V_min;
extern c10::Scalar loss_v;
class agent 
{
	int  action_dim;//动作空间
	neuro_net q_net,//决策网络
		target_q_net;//目标网络
	unsigned int count=0;//学习次数
	//torch::nn::HuberLoss loss;//损失函数
	//distributional 
	torch::Tensor support;
	double d_z = 1.0 * (V_max - V_min) / (atoms - 1.0);
public:	
	PriorityReplayMemory memory;
	bool train=1;
	torch::optim::Adam optimizer;
	//torch::optim::StepLR steplr;
	void save(const std::string Path)
	{
		torch::serialize::OutputArchive archive;
		q_net->save(archive);
		archive.save_to(Path);
	};
	
	void load(const std::string Path)
	{
		torch::serialize::InputArchive archive;
		archive.load_from(Path);
		q_net->load(archive);
		//q_net->reset_sigma();
		target_q_net->load(archive);
		//target_q_net->reset_sigma();
	}
	
	agent(int observation_dim, int action_dim)
		:action_dim(action_dim), q_net(observation_dim, action_dim), target_q_net(observation_dim, action_dim), memory(15)// ,loss(torch::nn::HuberLossOptions().delta(delta))
		, optimizer(q_net->parameters(), torch::optim::AdamOptions(lr)) //, steplr(optimizer, steplr_step_size, steplr_gamma),
	{
		support = torch::linspace(V_min, V_max, atoms).to(device);
		std::stringstream stream;
		torch::save(q_net, stream);
		torch::load(target_q_net, stream);
		q_net->to(device);
		target_q_net->to(device);
		for (auto& p : target_q_net->parameters())
		{
			p.detach_();
		}
	}
	int take_action(torch::Tensor state)
	{
		target_q_net->train(train);
		q_net->train(train);
		state = state.unsqueeze(0).to(device);
		//噪声网络dqn探索，与epsilon无关
		q_net->reset_noise();
		//distributional
		int action = (q_net->forward(state)* support).sum(2).argmax(1).item<int>();
		/*
		torch::Tensor actions_value = q_net->forward(state);
		int action = actions_value.argmax(1).item<int>();
		*/
		//std::cout << actions_value << std::endl;
		return action;
		/*正常dqn探索
		if (torch::rand(1).item<double>() <1-epsilon)//贪心
		{
			torch::Tensor actions_value = q_net->forward(state);
			int action = actions_value.argmax(1).item<int>();
			//std::cout << actions_value << std::endl;
			return action;
		}
		else//随机
		{
			std::random_device rd; 
			std::mt19937 gen(rd()); 
			std::uniform_int_distribution<> dist(0,action_dim-1);
			//避免使用srand对环境造成影响
			return dist(gen);
		}
		*/
	}
	void update(samples transition_dict)
	{		
		//torch::optim::Adam opt(q_net->parameters(),lr);//优化器使用Adam
		q_net->train();
		target_q_net->train();
		q_net->reset_noise();
		target_q_net->reset_noise();
		//torch::optim::Adam optimizer(q_net->parameters(),torch::optim::AdamOptions(lr).eps(min_lr));
		//抽取数据
		torch::Tensor states = torch::stack(std::get<0>(transition_dict)).to(torch::kFloat).to(device);
		torch::Tensor actions = torch::stack(torch::tensor(std::get<1>(transition_dict)),-1).to(torch::kInt64).to(device);
		torch::Tensor rewards = torch::stack(torch::tensor(std::get<2>(transition_dict)),-1).to(torch::kFloat).to(device);
		torch::Tensor next_states = torch::stack(std::get<3>(transition_dict)).to(torch::kFloat).to(device);
		torch::Tensor dones = torch::stack(torch::tensor(std::get<4>(transition_dict)),-1).to(torch::kInt64).to(device);
		torch::Tensor w = torch::stack(torch::tensor(std::get<6>(transition_dict)), -1).to(torch::kFloat).to(device);
		auto indexs= std::get<5>(transition_dict);
		//distributional:
		auto ps=q_net->forward(states).to(device);
		//std::cout << actions.sizes();
		auto ps_a = ps.gather(1, actions.unsqueeze(-1).expand({ ps.size(0),1,ps.size(2)})).to(device).squeeze(1);
		//std::cout << log_ps_a.sizes();
		torch::Tensor m;
		{;
			torch::NoGradGuard no_grad;
			auto pns = q_net->forward(next_states).to(device);
			auto dns = support.expand_as(pns) * pns;
			auto argmax_indices_ns = dns.sum(2).argmax(1).unsqueeze(-1);
			//std::cout << argmax_indices_ns.sizes();
			pns = target_q_net->forward(next_states);
			auto t = argmax_indices_ns.unsqueeze(-1).expand({ pns.size(0),1,pns.size(2) }).to(device).to(torch::kLong);
			auto pns_a = pns.gather(1, t).to(device).squeeze(1);
			//下面计算Tz
			auto Tz = rewards + dones * std::pow(gamma, N_step_replay) * support.unsqueeze(0);
			Tz = Tz.clamp(V_min, V_max);
			auto b = (Tz - V_min) / d_z;
			auto l = b.floor().to(torch::kInt64), u = b.ceil().to(torch::kInt64);
			l -= ((u > 0) * (l == u)).to(l);
			u += ((l < (atoms - 1)) * (l == u)).to(u);
			m = states.new_zeros({ batch_size, atoms }).to(device);
			auto offset = torch::linspace(0, ((batch_size - 1) * atoms), batch_size).unsqueeze(1).expand({batch_size, atoms}).to(actions);
			//std::cout << offset.sizes()<< l.sizes() << u.sizes()<<b.sizes()<<pns_a.sizes();
			m.view(-1).index_add_(0, (l + offset).view(-1), (pns_a * (u.to(torch::kFloat) - b)).view(-1));
			m.view(-1).index_add_(0, (u + offset).view(-1), (pns_a * (b - l.to(torch::kFloat))).view(-1));
		}
		auto Loss = -torch::sum(m * ((ps_a).log()), 1);
		//std::cout << Loss;
		auto loss = (w * Loss).mean();
		loss_v = loss.item();
		q_net->zero_grad();
		loss.backward();
		optimizer.step();
		for (int i = 0; i < batch_size; ++i)
		{
			double td_err = pow(abs(Loss[i].item().toDouble()) + 1e-5, alpha);
			//std::cout <<"err="<< td_err << std::endl;
			memory.update_priority(indexs[i], td_err);
		}
		/*非dis
		torch::Tensor predict_q_values = q_net->forward(states).gather(1, actions).to(device);//计算预测q	
		torch::Tensor max_action = std::get<1>(q_net->forward(next_states).max(1)).view({-1,1}).to(device).set_requires_grad(0);//max返回一个tuple,0号是最大值，1号是最大值的下标
		torch::Tensor max_next_q_values = target_q_net->forward(next_states).gather(1, max_action).to(device).set_requires_grad(0);
		//此处使用q_net选出下一步的action，之后用这个action在target_q_net中选取目标最优q值
		// 这一算法是double Q思想，用q_net评估action，然后用target_q_net来计算max_next_q_values，使用两个网络平均两个网络的误差
		// 下面的是经典DQN的过程，仅使用target_q_net计算q值，,通过max选出最大q,随着不断迭代，q_net的噪声会不断叠加
		//torch::Tensor max_next_q_values = std::get<0>(target_q_net.forward(next_states).detach().max(1)).view({-1, 1}).to(device).set_requires_grad(0);//计算最大q
		//
		torch::Tensor q_targets = rewards+ std::pow(gamma,N_step_replay) * max_next_q_values * (1 - dones).to(device).set_requires_grad(0);//计算N步目标q
		//std::cout << dones << q_targets;
		//torch::Tensor q_td = rewards + std::pow(gamma, N_step_replay) * max_next_q_values.to(device).set_requires_grad(0);
		//torch::Tensor q_targets = rewards + gamma * max_next_q_values * (1 - dones).to(device).set_requires_grad(0);//计算目标q（传统dqn）
		torch::Tensor Loss =w.mul( (predict_q_values.sub(q_targets)).pow(2) );//计算损失
		//std::cout << Loss;
		Loss = Loss.mean();
		//torch::Tensor Loss = loss(predict_q_values, q_targets);
		//std::cout << Loss << std::endl;
		loss_v = Loss.item();
		optimizer.zero_grad();
		Loss.backward();
		optimizer.step();
		
		for (int i = 0; i < batch_size; ++i) 
		{
			float td_err=pow( abs(q_targets[i][0].item().toFloat() - predict_q_values[i][0].item().toFloat()) + 1e-5,alpha);
			//std::cout <<"err="<< td_err << std::endl;
			memory.update_priority(indexs[i], td_err);
		}
		*/
		if (count % target_update == 0)
		{
			count = 0;
			std::stringstream stream;
			torch::save(q_net, stream);
			torch::load(target_q_net, stream);
			q_net->to(device);
			target_q_net->to(device);
		}
		count++;
		//std::cout << "update count"<<count << std::endl;//
	}
};