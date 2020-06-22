/*
*	DKU Operating System Lab
*	    Lab1 (Scheduler Algorithm Simulator)
*	    Student id : 32131706,32147356
*	    Student name : 백준하,이상규
*
*   lab1_sched.c :
*       - Lab1 source file.
*       - Must contains scueduler algorithm function'definition.
*
*/

#include <aio.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <pthread.h>
#include <asm/unistd.h>
#include <cmath>
#include <iostream>

#include "lab1_sched_types.h"

using namespace std;

int i, j, k, temp, swap; //자리 이동을 위한 임시변수와 인덱스 변수
int num;//프로세스의 갯수를 랜덤으로 할당할 변수

PROCESS* processarr;

//===========================FIFO,RR============================
void createProcess() {

	printf("랜덤하게 프로세스를 생성합니다. 아무키나 눌러주십시오.\n");
	getchar();

	num = (rand() % 2) + 3; // 2 ~ 5의 프로세스를 시드함수를 이용해 랜덤으로 생성

	processarr = (PROCESS*)malloc(sizeof(PROCESS)*num);

	for (i = 0; i < num; i++) {
		processarr[i].id = i;
		processarr[i].arrivaltime = rand() % 5; // 도착시간(0 ~ 5)
		processarr[i].cpuburst = rand() % 5 + 1; // cpu burst (1 ~ 5)
		processarr[i].priority = rand() % 15 + 1; //해당 값이 낮으면 우선순위가 높음을 의미한다.
		processarr[i].waitingtime = 0; //awt 초기화
		processarr[i].turnaroundtime = 0; //tat 초기화
	}
	printf("%d개의 프로세스가 생성되었습니다.\n\n", num);

}

void printProcess() { //print processes info
	for (i = 0; i < num; i++) {
		printf("Process_id : %d, arrival_time : %d, cpu_burst : %d \n", processarr[i].id, processarr[i].arrivaltime, processarr[i].cpuburst);
	}
}

/* FCFS(FIFO) 구현부 */
void FCFS() {

	PROCESS* processcopy;
	processcopy = (PROCESS*)malloc(sizeof(PROCESS)*num);

	for (i = 0; i < num; i++) { //copy processarr
		processcopy[i].id = processarr[i].id;
		processcopy[i].arrivaltime = processarr[i].arrivaltime;
		processcopy[i].cpuburst = processarr[i].cpuburst;
		processcopy[i].priority = processarr[i].priority;
		processcopy[i].waitingtime = processarr[i].waitingtime;
		processcopy[i].turnaroundtime = processarr[i].turnaroundtime;
	}


	int readyqueue[20];
	int eoq = 0; //(큐에서 마지막 프로세스(준비상태)의 인덱스) + 1,  (큐의 마지막 위치 값을 저장하기 위함.)
	int time = 0;//시간 초기화
	int running = -1; //작업하고 있는 프로세스 ,-1 는 ready 상태
	int lastarrival = 0;// 마지막 프로세스의 도착시간을 저장할 변수
	int end = 0; //모든 프로세스가 종료됨을 알리는 변수
	int record[100]; //간트차트 기록을 위한 변수
	float awt = 0;// average waiting time
	float att = 0;// average turnaround time


	for (i = 0; i < num; i++) {
		if (processcopy[i].arrivaltime > lastarrival)
			lastarrival = processcopy[i].arrivaltime;
	}

	while (1) {
		/* 프로세스가 도착했을 때 */
		for (i = 0; i < num; i++) {
			if (processcopy[i].arrivaltime == time) {
				readyqueue[eoq] = processcopy[i].id;
				eoq++;
			}
		}

		/* FCFS (scheduled) */
		if (running == -1 && (eoq > 0)) {
			running = readyqueue[0];
			for (i = 0; i < eoq - 1; i++) {
				readyqueue[i] = readyqueue[i + 1];
			}
			eoq--;

		}
		record[time] = running; //간트차트 작성을 위해 실행되고있는 프로세스를 기록함.

		if (running >= 0) //cpu burst 시간 감소
			processcopy[running].cpuburst--;

		time++;// 시간 증가시킴

		for (i = 0; i < eoq; i++) //ready상태의 queue에서의 대기시간 증가
			processcopy[readyqueue[i]].waitingtime++;

		if (running >= 0) {//만약 cpuburst가 0 이라면, 프로세스는 종료되며 -1로 설정함.
			if (processcopy[running].cpuburst == 0) {
				processcopy[running].turnaroundtime = time - processcopy[running].arrivaltime;
				running = -1;
			}

		}

		/* 모든 프로세스가 도착했고 실행할 프로세스가 없다면 ready 큐에 아무것도 없음을 의미. */
		if ((time > lastarrival) && (running == -1) && (eoq == 0))
			end = 1; //끝났음을 의미.

		if (end) //end 값이 1이면 끝났음을 의미.
			break;//스케쥴링을 끝내고 무한루프 탈출.
	}

	/* awt att 계산 */
	for (i = 0; i < num; i++) {
		awt += processcopy[i].waitingtime;
		att += processcopy[i].turnaroundtime;
	}

	awt /= (float)num;
	att /= (float)num;



	printf("1. FCFS \n\n");
	gantt_Chart(record, time, awt, att); //FCFS의 간트차트 출력

}

