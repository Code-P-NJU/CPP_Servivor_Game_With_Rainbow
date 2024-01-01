#pragma once
#include <tuple>
#include <vector>	
#include "MainScene.h"
#include "windows.h"
#include "QTimer"
#include <QtWidgets/QApplication>
using result_ = std::tuple<std::vector<float>, float, bool>;
class env
{
public:
	int obs_dim, act_dim;
	virtual result_ step(int action) = 0;//输入action，输出采取action后的结果{次态，奖励，是否完成}
	virtual std::vector<float> reset() = 0;//重置环境
	virtual void close() = 0;//关闭环境
	virtual void seed(int seed) = 0;//设置种子
	virtual void render() = 0;//显示界面
};
class Game_env :public env
{
	int timer=0;
	int seed_=114514;
	MainScene m;
	std::deque<QImage> frame_buffer;
public:	
	int w=192, h=108;
	Game_env() :m(nullptr)
	{
		obs_dim = w*h;
		act_dim = 5;
	};
	void close() {};
	void seed(int seed) 
	{
		this->seed_ = seed;
	};
	std::vector<float> reset() 
	{
		frame_buffer.clear();
		m.StartGame(1, seed_);
		m.pause();
		timer = 0;
		std::vector<float> state(3*w*h);
		QImage img = m.grab().toImage();
		img = img.scaled(w, h, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		frame_buffer.push_back(img);
		frame_buffer.push_back(img);
		frame_buffer.push_back(img);
		for (int i = 0; i < w; i++)
		{
			for (int j = 0; j < h; j++)
			{
				state[j*w + i] = qGray(frame_buffer[0].pixel(i, j));
				state[j*w + i  + w * h] = qGray(frame_buffer[1].pixel(i, j));
				state[j*w + i + 2 * w * h] = qGray(frame_buffer[2].pixel(i, j));
			}
		}
		return state;
	};
	result_ step(int action)
	{
		timer++;
		int pre_hp = m.player->hp;
		int pre_kill = m.kill_count;
		std::vector<float> state(obs_dim * 3);
		for (int i = 0; i < 4; ++i) 
		{
			m.player->v_x = 0;
			m.player->v_y = 0;
			switch (action)
			{
			case 0:
				m.player->v_x = -1 * m.player->speed;
				break;
			case 1:
				m.player->v_y = -1 * m.player->speed;
				break;
			case 2:
				m.player->v_x = m.player->speed;
				break;
			case 3:
				m.player->v_y = m.player->speed;
				break;
			}
			m.game_main();
			qApp->processEvents();
			QImage img = m.grab().toImage();
			img = img.scaled(w, h);
			//img.save("C://Users//24966//Desktop//1.png");
			//QImage out("C://Users//24966//Desktop//1.png")
			frame_buffer.push_back(std::move(img));
			frame_buffer.pop_front();
			//out.save("C://Users//24966//Desktop//1.png");
			if (m.player->hp <= 0 || m.time <= 0)
			{
				break;
			}
		}
		for (int i = 0; i < w; i++)
		{
			for (int j = 0; j < h; j++)
			{
				state[j*w + i] = qGray(frame_buffer[0].pixel(i, j));
				state[j * w + i + w * h] = qGray(frame_buffer[1].pixel(i, j));
				state[j * w + i + 2 * w * h] = qGray(frame_buffer[2].pixel(i, j));
			}
		}
		bool done = m.player->hp <= 0||m.time<=0;
		float reward=(pre_hp>m.player->hp? -0.2:0.2)+(pre_hp<m.player->hp? 2:0) +1*  (m.kill_count - pre_kill)+(m.time<=0? 30:0);
		//std::cout << reward<<' ';
		return{ state,reward,done };
	}
	void render()
	{
		m.show();
	}
};
