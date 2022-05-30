#include <iostream>
#include <future>
#include <cmath>

using namespace std;

int take_value(){
    int val;
    cout << "Please insert a fucking number :" << endl;
    cin >> val;
    return val;
}

bool check_if_prime(future<int> fVal){
    int val = fVal.get(); // wait until val is read from the command line
    for(int i = 2 ; i <= val/2; i++)
        if(val%i == 0)
            return false;
    return true;
}

bool give_answer(future<bool> fAns){
    bool ans = fAns.get(); // wait until the check is producted
    if(ans)
        cout << "the number is prime" << endl;
    else
        cout << "the number isn't prime" << endl;
    return true;
}

int main() {
    future<int> fVal = async(launch::async, take_value);
    future<bool> fAns = async(launch::async, check_if_prime, move(fVal));
    future<bool> fDone = async(launch::async, give_answer, move(fAns));
    fDone.get();
    return 0;
}
