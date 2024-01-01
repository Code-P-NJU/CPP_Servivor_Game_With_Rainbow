#pragma once
#include "MainScene.h"
#include"config.h"
#include <QIcon>
#include <ctime>
#include <QPushButton>
#include <iostream>
#include <fstream>
MainScene::~MainScene()
{}
void MainScene::initScene()
{
    setFixedSize(GAME_WIDTH, GAME_HEIGHT);//初始化窗口大小
    setWindowTitle(GAME_TITLE); //设置窗口标题
    setWindowIcon(QIcon(GAME_ICON));//设置图标
    timer.setInterval(GAME_RATE);//设置游戏速率
    //enemy_spown_timer.setInterval(ENEMY_SPOWN_INTERVAL);//设置生成怪物判定间隔
    map_img.load(MAP_BACKGROUND);//设置游戏背景图
    ui.kill_count->setFont(QFont("MS Serif",30,75,1));
    ui.level_point->setFont(QFont("MS Serif", 15, 75, 1));
    ui.Timer_Display->setStyleSheet("QLCDNumber{color: white}");//设置倒计时颜色
    //total_timer.setInterval(1000);//倒计时间隔，1s一减
}
void MainScene::dead_game_over() 
{
    pause();
    /*
    show_upgrate_menu();
    ui.GAME_OVER->show();
    ui.Button_Exit->show();
    ui.restart->show();
    ui.level_point->show();
    ui.level_point->setText(QString("Level Point : %1").arg(level_point));
    ui.level_point->setAlignment(Qt::AlignCenter);
    ui.kill_count->hide();
    ui.Timer_Display->hide();
    ui.HP_up->disconnect();
    ui.Lv_up->disconnect();
    ui.Speed_up->disconnect();
    connect(ui.HP_up, &QPushButton::clicked, this, [&]() {
        if (level_point > 0)
        {
            ++all_hp;
            --level_point;
            ui.level_point->setText(QString("Level Point : %1").arg(level_point));
            ui.level_point->setAlignment(Qt::AlignCenter);
        }
        else
        {
            ui.level_point->setText(QString("点数不足！"));
            ui.level_point->setAlignment(Qt::AlignCenter);
        }
        });
    connect(ui.Speed_up, &QPushButton::clicked, this, [&]() {
        if (level_point > 0)
        {
            ++all_speed;
            --level_point;
            ui.level_point->setText(QString("Level Point : %1").arg(level_point));
            ui.level_point->setAlignment(Qt::AlignCenter);
        }
        else
        {
            ui.level_point->setText(QString("点数不足！"));
            ui.level_point->setAlignment(Qt::AlignCenter);
        }
        });
    connect(ui.Lv_up, &QPushButton::clicked, this, [&]() {
        if (level_point > 0)
        {
            ++all_level;
            --level_point;
            ui.level_point->setText(QString("Level Point : %1").arg(level_point));
            ui.level_point->setAlignment(Qt::AlignCenter);
        }
        else
        {
            ui.level_point->setText(QString("点数不足！"));
            ui.level_point->setAlignment(Qt::AlignCenter);
        }
        });
        */
}
void MainScene::show_upgrate_menu() 
{
    ui.HP_up->show();
    ui.Lv_up->show();
    ui.Speed_up->show();
}
void MainScene::hide_upgrade_menu() 
{
    ui.HP_up->hide();
    ui.Lv_up->hide();
    ui.Speed_up->hide();
}
void MainScene::pause() 
{
    timer.stop();
    //total_timer.stop();
    player->weapon_pause();
    //enemy_spown_timer.stop();
}
void MainScene::game_continue() 
{
    timer.start();
    //total_timer.start();
    player->weapon_continue();
    //enemy_spown_timer.start();
}
void MainScene::hide_start_menu() 
{
    ui.Button_Exit->hide();
    ui.Button_Load->hide();
    ui.Button_Save->hide();
    ui.Button_Start->hide();
    ui.Button_Start_2->hide();
    ui.Background->hide();
    ui.title->hide();
    ui.level_point->hide();
}
void MainScene::show_start_menu() 
{    ui.Timer_Display->hide();
    ui.HP_up->hide();
    ui.Lv_up->hide();
    ui.restart->hide();
    ui.Speed_up->hide();
    ui.kill_count->hide();
    ui.Win->hide();
    ui.level_point->show();
    ui.level_point->setText(QString("HP:%1 Lv:%2 Speed:%3").arg(all_hp).arg(all_level).arg(all_speed));
    ui.level_point->setAlignment(Qt::AlignCenter);
    ui.GAME_OVER->hide();
    ui.Button_Exit->show();
    ui.Button_Load->show();
    ui.Button_Save->show();
    ui.Button_Start->show();
    ui.Button_Start_2->show();
    ui.title->show();
    ui.Background->setPixmap(QPixmap(START_BACKGROUND));
    ui.Background->show();
}
void MainScene::win_game_over() 
{
    pause();
    /*
    ui.Background->setPixmap(QPixmap(WIN_IMG));
    ui.Background->show();
    show_upgrate_menu();
    level_point += 2;//胜利额外增加两个点数
    ui.level_point->show();
    ui.level_point->setText(QString("Level Point : %1").arg(level_point));
    ui.level_point->setAlignment(Qt::AlignCenter);
    ui.kill_count->hide();
    ui.Win->show();
    ui.restart->show();
    ui.Timer_Display->hide();
    ui.Button_Exit->show();
    ui.HP_up->disconnect();
    ui.Lv_up->disconnect();
    ui.Speed_up->disconnect();
    connect(ui.HP_up, &QPushButton::clicked, this, [&]() {
        if (level_point > 0)
        {
            ++all_hp;
            --level_point;
            ui.level_point->setText(QString("Level Point : %1").arg(level_point));
            ui.level_point->setAlignment(Qt::AlignCenter);
        }
        else
        {
            ui.level_point->setText(QString("点数不足！"));
            ui.level_point->setAlignment(Qt::AlignCenter);
        }
        });
    connect(ui.Speed_up, &QPushButton::clicked, this, [&]() {
        if (level_point > 0)
        {
            ++all_speed;
            --level_point;
            ui.level_point->setText(QString("Level Point : %1").arg(level_point));
            ui.level_point->setAlignment(Qt::AlignCenter);
        }
        else
        {
            ui.level_point->setText(QString("点数不足！"));
            ui.level_point->setAlignment(Qt::AlignCenter);
        }
        });
    connect(ui.Lv_up, &QPushButton::clicked, this, [&]() {
        if (level_point > 0)
        {
            ++all_level;
            --level_point;
            ui.level_point->setText(QString("Level Point : %1").arg(level_point));
            ui.level_point->setAlignment(Qt::AlignCenter);
        }
        else
        {
            ui.level_point->setText(QString("点数不足！"));
            ui.level_point->setAlignment(Qt::AlignCenter);
        }
        });
        */
}
void MainScene::game_main() 
{
    step_count.step();
    player->move(block);//移动
    if (player->w != nullptr)//攻击检测
    {    
        player->w->change_facing();
        if (player->w->id == 1 || player->w->id == 2 && (player->w->state != player->w->state_last))
        {
            for (int i = 0; i < enemies_1.size(); ++i)
            {
                enemies_1[i]->hp -= player->w->attack(enemies_1[i]->get_rect());
                if (enemies_1[i]->hp <= 0)
                {
                    ++kill_count;
                    ui.kill_count->setText(QString("Kill Count : %1").arg(kill_count));
                    ui.kill_count->setAlignment(Qt::AlignCenter);
                    if (rand() % 10 == 0)
                    {
                        Medicine* p = new Medicine(enemies_1[i]->get_rect().center());
                        medicines.append(p);
                    }
                    delete enemies_1[i];
                    enemies_1.removeAt(i);
                }
            }
            for (int i = 0; i < enemies_2.size(); ++i)
            {
                enemies_2[i]->hp -= player->w->attack(enemies_2[i]->get_rect());
                if (enemies_2[i]->hp <= 0)
                {
                    ++kill_count;
                    ui.kill_count->setText(QString("Kill Count : %1").arg(kill_count));
                    ui.kill_count->setAlignment(Qt::AlignCenter);
                    if (rand() % 10 == 0)
                    {
                        Medicine* p = new Medicine(enemies_2[i]->get_rect().center());
                        medicines.append(p);
                    }
                    delete enemies_2[i];
                    enemies_2.removeAt(i);
                }
            }
        }
        if (player->w->id == 2)
        {
            player->w->state_last = player->w->state;
        }
    }
    for (int i = 0; i < enemies_1.size(); ++i)//enemy_1的行为
    {
        if (player->is_attacked(enemies_1[i]->get_rect()))
        {
            player->hp -= 5 + enemies_1[i]->get_level();//默认伤害为1*(rate)
            if (player->hp <= 0)
            {
                dead_game_over();
            }
        };
        enemies_1[i]->update_v(player->get_x(), player->get_y());
        enemies_1[i]->move(block);
    }
    for (int i = 0; i < enemies_2.size(); ++i)//enemy_2的行为
    {
        enemies_2[i]->move();
        if (player->is_attacked(enemies_2[i]->get_rect()))
        {
            enemies_2[i]->hp = 0;
            player->hp -= 5 + enemies_2[i]->get_level();//默认伤害为5
            if (player->hp <= 0)
            {
                dead_game_over();
            }
        };
    }
    for (int i = 0; i < bullets.size(); ++i)//bullet的行为
    {
        bullets[i]->move();
        for (int j = 0; j < block.n; ++j)
        {
            if (block.b[j].intersects(QRect(bullets[i]->position_x, bullets[i]->position_y, bullets[i]->size, bullets[i]->size)) || bullets[i]->position_y <= 0 || bullets[i]->position_y >= GAME_HEIGHT || bullets[i]->position_x <= 0 || bullets[i]->position_x >= GAME_WIDTH)
            {
                delete bullets[i];
                bullets.remove(i);
                break;
            }
        }
        if (i == bullets.size())
        {
            break;
        }
        if (player->is_attacked(QRect(bullets[i]->position_x, bullets[i]->position_y, bullets[i]->size, bullets[i]->size)))
        {
            delete bullets[i];
            bullets.remove(i);
            player->hp -= bullets[i]->atk;
            if (player->hp <= 0)
            {
                dead_game_over();
            }
        }
    }
    for (int i = 0; i < medicines.size(); ++i)//回血药行为
    {
        if (player->is_attacked(medicines[i]->rect) && player->hp < player->full_hp)
        {
            player->hp += medicines[i]->healing;
            if (player->hp > player->full_hp)
            {
                player->hp = player->full_hp;
            }
            delete medicines[i];
            medicines.remove(i);
        }
    }
    update();
}
void MainScene::enemy_spown() 
{
    int x, y, enemy_level = (GAME_TIME - time) / ENEMY_UP;
    if (enemy_level <= 4 && time >= 0)
    {
        x = rand() % (8 - enemy_level) + 5 * DEBUG_NO_ENEMY_1;//生成enemy_1的概率默认为4/8，x=0123为左上右下生成 
        y = rand() % (80 - 5 * enemy_level) + 5 * DEBUG_NO_ENEMY_2;//生成enemy_2的概率默认为4/80，x=0123为左上右下生成 
    }
    else
    {
        x = rand() % 4 + 5 * DEBUG_NO_ENEMY_1;
        y = rand() % 60 + 5 * DEBUG_NO_ENEMY_2;
    }
    if (x == 0)
    {
        enemy_1* p = new enemy_1(1, 10, rand() % (GAME_HEIGHT - CHARACTOR_SIZE-20)+10, 10, enemy_level, ENEMY_SPEED + enemy_level / 2);
        enemies_1.append(p);//后期更改难度时要改参数
    }
    else if (x == 1)
    {
        enemy_1* p = new enemy_1(1, rand() % (GAME_WIDTH - CHARACTOR_SIZE-20)+10, 10, 10, enemy_level, ENEMY_SPEED + enemy_level / 2);
        enemies_1.append(p);//后期更改难度时要改参数
    }
    else if (x == 2)
    {
        enemy_1* p = new enemy_1(1, GAME_WIDTH - CHARACTOR_SIZE-10, rand() % (GAME_HEIGHT - CHARACTOR_SIZE-20)+10, 10, enemy_level, ENEMY_SPEED + enemy_level / 2);
        enemies_1.append(p);//后期更改难度时要改参数
    }
    else if (x == 3)
    {
        enemy_1* p = new enemy_1(1, rand() % (GAME_WIDTH - CHARACTOR_SIZE-20)+10, GAME_HEIGHT - CHARACTOR_SIZE-10, 10, enemy_level, ENEMY_SPEED + enemy_level / 2);
        enemies_1.append(p);//后期更改难度时要改参数
    }
    if (y == 0)
    {
        enemy_2* p = new enemy_2(2, 5, rand() % GAME_HEIGHT, 10, enemy_level, ENEMY_SPEED + enemy_level / 2, bullets);
        connect(&step_count, &Count::shoot, p, &enemy_2::shot);
        enemies_2.append(p);//后期更改难度时要改参数
    }
    else if (y == 1)
    {
        enemy_2* p = new enemy_2(2, rand() % GAME_WIDTH, 5, 10, enemy_level, ENEMY_SPEED + enemy_level / 2, bullets);
        connect(&step_count, &Count::shoot, p, &enemy_2::shot);
        enemies_2.append(p);//后期更改难度时要改参数
    }
    else if (y == 2)
    {
        enemy_2* p = new enemy_2(2, GAME_WIDTH - 55, rand() % GAME_HEIGHT, 10, enemy_level, ENEMY_SPEED + enemy_level / 2, bullets);
        connect(&step_count, &Count::shoot, p, &enemy_2::shot);
        enemies_2.append(p);//后期更改难度时要改参数
    }
    else if (y == 3)
    {
        enemy_2* p = new enemy_2(2, rand() % GAME_WIDTH, GAME_HEIGHT - 55, 10, enemy_level, ENEMY_SPEED + enemy_level / 2, bullets);
        connect(&step_count, &Count::shoot, p, &enemy_2::shot);
        enemies_2.append(p);//后期更改难度时要改参数
    }
}
MainScene::MainScene(QWidget* parent) : QWidget(parent), seed(std::time(0))
{
    ui.setupUi(this);
    initScene(); //初始化场景
    player = new Charactors;
    step_count.p =& player;
    //connect(&total_timer, &QTimer::timeout, this, &MainScene::update_time);//游戏倒计时
    connect(&timer, &QTimer::timeout, this, &MainScene::game_main);//游戏主计时器
    connect(&step_count, &Count::enemyspown, this, &MainScene::enemy_spown);
    connect(&step_count, &Count::timeupdate, this, &MainScene::update_time);
    connect(&step_count, &Count::weapon_change_state, player->w, &Weapon::change_state);
    //connect(&enemy_spown_timer, &QTimer::timeout, this,&MainScene::enemy_spown);//敌人生成信号计时器
    connect(ui.Button_Exit,&QPushButton::clicked, this, &QWidget::close);
    connect(ui.Button_Start, &QPushButton::clicked, this, [&]() {MainScene::StartGame(1); });
    connect(ui.Button_Start_2, &QPushButton::clicked, this, [&]() {MainScene::StartGame(2); });
    connect(ui.restart, &QPushButton::clicked, this, &MainScene::show_start_menu);
    connect(ui.Button_Save, &QPushButton::clicked, this, [=]() 
        {
            std::ofstream output("./savedata.sv");
            output << all_hp << std::endl << all_level << std::endl << all_speed << std::endl<<level_point;
            output.close();
        });
    connect(ui.Button_Load, &QPushButton::clicked, this, [=]()
        {
            std::ifstream input("./savedata.sv");
            if (input)
            {
                input >> all_hp >> all_level >> all_speed>>level_point;
                input.close();
                ui.level_point->setText(QString("HP:%1 Lv:%2 Speed:%3").arg(all_hp).arg(all_level).arg(all_speed));
                ui.level_point->setAlignment(Qt::AlignCenter);
            }
            else
            {
                ui.level_point->setText("打开存档失败！");
                ui.level_point->setAlignment(Qt::AlignCenter);
            }
        });
    ui.Button_Load->click();
    show_start_menu();
}//游戏主程序
void MainScene::lv_up() 
{
    if (kill_count >= KILL_NUM_PER_LEVEL)
    {
        player->level_up();
        kill_count -= KILL_NUM_PER_LEVEL;
        ui.kill_count->setText(QString("Kill Count : %1").arg(kill_count));
        ui.kill_count->setAlignment(Qt::AlignCenter);
    }
}
void MainScene::StartGame(int id,int seed_) 
{    
    seed = seed_;
    srand(seed);
    ui.HP_up->disconnect();
    ui.Lv_up->disconnect();
    ui.Speed_up->disconnect();
    connect(ui.HP_up, &QPushButton::clicked, this, [&]() {
        if (kill_count >= KILL_NUM_PER_LEVEL)
        {
            player->HP_up();
            kill_count -= KILL_NUM_PER_LEVEL;
            ui.kill_count->setText(QString("Kill Count : %1").arg(kill_count));
            ui.kill_count->setAlignment(Qt::AlignCenter);
        }
        });
    connect(ui.Speed_up, &QPushButton::clicked, this, [&]() {
        if (kill_count >= KILL_NUM_PER_LEVEL)
        {
            player->Speed_up();
            kill_count -= KILL_NUM_PER_LEVEL;
            ui.kill_count->setText(QString("Kill Count : %1").arg(kill_count));
            ui.kill_count->setAlignment(Qt::AlignCenter);
        }
        });
    connect(ui.Lv_up, &QPushButton::clicked, this, &MainScene::lv_up);
    kill_count = 0;
    ui.kill_count->setText(QString("Kill Count : %1").arg(kill_count));
    ui.kill_count->setAlignment(Qt::AlignCenter);
    time = GAME_TIME;
    delete player;
    if (id == 1)
    {
        player = new Charactors(1, all_hp*30 + 50, all_level, 3 + all_speed);
        connect(&step_count, &Count::weapon_change_state, player->w, &Weapon::change_state);
    }
    else if (id == 2)
    {
        player = new Charactors(2, all_hp*50 + 80, all_level, 2 + all_speed);
        connect(&step_count, &Count::weapon_change_state, player->w, &Weapon::change_state);
    }
    for (int i = 0; i < enemies_1.size(); ++i)
    {
        delete enemies_1[i];
    }
    enemies_1.clear();
    for (int i = 0; i < enemies_2.size(); ++i)
    {
        delete enemies_2[i];
    }
    enemies_2.clear();
    for (int i = 0; i < bullets.size(); ++i)
    {
        delete bullets[i];
    }
    bullets.clear();
    for (int i = 0; i < medicines.size(); ++i)
    {
        delete medicines[i];
    }
    medicines.clear();
    block = Block(seed);
    hide_start_menu();
    hide_upgrade_menu();
    ui.restart->hide();
    ui.GAME_OVER->hide();
    ui.Timer_Display->display(QString("%1:%2").arg(time/60,2,10,QLatin1Char('0')).arg(time % 60, 2, 10, QLatin1Char('0')));
    ui.Timer_Display->show();
    ui.kill_count->show();
    //player->w->timer.start();
    timer.start();
    //total_timer.start();
    //enemy_spown_timer.start();
}
void MainScene::keyPressEvent(QKeyEvent* key) 
{
    if (key->key() == Qt::Key_W || key->key() == Qt::Key_A || key->key() == Qt::Key_S || key->key() == Qt::Key_D)
    {
        player->keyPressEvent(key);
    }
    else if (!timer.isActive()&&(key->key() == Qt::Key_0 || key->key() == Qt::Key_1 || key->key() == Qt::Key_2 || key->key() == Qt::Key_3 || key->key() == Qt::Key_4 || key->key() == Qt::Key_5 || key->key() == Qt::Key_6 || key->key() == Qt::Key_7 || key->key() == Qt::Key_8 || key->key() == Qt::Key_9))
    {
        s.append(key->text());
        if (s.size() == 100)
        {
            s.clear();
        }
        if (s.contains("1145141919810"))
        {
            ui.level_point->setText("哼哼哼，啊啊啊啊啊啊啊！");
            ui.level_point->setAlignment(Qt::AlignCenter);
            all_hp = 1145;
            all_level = 14;
            ui.level_point->show();
            s.clear();
            map_img.load(":/Resources/homo.png");
        }
    }
}
void MainScene::keyReleaseEvent(QKeyEvent* key) 
{
    if (key->key() == Qt::Key_Escape)
    {
        if (timer.isActive() == 1)
        {
            pause();
            show_upgrate_menu();
            ui.Button_Exit->show();
            ui.restart->show();
        }
        else
        {
            game_continue();
            hide_upgrade_menu();
            ui.Button_Exit->hide();
            ui.restart->hide();
        }
    }
    else
    {
        player->keyReleaseEvent(key);
    }
}
void MainScene::paintEvent(QPaintEvent* event)
{
    QPainter p(this);
    QPen pen;
    p.drawPixmap(QRect(0,0,GAME_WIDTH,GAME_HEIGHT),map_img);//绘制地图
    QBrush brush;
    pen.setColor(QColor(255,255,0));
    pen.setStyle(Qt::SolidLine);
    brush.setColor(QColor(255,255,0));
    brush.setStyle(Qt::SolidPattern);
    p.setPen(pen);
    p.setBrush(brush);
    p.drawRects(block.b,block.n);
    pen.setColor(QColor(255, 255, 255));
    p.setPen(pen); 
    for (int i = 0; i < enemies_1.size(); ++i)
    {
        enemies_1[i]->paint(p);
    }//绘制enemy_1
    for (int i = 0; i < enemies_2.size(); ++i)
    {
        enemies_2[i]->paint(p);
    }//绘制enemy_2
    for (int i = 0; i < bullets.size(); ++i)
    {
        p.drawPixmap(QRect(bullets[i]->position_x, bullets[i]->position_y, bullets[i]->size, bullets[i]->size), bullets[i]->img);
    }//绘制bullets
    for (int i = 0; i <medicines.size(); ++i)
    {
        p.drawPixmap(medicines[i]->rect,medicines[i]->img);
    }//绘制Medicine
    if (player != nullptr)
    {
        player->paint(p);
    }//绘制玩家
};
void MainScene::update_time() 
{
    if (time > 0)
    {
        --time;
        ui.Timer_Display->display(QString("%1:%2").arg(time / 60, 2, 10, QLatin1Char('0')).arg(time % 60, 2, 10, QLatin1Char('0')));
        ++level_point_time;
        {
            if (level_point_time >= LEVEL_POINT_UP)
            {
                ++level_point;
                level_point_time -= LEVEL_POINT_UP;
            }
        }
    }
    else
    {
        pause();
        win_game_over();
    }
}