/* 라운드 로빈 구현부 */
void Roundrobin() {
	int quantum = (rand() % 5) + 1;//타임 퀀텀을 랜덤하게 설정(1 ~ 5)
	int quantumleft = quantum;//잔여 타임 퀀텀을 설정

	/* 프로세스 사용을 위한 동적할당 */
	PROCESS* processcopy;
	processcopy = (PROCESS*)malloc(sizeof(PROCESS)*num);

	for (i = 0; i < num; i++) {
		processcopy[i].id = processarr[i].id;
		processcopy[i].arrivaltime = processarr[i].arrivaltime;
		processcopy[i].cpuburst = processarr[i].cpuburst;
		processcopy[i].priority = processarr[i].priority;
		processcopy[i].waitingtime = processarr[i].waitingtime;
		processcopy[i].turnaroundtime = processarr[i].turnaroundtime;
	}


	int readyqueue[20];
	int eoq = 0;
	int time = 0;
	int running = -1;
	int lastarrival = 0;
	int end = 0;
	int record[100]; //간트차트 기록을 위한 변수
	float awt = 0;// average waiting time
	float att = 0;// average turnaround time


	for (i = 0; i < num; i++) {
		if (processcopy[i].arrivaltime > lastarrival)
			lastarrival = processcopy[i].arrivaltime;
	}

	while (1) {
		/* 프로세스 도착 */
		for (i = 0; i < num; i++) {
			if (processcopy[i].arrivaltime == time) {
				readyqueue[eoq] = processcopy[i].id;
				eoq++;
			}
		}

		if (running == -1 && (eoq > 0)) {
			running = readyqueue[0];
			quantumleft = quantum;
			for (i = 0; i < eoq - 1; i++) {
				readyqueue[i] = readyqueue[i + 1];
			}
			eoq--;

		}
		record[time] = running; //간트차트 작성을 위해 실행되고있는 프로세스를 기록함.

		//cpu_burst 감소
		if (running >= 0) {
			processcopy[running].cpuburst--;
			quantumleft--;
		}

		time++;


		for (i = 0; i < eoq; i++) // ready상태의 queue에서의 대기시간 증가
			processcopy[readyqueue[i]].waitingtime++;

		if (running >= 0) { //만약 cpuburst가 0 이라면, 프로세스는 종료되며 -1로 설정함.
			if (processcopy[running].cpuburst == 0) {
				processcopy[running].turnaroundtime = time - processcopy[running].arrivaltime;
				running = -1;
			}

		}

		/*타임퀀텀이 0이 되었다면 */
		if ((running >= 0) && (quantumleft == 0))
		{
			readyqueue[eoq] = processcopy[running].id; //실행되고 있는 프로세스를 준비상태의 큐에 넣고 끝냄.
			eoq++; //eoq 증가.
			running = -1; //idle 상태로 돌림.
		}

		/*작동시간이 프로세스의 마지막 도착시간보다 크고 큐의 마지막 위치가 0이라면*/
		if ((time > lastarrival) && (running == -1) && (eoq == 0))
			end = 1; // 프로세스 종료

		/* 실행 끝. */
		if (end)
			break;
	}

	/* awt, att 시간 출력 */
	for (i = 0; i < num; i++) {
		awt += processcopy[i].waitingtime;
		att += processcopy[i].turnaroundtime;
	}

	awt /= (float)num;
	att /= (float)num;



	/* 라운드 로빈의 간트차트 출력 */
	printf("4. RoundRobin, time quantum = %d \n\n", quantum);
	gantt_Chart(record, time, awt, att);
}

