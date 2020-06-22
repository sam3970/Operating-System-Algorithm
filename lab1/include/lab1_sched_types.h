/*
*	DKU Operating System Lab
*	    Lab1 (Scheduler Algorithm Simulator)
*	    Student id : 32131706,32147356
*	    Student name : 백준하, 이상규
*
*   lab1_sched_types.h :
*       - lab1 header file.
*       - must contains scueduler algorithm function's declations.
*
*/
#ifndef _LAB1_HEADER_H
#define _LAB1_HEADER_H
#include <queue>
using namespace std;
//================FIFO, RR =======================
typedef struct process {
	int id;// PID
	int arrivaltime;
	int cpuburst;
	int priority; //낮은 숫자는 높은 우선순위를 의미합니다.
	int waitingtime;
	int turnaroundtime;
} PROCESS;

void createProcess();
void printProcess();
void FCFS();
void Roundrobin();
void gantt_Chart(int record[], int time, float awt, float att);
//============================================================

//=======================MLFQ, RM=============================


class RMScheduler;

class Process {
public:
	char identifier; //프로세스 식별자
	int remainingTime; //남은 수행시간
	int arrivalTime; //프로세스 도착시

	Process(char id = 0, int remainingT = 0, int arrivalT = 0);
};

class Process_MLFQ : public Process {
public:
	int level;	//큐의 어느레벨에 위치하는지
	int curHoldingTime;	//현재 level에서 수행한시
	Process_MLFQ(char id, int remainingT, int arrivalT, int level);
};

class Process_RM : public Process {
public:
	int period;	//작업의 주기
	int timePerPeriod;	//주기당 작업시간
	int elapsedInPeriod;	//현재주기에서 지난 시
	Process_RM(char id = 0, int _period = 0, int _timePerPeriod = 0, int arrivalT = 0);
	bool comparePeriod(Process_RM* target);	//주기의 대소비
};

class Scheduler {
protected:
	int completedProcessNum;	//완료된 프로세스 
	int totalProcessNum;		//시뮬레이션 대상 프로세스 수
	int curTime;			//현재시간
	queue<Process *> *unArrivedProcessQueue;	//도착되지 않은 프로세스의 대기큐
	Process * curWorkingProcess;			//현재 수행중인 프로세스

	virtual Process ** getNewProcesses();		//도착한 프로세스들 목록을 받는다
	virtual void processArrive() = 0;			//도착한 프로세스들을 대기큐에 넣는다
	virtual Process * getNextProcess() = 0;		//다음 수행할 프로세스를 찾는다
	virtual int doWork(Process * workingProcess) = 0;	//프로세스에 대해 작업 수행
	void mainAlgorithm();				//자식 스케줄러들이 공통적으로 실행할 알고리듬
public:
	virtual void runScheduler() = 0;			//이 메소드를 이용 스케줄링 시작한다
	Scheduler();
	~Scheduler();
};

class MLFQScheduler :public Scheduler {
	int numOfLevel;					//큐의 레벨수
	queue<Process *> *multi_level_queue;		//큐의 배열

protected:
	void processArrive();
	Process * getNextProcess();
	int doWork(Process * workingProcess);
	Process * curProcessRearrange(Process * curWorking); //수행 프로세스를 종료/재배치
	bool onlyOneQueue();				     //프로세스가 유일한지 확인
public:
	MLFQScheduler(int processNum);
	void runScheduler();
};

class RMScheduler :public Scheduler {

protected:
	int simulatingTime;		//총 시뮬레이팅할 시간
	int curWorkingNum;		//현재 수행중인 프로세스 개수

	void processArrive();
	Process * getNextProcess();
	int doWork(Process * workingProcess);

	Process_RM **curWorkingArray;	//현재 수행중인 프로세스 배열
	Process_RM * renewPeriod();	//주기 갱신/정상동작 체크

public:
	RMScheduler(int processNum, int _simulatingTime);
	void runScheduler();
};


//============================================================

#endif /* LAB1_HEADER_H*/



