/** Author: Hasan Almawi
* Class: asn2.cpp; main class only
* Assignment: CS3305B Assignment 2
* Create a Round Robin Scheduler Simulator
**/

//All the include statements
#include "asn2.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "queue"
#include "iostream"
#include "fstream"
#include "ostream"
#include "istream"
#include "iomanip"
#include "vector"

using namespace std;

//Defining the Process struct
//I used a struct instead of having to define a class because it would be cleaner and easier to use
struct PROCESS{
    int pid;//Integer to store the PID read in from the txt file for each process
    int arrivaltime;//Integer to store the arrival time of the process of each PID read in
    int IOfrequency;//Integer to store the IO frequency given in the text file
    int IOduration;//Integer to store the IO durtion given in the text file
    int CPUduration;//Integer to store the CPU duration given in the text file
    int CPUburst;//Integer to store the CPU burst, which is calculated through CPUDuration/IOFrequency
    int CPUdone;//Integer to check if the Process is done with the CPU it needed
    int IOdone; //Integer to indicate when the IO needs to begin for each process
    int quantum;//Integer to indicate the amount of Quantum used in each run of the process
    int q;//Integer to store which Queue the scheduler is currently in (0 or 1)
    int ioFinish;//Integer to indicate when the Process is done the IO (IOduration + time)
    std::string startState;//String to store the start state (debugging purpose only)
    std::string endState;//String to store the end state (debugging purpose only)
    };

//Defining the Compare Process arrival time for the priority queue Processes
//Sorts the Processes based on the arrival time, if they have equal arrival times then it compares the pid
struct CompareProcess{
    bool operator()(const PROCESS& p1, const PROCESS& p2){
         if(p1.arrivaltime>p2.arrivaltime) return true;
         if(p1.arrivaltime==p2.arrivaltime && p1.pid>p2.pid) return true;
         return false;
    }
};

//Defining the Compare IO finish time for the priority queue IOQ
//Sorts the Processes based on the IO finish time
struct CompareIO{
    bool operator()(const PROCESS& p1, const PROCESS& p2){
        if(p1.ioFinish>p2.ioFinish) return true;
        return false;
    }
};

