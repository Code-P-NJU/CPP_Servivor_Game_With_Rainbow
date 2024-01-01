#pragma once
#include <QPixmap>
#include"config.h"
class bullet
{
public:
	QPixmap img;
	int position_x;
	int position_y;
	int v_x;
	int v_y;
	int size = 30;
	int speed = 50;//rate
	int atk=10;
	bullet(int x, int y, int v_x, int v_y, int face,int lv): v_x(v_x), v_y(v_y),atk(50+lv*50),size(30+lv*10)
	{
		img.load(BULLET_IMG);
		if (face == 0)
		{
			position_x = x - size;
			position_y = y - size / 2;
			bullet::v_x -= speed;
		}
		else if (face == 1)
		{
			position_x = x - size/2;
			position_y = y - size ;
			bullet::v_y -= speed;
		}
		else if (face == 2)
		{
			position_x = x;
			position_y = y - size / 2;
			bullet::v_x += speed;
		}
		else if (face == 3)
		{
			position_x = x - size/2;
			position_y = y - size;
			bullet::v_y += speed;
		}
	}
	void move()
	{
		position_x += v_x;
		position_y += v_y;
	}
};