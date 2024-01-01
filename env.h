#pragma once
#include <iostream>
#include <tuple>
#include <vector>	
#include "windows.h"
using result_ = std::tuple<std::vector<float>,float,bool>;
class env 
{
public:
	int obs_dim,act_dim;
	virtual result_ step(int action)=0;//输入action，输出采取action后的结果{次态，奖励，是否完成}
	virtual std::vector<float> reset()=0;//重置环境
	virtual void close() = 0;//关闭环境
	virtual void seed(int seed) = 0;//设置种子
	virtual void render()=0;//显示界面
};
class My_env:public env 
{
	int random_range = 21;
	bool rend = 0;
	float x_1=0, x_t=-5, y_1=0, y_t=4;
	float count_ =100 ;
	float count =1;
	int dis_ =abs (x_1 - x_t) + abs(y_1 - y_t);
	int dis = dis_;
	int pre_dis=dis_ ;
public:
	My_env() 
	{
		obs_dim = 5;
		act_dim = 4;
	}
	result_ step(int action) 
	{
		pre_dis = dis;
		switch (action)
		{
			case 0: 
				x_1++;
				break;
			case 1:
				y_1++;
				break;
			case 2:
				x_1--;
				break;
			case 3:
				y_1--;
				break;
		}
		dis= abs(x_1 - x_t) + abs(y_1 - y_t);
		std::vector<float> state{(float)x_1/random_range,(float)y_1/random_range,(float)x_t/random_range,(float)y_t/random_range,(float)count/count_};
		float reward = 2*(dis < pre_dis ? 1.0 : -1.0)/(dis_+1) + (dis < 1 ? 10 : 0) - (1.0 / (count_+1));
		bool done = (count == count_) || (dis < 1);
		if (rend)
		{
			std::cout << '(' << x_1 << ',' << y_1 << ',' << count << ')' << std::endl;
		}
		++count;
		return { state,reward,done };
	};
	std::vector<float> reset( ) 
	{
		x_1 = 0;
		y_1 = 0;
		rend = 0;
		dis = abs(x_1 - x_t) + abs(y_1 - y_t);
		count = 0;
		std::vector<float> state{ x_1,y_1,x_t,y_t,count};
		return state;
	};
	std::vector<float> reset(bool is_random)
	{
		if (is_random) 
		{
			x_t = rand() % random_range - random_range / 2;
			y_t = rand() % random_range - random_range / 2;
			dis_ = abs(x_1 - x_t) + abs(y_1 - y_t);
		}
		x_1 = 0;
		y_1 = 0;
		rend = 0;
		dis = dis_;
		count = 0;
		std::vector<float> state{ x_1,y_1,x_t,y_t ,count};
		return state;
	};
	void seed(int seed) 
	{
		srand(seed);
	}
	void close() 
	{

	};
	void render() 
	{
		std::cout << "target_point:" << x_t << ',' << y_t << std::endl;
		rend = 1;
	};
};