//Main function
int main(int argc, char* argv[]){
    //Priority queue of Processes all sorted based on arrival time then pid
    priority_queue<PROCESS, vector<PROCESS>, CompareProcess> processes;
    //Q1 of Processes, all processes currently in the first quantum
    std::queue<PROCESS> Q1;
    //Priority queue of Processes in the IO all sorted based on the IO finish time
    priority_queue<PROCESS, vector<PROCESS>, CompareIO> IOQ;
    //Q2 of Processes, all processes currently in the second quantum
    std::queue<PROCESS> Q2;
    //Defining the Q1 quantum time
    int Q1Time=0;
    //Defining the Q2 quantum time
    int Q2Time=0;
    //Defining the temp process
    PROCESS qprocess;
    //Defining the CPU use flag since only one process can use CPU at a time
    bool CPUuse=false;

    //If the number of arguments read in is not correct, then it is incorrect
    if(argc!=4){
        cout<<"Incorrect number of arguments"<<endl;
    }
    //Otherwise it is correct
    else{
        //Initialize the time counter to 0;
        int time = 0;
        //Initialize the done flag for the program to be done
        bool done = false;
        //Initialize the Q1 quantum
        int Q1Time = atoi(argv[2]);
        //Initialize the Q2 quantum
        int Q2Time = atoi(argv[3]);
        //Reading in the file input
        std::ifstream inputFile;
        //Opening the file
        inputFile.open(argv[1]);
        //If the file is incorrect, then output error message
        if(!inputFile.good()){
            cout<<"Incorrect file input"<<endl;
            return 1;
        }
        else{
            //Reading in each line
            while(!inputFile.eof()){
                //Reading in the tokens of each line
                char tokens[512];
                inputFile.getline(tokens, 512);
                int n=0;
                //Number of tokens is 5
                const char* token[5];
                //Tokenize the strings
                token[0] = strtok(tokens, " ");
                if(token[0]){
                        for(n=1;n<5;++n){
                            token[n]=strtok(0, " ");
                        }
                        if(!token[n]) break;
                        //Initialize a process
                        PROCESS p;
                        //PID is the first token
                        p.pid = atoi(token[0]);
                        //Arrival time is the second token
                        p.arrivaltime = atoi(token[1]);
                        //CPU duration is the third token
                        p.CPUduration = atoi(token[2]);
                        //IO frequency is the fourth token
                        p.IOfrequency = atoi(token[3]);
                        //IO duration is the last token
                        p.IOduration = atoi(token[4]);
                        //CPU duration calculated
                        p.CPUburst = p.CPUduration/p.IOfrequency;
                        //Start state initalized for debugging
                        p.startState = "None";
                        //End state initialized for debugging
                        p.endState = "Newly Arrived";
                        //Every process starts in the first queue
                        p.q=0;
                        //Initialize CPU done
                        p.CPUdone=0;
                        //Initialize IO done
                        p.IOdone=0; //IOduration + clock
                        //Initialize Quantum
                        p.quantum=0;
                        //Push the process into the Processes Queue
                        processes.push(p);
                    }
            }
            //While the program is not done
            while(!done){
            //Step 1: If there is something in CPU: Run and check if it needs to go to ready, io or finished
            if(CPUuse){ //IF CPU is in use
                qprocess.quantum++;//Increase the quantum used by 1
                qprocess.CPUdone++;//Increase the CPU consumption by 1
                qprocess.IOdone++;//Increase the IO done by 1
                //If the process is done with the CPU
                if(qprocess.IOdone==qprocess.CPUburst){
                    //Reset the quantum counter
                    qprocess.quantum=0;
                    //Reset the IO done counter
                    qprocess.IOdone=0;
                    //Set the IO finish to the current time + the IO duration
                    qprocess.ioFinish=time+qprocess.IOduration;
                    //Set the CPU flag to false
                    CPUuse=false;
                    //Push the process into the IOQ to do IO
                    IOQ.push(qprocess);
                }
                //If the CPU done counter is equal to the CPU duration, then the process is done
                else if(qprocess.CPUdone==qprocess.CPUduration){
                    //Set the CPU flag to false
                    CPUuse=false;
                    //Print the PID and the time it took to finish
                    cout<<"PID: "<<qprocess.pid<<" Time: "<<time<<endl;
                }
                //If the quantum counter is equal to the Q1 quantum and it is currently in Q1
                //then it means that the CPU burst is greater than the quantum of Q1, and it needs to be put
                //in Q2
                else if(qprocess.quantum==Q1Time&&qprocess.q==0){
                    //Set CPU flag to false
                    CPUuse=false;
                    //Reset the quantum
                    qprocess.quantum=0;
                    //Set the q value to 1 to indicate it is in the second q
                    qprocess.q=1;
                    //Push the process into the second q
                    Q2.push(qprocess);
                }
                //If the process quantum counter is equal to the second quantum, then the CPU burst is greater
                //than the quantum of the second quantum, so we repush it into Q2 to keep going till it's done
                else if(qprocess.quantum==Q2Time&&qprocess.q==1){
                    //Set CPU flag to false
                    CPUuse=false;
                    //Reset the quantum
                    qprocess.quantum=0;
                    //Push the process into the second q again
                    Q2.push(qprocess);
                }
            }
            //Step 2: Check IOQ if the processes are done, or they need to go to their ready queues
                while(!IOQ.empty()&&IOQ.top().ioFinish<=time){
                    //Set the temp to the top of the IO queue
                    PROCESS temp = IOQ.top();
                    //If the process is done, then print it out
                    if(temp.CPUdone==temp.CPUduration){
                        cout<<"PID: "<<qprocess.pid<<" Time: "<<time<<endl;
                    }
                    else{
                        //If the process is in Q1, then push it back into Q1
                        if(temp.q==0){
                            Q1.push(temp);
                        }
                        //Otherwise push it back into Q2
                        else{
                            Q2.push(temp);
                        }
                    }
                    //Pop thte IO Q since we're done with this process in the IO
                    IOQ.pop();
                }
            //Step 3: Check new process arrived queue to see if it matches time to add it to Q1
                while(processes.top().arrivaltime==time&&!processes.empty()){
                    //Set the temp to the top of the process
                    PROCESS temp = processes.top();
                    //Pop the processes queue
                    processes.pop();
                    //Push the process into the first queue
                    Q1.push(temp);
                }
            //Step 4: If there is nothing in the CPU, check ready q1 if there is anything to move there, if not then check ready queue 2
                if(!CPUuse){
                    //If Q1 is not empty, then there is a process to be run
                    if(!Q1.empty()){
                        //Process is the front of the queue
                        qprocess=Q1.front();
                        //CPU flag set to true
                        CPUuse=true;
                        //Pop the process
                        Q1.pop();
                    }
                    //If Q2 is not empty, then there is a process to be run
                    else if(!Q2.empty()){
                        //Process is the front of the queue
                        qprocess=Q2.front();
                        //CPU flag set to true
                        CPUuse=true;
                        //Pop the process
                        Q2.pop();
                    }
                    //If the IOQ and all the rest of the queues are empty, then we are done
                    else if(IOQ.empty()&&processes.empty()){
                        //Done flag is triggered
                        done=true;
                    }
                }
                //Increment time at the end of the loop
                time++;
            }
        }
    }
}