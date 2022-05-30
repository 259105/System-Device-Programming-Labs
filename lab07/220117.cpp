#include <iostream>
#include <vector>
#include <thread>
#include <mutex>

using namespace std;

int currTime;
int startTime;

int nP;
int nBusyPark;

mutex mNBusyPark;


void newCarArrive(){
    thread::id tid = std::this_thread::get_id();
    cout << "car " << tid << " try to enter in the park" << endl;

    unique_lock<mutex> l{mNBusyPark};
    if(nBusyPark>=nP){
        cout << "The park is full" << endl;
        return;
    }
    nBusyPark++;

    cout << tid << " parked" << endl;
    l.unlock();

    // now the car compute the time and go to sleep
    srand( nBusyPark );

    int t2 = rand() % 4 + 4;
    cout << tid << " will sleep for " << t2 << " seconds" << endl;
    std::this_thread::sleep_for(chrono::seconds(t2));
    // after sleeping the car go out of the parking

    l.lock();
    nBusyPark--;
    cout << tid << " go out" << endl;

}

int main(int argc, char **argv){
    if(argc != 3){
        cout << "wrong number of parameters" << endl;
        exit(1);
    }
    nP = atoi(argv[1]);
    int totTime = atoi(argv[2]);
    currTime = 0;
    startTime = (unsigned)time(NULL);;
    nBusyPark = 0;
    srand((unsigned int) nP);
    int t1;

    vector<thread> threads;

    while(currTime-startTime<totTime){
        t1 = rand() % 3 + 1;
        std::this_thread::sleep_for(chrono::seconds(t1));
        // run the thread
        threads.emplace_back(thread(newCarArrive));

        currTime = (unsigned)time(NULL);
    }

    for(auto &t : threads){
        t.join();
    }

    return 0;
}
