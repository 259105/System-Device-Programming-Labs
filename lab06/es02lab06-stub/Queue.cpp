#include "Queue.h"
#include <iostream>
#include <vector>

Queue::Queue() : _size(0), _queue(nullptr) {
    std::cout << "Default constructor" << std::endl;
}

enum Client Queue::pop() {
    if(empty())
        throw QueueEmptyException();
    enum Client val = _queue[0];
    Client *newQueue = new Client[_size-1];
    for(int i=1; i<_size; i++)
        newQueue[i-1] = _queue[i];

    _queue = newQueue;
    _size--;
    return val;
}

bool Queue::empty() {
    return _size == 0;
}

void Queue::_pushAt(enum Client value, int position) {
    Client *newQueue = new Client[_size+1];
    for(int i=0;i<position;i++)
        newQueue[i] = _queue[i];
    newQueue[position] = value;
    for(int i=position+1;i<_size+1;i++)
        newQueue[i] = _queue[i-1];

    _queue = newQueue;
    _size++;
}

void Queue::printQueue() {
    for(int i=0; i<_size; i++)
        std::cout << i+1 << ": " << _queue[i] << std::endl;
}

int Queue::push(enum Client client) {
    int i, position;
    switch(client) {
        case PRIORITY:
            for (i = 0; i < _size; i++) {
                if (_queue[i] != PRIORITY)
                    break;
            }
            position = i;
            _pushAt(client, position);
            break;
        case MONEY:
            position = _size;
            _pushAt(client, position);
            break;
        case POSTAL:
            int nMoney = 0;
            for(i = 0; i < _size; i++) {
                if (_queue[i] == MONEY)
                    ++nMoney;
                if (nMoney > 3)
                    break;
            }
            position = i;
            _pushAt(client, position);
            break;
    }
    return position;
}




