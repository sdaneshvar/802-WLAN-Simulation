#include <iostream>
#include <cstdlib>
#include <cmath>
#include <math.h>
#include <queue>
#include <list>
#include <random>
using namespace std;

struct event {
    int type; // 0 = arrival, 1 = departure, 2 = dataframe, 3 = acknowledgement, 4 = sync
    double eventTime; // timestamp when it happens
    int packetSize; // randomly generated r value, how long it takes to process r bytes
    int source;
    int dest;
    int retries = 1;
};

struct packet {
    double serTime;
    int source;
    int dest;
};

/*struct buffer {
    list<packet *> bQ;
};*/

struct host {
    list<packet *> buffer;
    double backoff;
    int length;
    int retries;
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

double fRand(double fMin, double fMax)
{
    double f = (double)rand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}

int fRando(int fMin, int fMax)
{
    int randNum = rand()%(fMax-fMin + 1) + fMin;
    return randNum;
}

void genDataFrameLen(event* e)
{
    double r = fRand(.0001, .01);  

    default_random_engine generator;
    exponential_distribution<double> distribution(r);

    double number = distribution(generator);
    while (number > 1544 || number < 1) 
    { 
        cout << number << endl;
        genDataFrameLen(e); 
    }

    e->packetSize = floor(number + 0.5);
}

int genTrans(event* e)
{
    // calculate tranmission time of r size bytes
    int transTime = 0;
    transTime = (e->packetSize * 8) / (11 * pow(10, 6));
    return transTime;
}

double genBackoff( int array[], int T, int retries)
{
    double backoff = floor(((drand48() * T) * retries) + 0.5);
    for (int i = 0; i < 10; i++)
    {
        if (array[i] == int(backoff)) {
            genBackoff(array, T, retries);
        }
        array[i] = backoff;
    }
    return backoff;
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
    int bo = 0; // # of packets in buffer


    // Service & arrival rate
    double mu = 1;
    double lambda = 0.9; //#1: {0.1, 0.25, 0.4, 0.55, 0.65, 0.80, 0.90}; #3: {0.2, 0.4, 0.6, 0.8, 0.9}


    // first GEL event
    event *e = new event;
    e->type = 0;
    e->eventTime = NEDT(lambda) + currClock;
    e->source = fRando(1, 10);
    e->dest = fRando(1, 10);
    while (e->source == e->dest) { e->dest = fRando(1, 10); }
    gel.push_back(e);


    // create N hosts
    host* hosts = new host[10];
    int globalArray[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; 
    // how to access host objects psuedo code

    for (int i = 0; i < 10; i++)
    {
        hosts[i].backoff = genBackoff( globalArray, 10, 1);
    } 

    for (int i = 0; i < 10; i++)
    {
        cout << hosts[i].backoff << " " << globalArray[i] << endl;
    }
    /*
    for (int i = 0; i < 10; i++)
    {
        cout << hosts[i].bQ.front()->serTime << endl;
    }

    event *eTest = new event;
    genDataFrameLen(eTest);
    genDataFrameLen(e);
    cout << "Packet size is " << eTest->packetSize << " bytes." << endl;
    cout << "Packet size is " << e->packetSize << " bytes." << endl;
    // end psuedo*/

    for (int i = 0; i < 100000; i++)
    {
        //cout << currClock << endl;
        if(gel.front()->type == 0) // event is an arrival
        {
            currClock = gel.front()->eventTime;
            packet *p = new packet;
            p->serTime = NEDT(mu);
            p->source = gel.front()->source;
            p->dest = gel.front()->dest;

            event *eNew = new event;
            eNew->type = 0;
            eNew->eventTime = currClock + NEDT(lambda);
            eNew->source = fRando(1, 10);
            eNew->dest = fRando(1, 10);
            while (e->source == e->dest) { e->dest = fRando(1, 10); }

            gel.push_back(eNew);
            //gel.sort(compare);
            if (hosts[gel.front()->source].length == 0)
            {
                // create departure
                event *eNewer = new event;
                eNewer->eventTime = currClock + p->serTime;
                eNewer->type = 1;
                eNewer->source = gel.front()->source;
                eNewer->dest = gel.front()->dest;
                gel.push_back(eNewer);

                freeTime = freeTime + (currClock - lastTime);

                hosts[gel.front()->source].buffer.push_back(p);
                hosts[gel.front()->source].length++;
            }
            else 
            {
                hosts[gel.front()->source].buffer.push_back(p);
                hosts[gel.front()->source].length++;
            }
        }
        if (gel.front()->type == 1) // event is a departure
        {
            currClock = gel.front()->eventTime;
            if (hosts[gel.front()->source].length == 0)
            {
                if (hosts[gel.front()->source].backoff == 0 )
                {

                }
            }
            if (hosts[gel.front()->source].length > 0)
            {
                buffer.pop_front();
                if (hosts[gel.front()->source].length == 1)
                    lastTime = currClock;
                hosts[gel.front()->source].length--;
                event* eNew = new event;
                eNew->eventTime = currClock + NEDT(mu);
                eNew->type = 1;
                gel.push_back(eNew);
            }
        }
        /*if (gel.front()->type == 2) // event is a dataFrame
        {

        }
        if (gel.front()->type == 3) // event is an acknowledgment
        {

        }
        if (gel.front()->type == 4) // event is a sync
        {

        }*/
        gel.pop_front();
        gel.sort(compare);
        //meanQ = meanQ + (length * (currClock-prevClock));
        prevClock = currClock;
        //cout << "Buffer length is " << length << endl;
        //cout << "Clock is " << currClock << endl;
    }
    busyTime = currClock - freeTime;
    double busyPercent = (busyTime/currClock)*100;
    double avgQ = (meanQ / currClock);
    //cout << "Link processor utilization: " << busyPercent << endl;
    cout << "length " << avgQ << endl;

    return 0;
}
