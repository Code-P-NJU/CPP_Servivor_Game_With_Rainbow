#pragma once
#include "agent.h"
#include "ReplayMemory.h"
#include "game_env.h"
#include "windows.h"
extern c10::Device device;
extern double gamma,lr, max_lr, min_lr;
extern int batch_size;
extern float steplr_gamma;
const double Pi = 3.14159265358979323846;
class DQN 
{
//public:
	Game_env env;
	agent agent;
	int T=5;
	int T_mul = 2;
	int T_cur = 0;
public:
	DQN() : agent(env.obs_dim, env.act_dim)
	{
	}
	void save_net(const std::string path)
	{
		agent.save(path);
	}
	void load_net(const  std::string path)
	{
		agent.load(path);
	}
	void env_seed(int seed)
	{
		env.seed(seed);
	}
	void lr_step() 
	{		
		lr = min_lr + 0.5 * (max_lr - min_lr) * (1.0 + cos(Pi*T_cur/T));
		T_cur += steplr_step_size;
		if (T_cur > T) 
		{
			T_cur = 0;
			T *= T_mul;
		}
		//lr *= steplr_gamma;
		if (lr < min_lr) 
		{
			lr = min_lr;
		}
		for (auto &p : agent.optimizer.param_groups())
		{
			p.options().set_lr(lr);
		}
	}
	void human_control() 
	{
		agent.train=1;
		float reward_total = 0;
		env.seed(114514);
		auto state = torch::from_blob(env.reset().data(), { 3,env.h,env.w }).to(device);
		//env.render();
		while (1)
		{
			int action;
			std::cin >> action;
			action %= 5;
			auto result = env.step(action);
			auto next_state = torch::from_blob(std::get<0>(result).data(), { 3,env.h,env.w }).to(device);
			auto reward = std::get<1>(result);
			auto done = std::get<2>(result);
			//std::cout << next_state<<reward<<done << std::endl;//
			agent.memory.push(state.to(torch::kUInt8), action, reward, next_state.to(torch::kUInt8), done);
			if (agent.memory.size() > batch_size)
			{
				//std::cout << replaymemory.size() << std::endl;//
				agent.update(agent.memory.sample(batch_size));
			}
			state = next_state;
			//std::cout << state << std::endl;//
			if (done)
			{
				break;
			}	
		}
	}
	float run_episode()
	{
		agent.train = 1;
		float reward_total = 0;
		//env.seed(114514);
		auto state = torch::from_blob(env.reset().data(), { 3,env.h,env.w }).to(device);
		//env.render();
		while (1) 
		{
			auto action=agent.take_action(state);
			auto result = env.step(action);
			auto next_state = torch::from_blob(std::get<0>(result).data(), { 3,env.h,env.w }).to(device);
			auto reward = std::get<1>(result);
			auto done = std::get<2>(result);
			//std::cout << next_state<<reward<<done << std::endl;//
			agent.memory.push(state.to(torch::kUInt8), action, reward, next_state.to(torch::kUInt8), done);
			//std::cout << "line41" << std::endl;//
			reward_total += reward;
			//std::cout << agent.memory.size() << std::endl;
			if (agent.memory.size() > batch_size+N_step_replay-1)
			{
				agent.update(agent.memory.sample(batch_size));
			}
			state = next_state;
			//std::cout << state << std::endl;//
			if (done)
			{
				break;
			}
		}
		return reward_total;
	}
	auto episode_evaluate(int render) 
	{
		agent.train=0;
		std::vector<float> reward_list;
		if (render)
				env.render();
		for (int i = 0; i < 5; ++i)
		{
			env.seed(1919810+i);
			auto state = torch::from_blob(env.reset().data(), { 3,env.h,env.w }).to(device);
			float reward_episode = 0;
			while (1)
			{
				auto action = agent.take_action(state);
				auto result = env.step(action);
				auto next_state = torch::from_blob(std::get<0>(result).data(), { 3,env.h,env.w }).to(device);
				auto reward = std::get<1>(result);
				auto done = std::get<2>(result);
				reward_episode += reward;
				state = next_state;
				if (done)
					break;
			}
			reward_list.push_back(reward_episode);
		}
		float total_reward=0;
		for (auto i : reward_list)
		{
			total_reward += i;
		}
		total_reward /= reward_list.size();
		return total_reward;
	}
	auto episode_evaluate_on_train(int render)
	{
		agent.train = 0;
		std::vector<float> reward_list;
		if (render)
			env.render();
		for (int i = 0; i < 5; ++i)
		{		
			env.seed(114514+i);
			auto state = torch::from_blob(env.reset().data(), { 3,env.h,env.w }).to(device);
			float reward_episode = 0;
			while (1)
			{
				auto action = agent.take_action(state);
				auto result = env.step(action);
				auto next_state = torch::from_blob(std::get<0>(result).data(), { 3,env.h,env.w }).to(device);
				auto reward = std::get<1>(result);
				auto done = std::get<2>(result);
				reward_episode += reward;
				state = next_state;
				/*
				QImage out(env.w, env.h, QImage::Format_RGB32);
				QImage out1(env.w, env.h, QImage::Format_RGB32);
				QImage out2(env.w, env.h, QImage::Format_RGB32);
				for (int i = 0; i < env.w; ++i)
				{
					for (int j = 0; j < env.h; ++j)
					{
						int gray = state[0][j][i].item().toInt();
						out.setPixelColor(i, j, QColor(gray, gray, gray));
						gray = state[1][j][i].item().toInt();
						out1.setPixelColor(i, j, QColor(gray, gray, gray));
						gray = state[2][j][i].item().toInt();
						out2.setPixelColor(i, j, QColor(gray, gray, gray));
					}
				}
				out.save("C://Users//24966//Desktop//test.bmp","BMP",100);
				out1.save("C://Users//24966//Desktop//test1.bmp", "BMP", 100);
				out2.save("C://Users//24966//Desktop//test2.bmp", "BMP", 100);
				*/
				if (done)
					break;
			}
			reward_list.push_back(reward_episode);
		}
		float total_reward = 0;
		for (auto i : reward_list)
		{
			total_reward += i / reward_list.size();
		}
		return total_reward;
	}
	void test(int delay_time)
	{
		agent.train = 0;
		std::vector<float> reward_list;
		env.seed(time(NULL));
		auto state = torch::from_blob(env.reset().data(), { 3,env.h,env.w }).to(device);
		env.render();
		float reward_episode = 0;
		while (1)
		{
			auto action = agent.take_action(state);
			auto result = env.step(action);
			auto next_state = torch::from_blob(std::get<0>(result).data(), { 3,env.h,env.w }).to(device);
			auto reward = std::get<1>(result);
			auto done = std::get<2>(result);
			reward_episode += reward;
			state = next_state;
			if (done)
				break;
			Sleep(delay_time);
		}
		std::cout << "test_total_reward: " << reward_episode << std::endl;
	}
};