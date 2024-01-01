#pragma once
#include <QtWidgets/QWidget>
#include "ui_MainScene.h"
#include "Block.h"
#include "Charactors.h"
#include <QTimer>
#include <QKeyEvent>
#include<QList>
#include "Medicine.h"

class Count :public QObject
{
    Q_OBJECT

    unsigned int count = 0;
public:
    Charactors** p;
    void step() 
    {
        if (count % 1000 == 0)
        {
            emit timeupdate();
        }
        if (count % ENEMY_SPOWN_INTERVAL==0)
        {
            emit enemyspown();
        }
        if (*p!=nullptr&&(*p)->w!=nullptr&&count%(*p)->w->interval == 0)
        {
            emit weapon_change_state();
        }
        if (count %5000 == 0)
        {
            emit shoot();
        }
        count += GAME_RATE;
    }
signals:
    void enemyspown();
    void timeupdate();
    void weapon_change_state();
    void shoot();
};
class MainScene : public QWidget
{
    Q_OBJECT
public:
    friend class Count;
    friend class Game_env;
    Count step_count;
    MainScene(QWidget* parent);
    ~MainScene();
    void  initScene();
    void StartGame(int id,int seed=std::time(0));
    void paintEvent(QPaintEvent *event);
    void keyPressEvent(QKeyEvent* key);
    void keyReleaseEvent(QKeyEvent* key);
    void pause();
    void game_continue();
    void dead_game_over();
    void show_start_menu();
    void hide_start_menu();
    void show_upgrate_menu();
    void hide_upgrade_menu();
    void win_game_over();
    void game_main();
    void enemy_spown();
    void update_time();
    void lv_up();
private:
    Ui::MainSceneClass ui;
    int seed; 
    Block block;
    QPixmap map_img;
    Charactors* player=nullptr;
    QTimer timer;
    //QTimer total_timer;
    //QTimer enemy_spown_timer;
    int time=GAME_TIME;//倒计时
    QList<enemy_1*> enemies_1;
    QList<enemy_2*> enemies_2;
    QList<bullet*> bullets;//子弹类
    QList <Medicine*> medicines;//回复药类（后期可扩展为掉落物类）
    int all_hp=0;
    int all_level=0;
    int all_speed=0;
    int kill_count=0;
    int level_point=0;
    int level_point_time=0;
    QString s;
};
