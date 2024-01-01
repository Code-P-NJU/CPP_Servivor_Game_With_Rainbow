#pragma once
#include <deque>
#include <algorithm>
#include "torch/torch.h"
#include <random>
#include "sum_tree.h"
extern int N_step_replay;
extern float alpha,beta;
extern double gamma;
using transition = std::tuple<torch::Tensor, int, float, torch::Tensor, bool>;//五个tensor对应状态 动作 奖励 次态 结束
using samples = std::tuple<std::vector<torch::Tensor>, std::vector<int>, std::vector<float>, std::vector<torch::Tensor>, std::vector<int>,std::vector<int>,std::vector<float>>;//bool类型vector似乎不能转成tensor，故这里抽取时done用int表示

class PriorityReplayMemory
{
	sum_tree tree;
	size_t size_=0;
	int memory_max_size;
	int curr = 0;
	transition* memory = nullptr;
public:
	PriorityReplayMemory(int mem_size) :tree(mem_size), memory_max_size(pow(2, mem_size))//实际memory_max_size为2^mem_size
	{
		memory = new transition[memory_max_size];
	}
	size_t size() //记忆重放池大小
	{
		return size_;
	}
	samples sample(int batch_size)//随机抽取batch_size个样本
	{
		//std::vector<transition> batch_data;
		std::vector<torch::Tensor> state, next_state;
		std::vector< int> action;
		std::vector<int> done;
		std::vector<float> reward;
		std::vector<float>weight;
		std::vector<int> index_;
		state.reserve(batch_size);
		next_state.reserve(batch_size);
		action.reserve(batch_size);
		done.reserve(batch_size);
		reward.reserve(batch_size);
		weight.reserve(batch_size);
		index_.reserve(batch_size);
		float len = tree.head()/batch_size;
		float Pmin = tree.min() / tree.head();
		if (Pmin == 0) 
		{
			Pmin = 0.0001;
		}
		//multi_step dqn:
		for (int i = 0; i < batch_size; ++i)
		{
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_real_distribution<double> dist(i * len + 1e-7, (i + 1) * len);
			int begin = tree.search(dist(gen));
			int k = 10;
			while (k != 0 && !((begin - curr + size_) % size_ <= size_ - N_step_replay))
			{
				begin = tree.search(dist(gen));
				k--;
			}
			if (k == 0)
			{
				std::uniform_real_distribution<float> dist2(1e-7, tree.head());
				begin = tree.search(dist2(gen));
				while (!((begin - curr + size_) % size_ <= size_ - N_step_replay))
				{
					begin = tree.search(dist2(gen));
					//std::cout << begin<<' ';
				}
			}
			index_.push_back(begin);
			weight.push_back(pow((tree[begin] / tree.head()) / Pmin, -::beta));
			float sum_reward = 0;
			torch::Tensor state_ahead;
			bool done_ahead;
			for (int j = 0; j < N_step_replay; ++j)
			{
				int index = (begin + j) % size_;
				sum_reward += std::pow(gamma, j) * std::get<2>(memory[index]);
				if (std::get<4>(memory[index]))
				{
					state_ahead = std::get<3>(memory[index]);
					done_ahead = true;
					break;
				}
				else
				{
					state_ahead = std::get<3>(memory[index]);
					done_ahead = false;
				}
			}
			/*
			if  (std::get<0>(memory[begin]).sizes()[0] == 0)
			{
				std::cout << "state dim err" << std::endl << " mem_size: " << size_ << " mem_curr:" << curr << " index: " << begin<<"sum:" << tree.head()<<std::endl;
				for (int i = 0; i < size_; ++i)
				{
					std::cout << tree[i]<<' ';
				}
				std::cin >> sum_reward;
			}
			if (state_ahead.sizes()[0] == 0)
			{
				std::cout << "state_ahead dim err" << std::endl << " mem_size: " << size_ << " mem_curr:" << curr << " index: " << begin << "sum:" << tree.head() << std::endl;
				for (int i = 0; i < size_; ++i)
				{
					std::cout << tree[i] << ' ';
				}
				std::cin >> sum_reward;
			}
			*/
			state.push_back(std::get<0>(memory[begin]));
			action.push_back(std::get<1>(memory[begin]));
			reward.push_back(sum_reward);
			next_state.push_back(state_ahead);
			done.push_back(done_ahead);
		}
		//std::cout << "sample done" << std::endl;//
		return  { state,action,reward,next_state,done,index_,weight};
	}
	void push(torch::Tensor state, int action, float reward, torch::Tensor next_state, bool done)
	{
		memory[curr] = { state, action, reward, next_state, done };
		tree.change_(curr,tree.max() + 0.5);
		curr++;
		if (curr == memory_max_size)
		{
			curr = 0;
		}
		if (size_ < memory_max_size)
		{
			++size_;
		}
		tree.length = size_;
	}
	void update_priority(int i,float p)
	{
		tree.change_(i, p);
	}
};

/*
class ReplayMemory
{
private:
	std::deque<transition> memory;
	int memory_max_size;
public:
	using samples = std::tuple < std::vector<torch::Tensor>, std::vector<int>, std::vector<float>, std::vector<torch::Tensor>, std::vector<int>>;
	ReplayMemory(int mem_size) :memory_max_size(pow(2,mem_size)) {};
	size_t size() //记忆重放池大小
	{
		return memory.size();
	}
	samples sample(int batch_size)//随机抽取batch_size个样本
	{
		std::vector<transition> batch_data;
		std::vector<torch::Tensor> state, next_state;
	    std:: vector< int> action;
		std::vector<int> done;
		std::vector<float> reward;
		batch_data.reserve(batch_size);
		state.reserve(batch_size);
		next_state.reserve(batch_size);
		action.reserve(batch_size);
		done.reserve(batch_size);
		reward.reserve(batch_size);
		//multi_step dqn:
		for (int i = 0; i < batch_size; ++i)
		{
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_int_distribution<> dist(0, (memory.size() - 1 - N_step_replay));
			int begin = dist(gen);
			float sum_reward = 0;
			torch::Tensor state_ahead;
			bool done_ahead;
			for (int j = 0; j < N_step_replay; ++j)
			{
				sum_reward += std::pow(gamma, j)* std::get<2>(memory[begin+j]);
				if (std::get<4>(memory[begin + j]))
				{
					state_ahead = std::get<3>(memory[begin + j]);
					done_ahead = true;
					break;
				}
				else
				{
					state_ahead = std::get<3>(memory[begin + j]);
					done_ahead = false;
				}
			}
			batch_data.push_back( { std::get<0>(memory[begin]),std::get<1>(memory[begin]),sum_reward,state_ahead,done_ahead} );
		}
		//std::sample(memory.begin(), memory.end(), std::back_inserter(batch_data), batch_size, std::mt19937{ std::random_device{}() });//从memory里随机采样(传统DQN)
		for (auto &p : batch_data)
		{
			state.push_back(std::get<0>(p));
			action.push_back(std::get<1>(p));
			reward.push_back(std::get<2>(p));
			next_state.push_back(std::get<3>(p));
			done.push_back(std::get<4>(p));
		}
		//std::cout << "sample done" << std::endl;//
		return  { state,action,reward,next_state,done};
	}
	void push(torch::Tensor state, int action, float reward, torch::Tensor next_state, bool done)//存入记忆池
	{
		if (memory.size() == memory_max_size)
		{
			memory.pop_front();
		}
		memory.push_back({ state.clone(), action, reward, next_state.clone(), done});
	}
};
*/