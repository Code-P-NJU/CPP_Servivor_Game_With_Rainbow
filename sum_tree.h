#pragma once
class sum_tree {
    double* data;
    int  size_;
public:
    int length;
    sum_tree(int log_length);//log_length为最大长度取log
    //void add(float f);//加f进来
    void change_(int i, float f);//第i个改成f
    float min();//最小值下标
    float max();//最大权重的值
    int search(double f);//查找
    double head();//头节点
    double& operator [] (int i);
};
sum_tree::sum_tree(int log_length) {
    size_ = 1;
    for (int i = 0; i < log_length; i++) {
        size_ *= 2;
    }
    data = new double[size_ * 2 - 1];
    for (int i = 0; i < 2 * size_ - 1; i++) {
        data[i] = 0;
    }
    length = 0;
};
/*
void sum_tree::add(float f) {
    change_(pos - size_ + 1, f);
    if (length < size_) length++;
    if (pos == 2 * size_ - 2) {
        pos = size_ - 1;
    }
    else {
        pos++;
    }
};//加f进来
*/
//void sum_tree::delete_(int i){
//    change_(size_-1+i, 0);
//};//删第i个
void sum_tree::change_(int i, float f) {
    double gap = f - data[size_ + i - 1];
    int temp = size_ + i - 1;
    while (temp > 0) {
        data[temp] += gap;
        temp = (temp - 1) / 2;
    }
    data[0] += gap;
};//第i个改成f
float sum_tree::min() {
    float m = data[size_ - 1];
    for (int k = 0; k < length; k++) {
        if (data[size_ - 1 + k] < m) {
            m = data[size_ - 1 + k];
        }
    }
    return m;
}
float sum_tree::max() {
    float m = data[size_ - 1];
    for (int k = 0; k < length; k++) {
        if (data[size_ - 1 + k] > m) {
            m = data[size_ - 1 + k];
        }
    }
    return m;
};
int sum_tree::search(double f) {
    int temp = 0;
    while (temp < size_ - 1) {
        if (data[temp * 2 + 1] >= f) {
            temp = 2 * temp + 1;
        }
        else {
            f -= data[temp * 2 + 1];
            temp = 2 * temp + 2;
        }
    }
    return temp - size_ + 1;
};//查找
double sum_tree::head() {
    return data[0];
};//头节点
double& sum_tree::operator [] (int i) {
    return data[size_ - 1 + i];
};