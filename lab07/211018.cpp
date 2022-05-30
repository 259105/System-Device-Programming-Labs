#include <iostream>
#include <thread>


#define START_TEMP 18.5
#define START_TARGET 18
#define TEMP_PERIOD 5
#define ADMIN_PERIOD 3

using namespace std;

double roomTemp = START_TEMP;
double goalTemp = START_TARGET;
bool heatingSystem_isOn;
bool runSystem;

void targetTemp_f(){
    while(runSystem){
        this_thread::sleep_for(chrono::seconds(TEMP_PERIOD));
        cin >> goalTemp;
        if(goalTemp == -1){
            runSystem = false;
            heatingSystem_isOn = false;
        }
    }
}

void currentTemp_f(){
    while(runSystem){
        this_thread::sleep_for(chrono::seconds(TEMP_PERIOD));
        if(heatingSystem_isOn){
            roomTemp += 0.3;
        }else{
            roomTemp -= 0.3;
        }
        cout << "roomTemp: " << roomTemp << endl;
    }
}

void admin_f(){
    while(runSystem){
        this_thread::sleep_for(chrono::seconds(ADMIN_PERIOD));
        if(heatingSystem_isOn && roomTemp > (goalTemp+0.2)){
            heatingSystem_isOn = false;
            cout << "I turned off the heating system" << endl;
        }else if(!heatingSystem_isOn && roomTemp <= (goalTemp-0.2)){
            heatingSystem_isOn = true;
            cout << "I turned on the heating system" << endl;
        }
    }
}

int main(){
    runSystem = true; // activate the system
    heatingSystem_isOn = true; // activate the system


    thread admin(admin_f), currentTemp(currentTemp_f), targetTemp(targetTemp_f);

    admin.join();
    currentTemp.join();
    targetTemp.join();

    return(0);
}
