#pragma once
#include<QWidget>
#include<QTimer>
#include "config.h"
#include <QPainter>
class Count;
class Weapon :public QWidget
{
	friend class Count;
public:
	int id = 1;
	int atk = 5;//每次判定的伤害 rate
	QPixmap img;
	int range;
	int center_x;
	int center_y;
	int face=1;//朝向，0123分别为左上右下
	int interval = 1000;
	//QTimer timer;
	QRect rect;
	bool state=0;
	bool state_last=0;
	//Weapon(Weapon& w): img(w.img),range(w.range),center_x(center_x),center_y(w.center_y),face(w.face),interval(w.interval),rect(w.rect),timer(w.timer)
	//{}
	virtual int attack(const QRect& r)
	{
		if (rect.intersects(r))
		{
			return  atk;
		}
		else 
			return 0;
	}
	Weapon(int x, int y, int size, int level) :range(size), center_x(x), center_y(y), rect(x - range, y - range - 25, 2 * range, range / 2)
	{
		img.load(KNIFE_2_IMG);
		interval *= pow(0.9, level);
		interval = (interval / GAME_RATE) * GAME_RATE + GAME_RATE;
		//timer.setInterval(interval);
		range += 5 * level;
		//connect(&timer, &QTimer::timeout, this, &Weapon::change_state);
	}
	virtual void change_state() 
	{
		++face;
		if (face == 4)
		{
			face = 0;
		}
	}
	virtual void paint(QPainter& p)
	{
		p.drawPixmap(rect, img);
	}
	virtual void change_facing() 
	{
		if (face == 0)
		{
			rect = QRect(center_x - range-25, center_y - range , range/2,2 * range);
			img.load(KNIFE_2__0IMG);
		}
		else if (face == 1)
		{
			rect = QRect(center_x - range, center_y - range - 25, 2 * range, range/2);
			img.load(KNIFE_2_IMG);
		}
		else if (face == 2) 
		{
			rect = QRect(center_x + range , center_y - range,range/2, 2 * range);
			img.load(KNIFE_2__2IMG);
		}
		else if (face == 3)
		{
			rect = QRect(center_x - range, center_y + range , 2 * range, range/2);
			img.load(KNIFE_2__3IMG);
		}
	}
};
class Weapon_2 :public Weapon
{
public:
	Weapon_2(int x, int y, int size,int level) :Weapon(x, y, size,level)
	{
		id = 2;
		img.load(KNIFE_1_IMG);
		//timer.disconnect();
		atk = 10;//默认伤害10
		range = 100;//默认范围
		range += 10 * level;
		interval = 750*pow(0.8,level);
		interval = (interval / GAME_RATE) * GAME_RATE + GAME_RATE;
		//timer.setInterval(interval);
		//connect(&timer, &QTimer::timeout, this,&Weapon_2::change_state);
	}
	void change_state() 
	{
		state = !state;
	}
	void change_facing() 
	{
		if (face == 0)
		{
			if (state)
			{
				rect = QRect(center_x - range, center_y - range, range, range);
				img.load(KNIFE_1_IMG);
			}
			else 
			{
				rect = QRect(center_x - range, center_y , range, range);
				img.load(KNIFE_1__0IMG);
			}
		}
		else if (face == 1)
		{
			if (!state)
			{
				rect = QRect(center_x - range, center_y - range, range, range);
				img.load(KNIFE_1_IMG);
			}
			else
			{
				rect = QRect(center_x , center_y-range, range, range);
				img.load(KNIFE_1__2IMG);
			}
		}
		else if (face == 2)
		{
			if (!state)
			{
				rect = QRect(center_x , center_y - range, range, range);
				img.load(KNIFE_1__2IMG);
			}
			else
			{
				rect = QRect(center_x , center_y, range, range);
				img.load(KNIFE_1__3IMG);
			}
		}
		else if (face == 3)
		{
			if (state)
			{
				rect = QRect(center_x - range, center_y, range, range);
				img.load(KNIFE_1__0IMG);
			}
			else
			{
				rect = QRect(center_x , center_y, range, range);
				img.load(KNIFE_1__3IMG);
			}
		}
	}
	virtual int attack(const QRect& r) 
	{
		if (pow(r.center().x() - center_x, 2) + pow(r.center().y() - center_y, 2) < 2*pow(range+ r.width() / 2, 2))
		{
			int d_x = r.center().x() - center_x;
			int d_y = r.center().y() - center_y;
			if (((face == 0&&d_x<0)||(face==2&&d_x>0))&&abs(d_x)>=abs(d_y))
			{
				return atk;
			}
			else if (((face == 1 && d_y < 0) || (face == 3 && d_y > 0)) && abs(d_x) <= abs(d_y))
			{
				return atk;
			}
			return 0;
		}
		return 0;
	}
};