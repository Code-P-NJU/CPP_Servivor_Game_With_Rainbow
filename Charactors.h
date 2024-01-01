#pragma once
#include<Qwidget>
#include "Weapon.h"
#include <QKeyEvent>
#include "Block.h"
#include <cmath>
#include <QList>
#include "bullet.h"
#include <iostream>
class Charactors : public QWidget
{
protected:
	int v_x = 0;
	int v_y = 0;
	int size = CHARACTOR_SIZE;
	int level = 0;
	int speed;
	int position_x = GAME_WIDTH / 2;
	int position_y = GAME_HEIGHT / 2;
	int id;
public:
	friend class Game_env;
	QPixmap img;
	int full_hp;
	int hp;	//血量在外界需要获得、更新，故此处决定对外暴露
	Weapon* w = nullptr;
	void paint(QPainter& p) const;
	int get_size();
	int get_id() const;
	void weapon_pause();
	void weapon_continue();
	QRect get_rect() const;
	int get_level() const;
	int get_x() const;
	int get_y() const;
	virtual void HP_up();
	virtual void Speed_up();
	virtual void level_up();
	bool is_attacked(const QRect& r); //受击过程（通过碰撞检测实现）
	void keyPressEvent(QKeyEvent* key);
	void keyReleaseEvent(QKeyEvent* key);
	virtual void move(Block& block);
	Charactors();
	Charactors(int id, int hp, int level, int speed);
	~Charactors();
};
class enemy_1 :public Charactors
{
public:
	enemy_1();
	enemy_1(int id, int x, int y, int hp, int level, int speed);
	void update_v(int x, int y);
	void move(Block& block);
};
class enemy_2 :public Charactors
{
	QList<bullet*>& bullets;
public:
	//QTimer shot_timer;
	enemy_2(int id, int x, int y, int hp, int level, int speed, QList<bullet*>& bullets);
	void move();
	void shot();
};