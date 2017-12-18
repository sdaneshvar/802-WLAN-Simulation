#include <iostream>
#include <cstdlib>
#include <cmath>
#include <queue>
#include <list>
using namespace std;

struct event {
    bool type;
    double eventTime;
};

struct packet {
    double serTime;
};

double NEDT (double rate)
{
    double u;
    u = drand48();
    return ((-1/rate)*log(1-u));
}

bool compare(event *first, event *second)
{
    return first->eventTime < second->eventTime;
}

int main ()
{
    // Initialize GEL
    list <event *> gel;

    // Initialize Queue
    list <packet *> buffer;
    int MAXBUFFER = 1000000; // #3: {1, 20, 50}

    // Counters
    double currClock = 0;
    double prevClock = 0;
    double freeTime = 0;
    double lastTime = 0;
    double busyTime = 0;
    double meanQ = 0;
    int length = 0; // # of packets in buffer
    int dropTop = 0;

    // Service & arrival rate
    double mu = 1;
    double lambda = 0.9; //#1: {0.1, 0.25, 0.4, 0.55, 0.65, 0.80, 0.90}; #3: {0.2, 0.4, 0.6, 0.8, 0.9}

    // first GEL event
    event *e = new event;
    e->type = false;
    e->eventTime = NEDT(lambda) + currClock;
    gel.push_back(e);

    for (int i = 0; i < 100000; i++)
    {
    	//cout << currClock << endl;
        if(gel.front()->type == false)
        {
            currClock = gel.front()->eventTime;
            packet *p = new packet;
            p->serTime = NEDT(mu);

            event *eNew = new event;
            eNew->type = false;
            eNew->eventTime = currClock + NEDT(lambda);

            gel.push_back(eNew);
            //gel.sort(compare);
            if (length == 0)
            {
                // create departure
                event *eNewer = new event;
                eNewer->eventTime = currClock + p->serTime;
                eNewer->type = true;
                gel.push_back(eNewer);

                freeTime = freeTime + (currClock - lastTime);

                buffer.push_back(p);
                length++;
            }
            else 
            {
                if (length - 1 < MAXBUFFER) // buffer is not full
                {
                    buffer.push_back(p);
                    length++;
                }
                else // buffer is full
                {
                    dropTop++;
                }
            }
        }
        else // gel.front()->type = true
        {
            currClock = gel.front()->eventTime;
            if (length > 0)
            {
                buffer.pop_front();
                if (length == 1)
                	lastTime = currClock;
                length--;
                event* eNew = new event;
                eNew->eventTime = currClock + NEDT(mu);
                eNew->type = true;
                gel.push_back(eNew);
            }
        }
        gel.pop_front();
        gel.sort(compare);
        meanQ = meanQ + (length * (currClock-prevClock));
        prevClock = currClock;
        //cout << "Buffer length is " << length << endl;
        //cout << "Clock is " << currClock << endl;
    }
    busyTime = currClock - freeTime;
    double busyPercent = (busyTime/currClock)*100;
    double avgQ = (meanQ / currClock);
    //cout << "Link processor utilization: " << busyPercent << endl;
    cout << "Mean queue length " << avgQ << endl;

    return 0;
}