/* 간트차트 구현부 */
void gantt_Chart(int record[], int time, float awt, float att) {

	if (time <= 25) {
		/*for (i = 0; i < time; i++)
			printf("----");

		printf("\n");
		*/
		for (i = 0; i < time; i++) {
			if (record[i] >= 0)
				printf(" %d |", record[i]);
			else
				printf(" i |");
		}
		printf("\n");

		for (i = 0; i < time; i++)
			printf("----");

		printf("\n");


		for (i = 0; i <= time; i++)
			printf("%-4d", i);
		printf("\n");
		printf("\n");
	}

	else {
		/*
		for (i = 0; i < 25; i++)
			printf("----");

		printf("\n");
		*/
		for (i = 0; i < 25; i++) {
			if (record[i] >= 0)
				printf(" %d |", record[i]);
			else
				printf(" i |");
		}
		printf("\n");

		for (i = 0; i < 25; i++)
			printf("----");

		printf("\n");

		for (i = 0; i <= 25; i++)
			printf("%-4d", i);
		printf("\n");

		for (i = 25; i < time; i++)
			printf("----");
		printf("\n");


		for (i = 25; i < time; i++) {
			if (record[i] >= 0)
				printf(" %d |", record[i]);
			else
				printf(" i |");
		}
		printf("\n");

		for (i = 25; i < time; i++)
			printf("----");

		printf("\n");


		for (i = 25; i <= time; i++)
			printf("%-4d", i);
		printf("\n");
		printf("\n");
	}
	printf("Average Wait Time: %f, Average Turnaround Time: %f", awt, att);

	printf("\n");
	printf("\n");

}
//========================================================================

//======================MLFQ,RM=========================================



Process::Process(char id, int remainingT, int arrivalT) {
	identifier = id;
	remainingTime = remainingT;
	arrivalTime = arrivalT;

}

Process_MLFQ::Process_MLFQ(char id, int remainingT, int arrivalT, int _level) :Process(id, remainingT, arrivalT) {
	level = _level;
	curHoldingTime = 0;
}

Process_RM::Process_RM(char id, int _period, int _timePerPeriod, int arrivalT) : Process(id, _timePerPeriod, arrivalT) {
	period = _period;
	timePerPeriod = _timePerPeriod;
	elapsedInPeriod = 0;
}

bool Process_RM::comparePeriod(Process_RM * target) {
	int tPeriod = target->period;
	if (period > tPeriod)return 1;
	if (period == tPeriod)return 0;
	return -1;
}

Scheduler::Scheduler() {
	completedProcessNum = 0;
	curTime = -1;
	unArrivedProcessQueue = new queue<Process *>();
}

Scheduler::~Scheduler() {}

Process** Scheduler::getNewProcesses() {
	Process** tempArr;
	Process * curProcess = 0;
	int readySize;

	tempArr = new Process*[totalProcessNum + 1];
	tempArr[totalProcessNum] = 0;
	readySize = unArrivedProcessQueue->size();
	int index = 0;
	for (int i = 0; i < readySize; i++) {
		curProcess = (Process *)(unArrivedProcessQueue->front());

		if (curProcess->arrivalTime <= curTime) {
			tempArr[index] = curProcess;
			unArrivedProcessQueue->pop();
			index++;
		}
		else {
			break;
		}
	}
	return tempArr;
}

void Scheduler::mainAlgorithm() {
	int elapsed = 1;
	//cout << "curTime : " << curTime << " ";
	curWorkingProcess = getNextProcess();
	if (curWorkingProcess) {
		elapsed = doWork(curWorkingProcess);
	}
	else {
		if (curTime >= 0)cout << "0";
	}
	curTime += elapsed;
}

//==========MLFQ Scheduler============

