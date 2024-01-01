#pragma once
#include "DQN.h"
#include <QtWidgets/QApplication>
#include <QPixmap>
#include "windows.h"
#include <fstream>
#include <c10/cuda/CUDAStream.h>
#include <ATen/cuda/CUDAEvent.h>
#pragma comment(lib,"User32.lib")
c10::Scalar loss_v;
double gamma = 0.99;//折扣因子,一般在[0,1]之间，越大越在乎远期利益，0时只在乎眼前收益,经验公式: 到达终点需要的平均step=1/(1-gamma)
double lr = 3e-4; //初始学习率
double max_lr = 1e-3;//最大学习率
double min_lr = 1e-6;//最低学习率
//double epsilon = 0.3;//随机动作的概率,越大越随机(noisy_net下不需要)
int batch_size = 64;
int target_update = 10;
//float delta = 10;//huberloss参数
float alpha = 1;//优先级经验回放的参数alpha,决定我们要使用多少 ISweight 的影响, 如果 alpha = 0, 我们就没使用到任何 Importance Sampling.
float beta = 0.4;//优先级经验回放的参数beta,决定你有多大的程度想抵消Prioritized Experience Replay对收敛结果的影响。
int steplr_step_size = 1;
float steplr_gamma = 0.9;
int n = 0;//倒数n次迭代保持epsilon最低,此项为了epsilon下降策略而存在，使用noisynet时设为零
int N_step_replay = 5;//multi_step dqn,取1时退化为传统dqn
int atoms = 51;
int V_min = -10, V_max = 70;
const std::string PATH = { "model//model.pt" };
int episode_num = 4000, Iteration_num = 40;
int best_train = 0;
int best_test = 0;
c10::Device device = c10::Device(torch::cuda::is_available() ? torch::DeviceType::CUDA : torch::DeviceType::CPU);
int main(int argc, char *argv[])
{
    system("set CUDA_LAUNCH_BLOCKING=0");
    try {
        QApplication a(argc, argv);
        DQN Dqn;
        std::fstream  f;
        f.open(PATH);
        if (f.is_open() == 1)
        {
            Dqn.load_net(PATH);
        }
        f.close();
        // float temp = epsilon;
         //epsilon = 0;

        /*模型测试模块
        for (int i = 0; i < 10; ++i)
        {
            Dqn.test(0);
        }
        return 0;
        //*/

        //模型初始状态测试
        float re = Dqn.episode_evaluate_on_train(1);
        std::cout << "start_state:| " << re <<" | ";
        re = Dqn.episode_evaluate(0);
        std::cout << re << " | " << std::endl;;

        //Dqn.human_control();
         // epsilon = temp;
         
        //模型训练部分
        for (int j = 1; j <= (episode_num / Iteration_num) - n; ++j)
        {
            // if (epsilon < 0.05)
            //   epsilon = 0.05; 
            if(j%steplr_step_size==0)
            Dqn.lr_step();
            for (int i = 1; i <= Iteration_num; ++i)
            {
                srand(time(0));
                Dqn.env_seed(114513+i-rand()%2*Iteration_num+Iteration_num);//在此调整训练种子的选取
                printf("\rIteration:[%d/%d] : [%d/%d]", j, episode_num / Iteration_num, i, Iteration_num);
                for (int z = 1; z <= i * 20 / Iteration_num; z++)
                {
                    std::cout << '=';
                }
                std::cout << '>';
                Dqn.run_episode();
            }
            //temp = epsilon;
           // epsilon = 0.05;
            float reward_train = Dqn.episode_evaluate_on_train(0);
            float reward = Dqn.episode_evaluate(0);
            //epsilon = temp;
            std::cout << "| episode |" << j * Iteration_num << "| total_reward_on_train |" << reward_train << "| total_reward_on_test |" << reward /* << "| epsilon: " << epsilon */ << "| Loss |" << loss_v << "| learning_rate |"<<lr << std::endl;

            Dqn.save_net(PATH);
            if (reward_train > best_train) 
            {
                Dqn.save_net("model//model_best_train.pt");//在测试种子上表现最好的模型
                best_train = reward_train;
            }
            if (reward > best_test)
            {
                Dqn.save_net("model//model_best_test.pt");//在训练种子上表现最好的模型
                best_test = reward;
            }
            //epsilon -= (0.3-0.05) / ((episode_num / Iteration_num) - n); //epsilon线性递减
            //c10::cuda::CUDACachingAllocator::emptyCache();
        }
        //epsilon = 0.05;
        for (int j = (episode_num / Iteration_num) - n + 1; j <= episode_num / Iteration_num; ++j)
        {
            for (int i = 1; i <= Iteration_num; ++i)
            {
                Dqn.env_seed(114513 + i);
                printf("\rIteration:[%d/%d] : [%d/%d]", j, episode_num / Iteration_num, i, Iteration_num);
                for (int z = 1; z <= i * 20 / Iteration_num; z++)
                {
                    std::cout << '=';
                }
                std::cout << '>';
                Dqn.run_episode();
            }
            float reward_train = Dqn.episode_evaluate_on_train(0);
            float reward = Dqn.episode_evaluate(0);
            std::cout << "| episode |" << j * Iteration_num << "| total_reward_on_train |" << reward_train << "| total_reward_on_test |" << reward /* << "| epsilon: " << epsilon */ << "| Loss |" << loss_v << "| learning_rate |" << lr << std::endl;
            if (j % steplr_step_size == 0)
                Dqn.lr_step();
            Dqn.save_net(PATH);
        }
        Dqn.test(0);
        Sleep(1000);
        Dqn.test(0);
        std::cout<<"press any key to close";
        std::cin >> n;
        return  0;
    }
    catch (torch::Error e)
    {
        int x;
        std::cout <<std::endl<< e.msg();
        std::cout <<std::endl<< e.what();
        std::cin >> x;
    }
}