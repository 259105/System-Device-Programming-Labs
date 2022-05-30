#include <iostream>
#include <thread>
#include <future>
#include <vector>
#include <queue>

#define MAX 15

using namespace std;

mutex m;
condition_variable adminCV, adderCV;
queue<int> taskQueue;
int var = 0;
int done;

void admin_f(){
    unique_lock<mutex> l{m};
    var = 10;
    done = 0;
    cout << "var set to 10 and done to 0" << endl;
    adderCV.notify_all();

    while(var<MAX && done < 3) {
        cout << "admin go to sleep" << endl;
        adminCV.wait(l);
        cout << "admin awoken" << endl;
    }
    cout << "final value:" << var  << endl;
}

void adder_f(){
    unique_lock<mutex> l{m};
    while(var == 0){
        cout << "adder go to sleep" << endl;
        adderCV.wait(l);
        cout << "adder awoken" << endl;
    }
    std::srand((unsigned)time(NULL));
    if(var<MAX){
        int r = std::rand() % 6;
        cout << "adder added n:" << r << endl;
        var += r;
    }
    done++;
}

int main() {
    std::vector<std::thread> adders;
    std::thread admin(admin_f);
    for(int i = 0; i < 3; i++){
        std::srand((unsigned)time(NULL)); //makes seed different for calling rand()
        adders.emplace_back(std::thread( adder_f));
    }
    for(auto& i : adders) {
        i.join();
    }
    adminCV.notify_one();
    admin.join();
    return 0;
}