MLFQScheduler::MLFQScheduler(int processNum) {
	//입력받은 process들을 도착시간 순으로 정렬하여 unArrivedProcessQueue에 넣어준다.
	totalProcessNum = processNum;
	Process **tempArr;
	int *indexArr;
	int *atArr;
	char id;
	int remainingTime;
	int arrivalTime;
	int temp;

	numOfLevel = 3;	//이 값을 변경하면 더 많은 큐를 가질 수 있다.

	tempArr = new Process*[totalProcessNum];
	indexArr = new int[totalProcessNum];
	atArr = new int[totalProcessNum];

	multi_level_queue = new queue<Process *>[numOfLevel];
	Process *tempProcess;

	cout << "Create process list for MLFQScheduler...\n";
	cout << "please type following direction-->\n";
	cout << "processidentifier serviceTime arrivalTime\n";
	cout << "ex) a 0 3\n";
	for (int i = 0; i < totalProcessNum; i++) {
		cout << i + 1 << " process : ";
		cin >> id >> arrivalTime >> remainingTime;
		indexArr[i] = i;
		atArr[i] = arrivalTime;
		tempProcess = new Process_MLFQ(id, remainingTime, arrivalTime, 0);
		tempArr[i] = tempProcess;

	}

	for (int i = totalProcessNum - 1; i > 0; i--) {
		for (int j = 0; j < i; j++) {
			if (atArr[j] > atArr[j + 1]) {
				temp = atArr[j];
				atArr[j] = atArr[j + 1];
				atArr[j + 1] = temp;
				temp = indexArr[j];
				indexArr[j] = indexArr[j + 1];
				indexArr[j + 1] = temp;
			}
		}
	}
	//버블정렬을 이용해 도착시간순 정

	for (int i = 0; i < totalProcessNum; i++) {
		unArrivedProcessQueue->push(tempArr[i]);
	}

	delete tempArr;
	delete indexArr;
	delete atArr;
}

void MLFQScheduler::runScheduler() {
	while (completedProcessNum != totalProcessNum) {
		mainAlgorithm();
		processArrive();
		curProcessRearrange(curWorkingProcess);
		//cout << "\n";
	}
}

void MLFQScheduler::processArrive() {
	int numOfNewProcess;
	numOfNewProcess = unArrivedProcessQueue->size();
	Process ** arrivedProcesses;
	arrivedProcesses = getNewProcesses();
	numOfNewProcess -= unArrivedProcessQueue->size();

	for (int i = 0; i < numOfNewProcess; i++) {
		multi_level_queue[0].push(arrivedProcesses[i]);
	}
	//도착한 프로세스를 최상단 level의 큐에 넣어준다.
	delete arrivedProcesses;
}

Process * MLFQScheduler::getNextProcess() {
	Process * temp = 0;
	for (int i = 0; i < numOfLevel; i++) {
		if (!(multi_level_queue[i].empty())) {
			temp = multi_level_queue[i].front();
			multi_level_queue[i].pop();
			break;
		}
	}
	//최상단큐에 가장먼저 들어온 process를 추
	return temp;
}

bool MLFQScheduler::onlyOneQueue() {
	int size = 0;
	for (int i = 0; i < numOfLevel; i++) {
		size += multi_level_queue[i].size();
	}
	if (size == 0) {
		return true;
	}
	else {
		return false;
	}
}

Process * MLFQScheduler::curProcessRearrange(Process * curWorking) {
	Process_MLFQ * tempProcessPointer = (Process_MLFQ *)curWorking;

	if (!curWorking)return NULL;
	if (tempProcessPointer->remainingTime > 0) {
		//수행시간이 남아있을
		if (onlyOneQueue()) {
			//단 하나의 프로세스만 있었다면 하위레벨로 내려가지 않는다
			tempProcessPointer->curHoldingTime = 0;
		}
		if (tempProcessPointer->curHoldingTime >= pow(2, tempProcessPointer->level)) {
			if (tempProcessPointer->level != 2)tempProcessPointer->level++;
			tempProcessPointer->curHoldingTime = 0;
		}
		multi_level_queue[tempProcessPointer->level].push((Process*)tempProcessPointer);
		return tempProcessPointer;
	}
	else {
		completedProcessNum++;
		return NULL;
	}
}

