/***
 * A stack with pop, push, min are O(1)
 *
 ***/
#include <iostream>

using namespace std;

class MINSTACK {
    public:
        MINSTACK(int size=20);
        ~MINSTACK();
        void pop();
        void push(int n);
        int min();
    private:
        int top;
        int* stack;
        int* min_stack;
};

MINSTACK::MINSTACK(int size) : top(-1), stack(new int[size]), min_stack(new int[size]) {
    cout << "Create MINSTACK with size: " << size << endl;
}
MINSTACK::~MINSTACK() {
    cout << "Delete MINSTACK" << endl;
    delete []stack;
    delete []min_stack;
}
void MINSTACK::pop() {
    if(this->top < 0) {
        cout << "MINSTACK is empty" << endl;
        return;
    }
    this->stack[this->top] = ~0;
    this->min_stack[this->top--] = ~0;
}
void MINSTACK::push(int n) {
    this->stack[++this->top] = n;
    if(this->top > 0 && n > this->min_stack[this->top - 1]) n = this->min_stack[this->top - 1];
    this->min_stack[this->top] = n;
}
int MINSTACK::min() {
    if(this->top >= 0) {
        cout << "Min: " << this->min_stack[this->top] << endl;
        return this->min_stack[this->top];
    }
    else {
        cout << "MINSTACK is empty, no min" << endl;
        return ~0;
    }
}

int main() {
    MINSTACK* ms = new MINSTACK(20);
    ms->pop();
    ms->push(5);
    ms->push(4);
    ms->push(3);
    ms->push(1);
    ms->push(2);
    ms->min();
    ms->pop();
    ms->min();
    ms->pop();
    ms->min();
    ms->pop();
    ms->min();
    ms->pop();
    ms->min();
    ms->pop();
    ms->min();
    ms->pop();
    delete ms;
    return 0;
}