int MLFQScheduler::doWork(Process * workingProcess) {
	Process_MLFQ * tempProcess = (Process_MLFQ *)workingProcess;

	int timeOfWork = pow(2, tempProcess->level);
	//최대 2의 level승만큼을 수행하며
	if (timeOfWork > tempProcess->remainingTime) {
		//남아있는 시간이 더 작다면 그만큼만 수행한다
		timeOfWork = tempProcess->remainingTime;
	}

	for (int i = 0; i < timeOfWork; i++) {
		cout << tempProcess->identifier;
		tempProcess->remainingTime--;
		tempProcess->curHoldingTime++;
	}
	return timeOfWork;
}

//==========RM Scheduler============

RMScheduler::RMScheduler(int processNum, int _simulatingTime) {
	char id;
	int period;
	int timePerPeriod;
	int arrivalTime;
	Process * tempProcess;

	curWorkingNum = 0;
	simulatingTime = _simulatingTime;
	totalProcessNum = processNum;
	curWorkingArray = new Process_RM*[totalProcessNum];

	cout << "Create process list for RMScheduler...\n";
	cout << "please type following direction-->\n";
	cout << "processIdentifier period timePerPriod\n";
	cout << "ex) a 4 1\n";
	for (int i = 0; i < totalProcessNum; i++) {
		cout << i + 1 << " process : ";
		cin >> id >> period >> timePerPeriod;
		tempProcess = new Process_RM(id, period, timePerPeriod, 0);
		unArrivedProcessQueue->push(tempProcess);
	}
}

void RMScheduler::runScheduler() {
	Process_RM * errorProcess = 0;
	while (curTime <= simulatingTime) {
		mainAlgorithm();
		errorProcess = renewPeriod();
		if (errorProcess) {
			cout << "\none Process " << errorProcess->identifier << " failed to work in period\n";
			break;
		}
		processArrive();
		//cout << "\n";
	}
}

void RMScheduler::processArrive() {
	int numOfNewProcess;
	numOfNewProcess = unArrivedProcessQueue->size();
	Process ** arrivedProcesses;
	arrivedProcesses = getNewProcesses();
	numOfNewProcess -= unArrivedProcessQueue->size();

	for (int i = 0; i < numOfNewProcess; i++) {
		curWorkingArray[i] = (Process_RM*)arrivedProcesses[i];
	}
	delete arrivedProcesses;
	curWorkingNum += numOfNewProcess;
}

Process * RMScheduler::getNextProcess() {
	Process_RM * tempProcess = 0;
	Process_RM * minPeriodProcess = 0;
	int tempInt;
	for (int i = 0; i < curWorkingNum; i++) {
		tempProcess = curWorkingArray[i];
		if (tempProcess->remainingTime > 0) {
			//현재 주기에서 실행시간이 남아있으면서
			if (!minPeriodProcess)minPeriodProcess = tempProcess;
			tempInt = tempProcess->comparePeriod(minPeriodProcess);
			//가장 주기가 작은작업을 선정한다,만약 주기가 같다면 id를이용해 결정한다.
			switch (tempInt) {
			case -1:
				minPeriodProcess = tempProcess;
				break;
			case 0:
				if (minPeriodProcess->identifier > tempProcess->identifier)
					minPeriodProcess = tempProcess;
				break;
			}
		}
	}
	return minPeriodProcess;
}

int RMScheduler::doWork(Process * workingProcess) {
	Process_RM * tempProcess = (Process_RM *)workingProcess;
	cout << tempProcess->identifier;
	tempProcess->remainingTime--;
	return 1;
}

Process_RM * RMScheduler::renewPeriod() {
	Process_RM * tempProcess = 0;
	for (int i = 0; i < curWorkingNum; i++) {
		tempProcess = curWorkingArray[i];
		tempProcess->elapsedInPeriod++;
		//수행중인 모든 프로세스의 주기에서 머무른 시간을 증가시킨다
		if (tempProcess->elapsedInPeriod >= tempProcess->period) {
			if (tempProcess->remainingTime > 0)
				return tempProcess;
			//주기가 다되었음에도 시간이 남았다면, 이를 반환한다
			else {
				tempProcess->elapsedInPeriod = 0;
				tempProcess->remainingTime = tempProcess->timePerPeriod;
			}
			//그렇지않다면 주기에서 머무른시간과 주기내 남은 실행시간을초기화한다.
		}
	}
	return NULL;
}

//========================================================================



