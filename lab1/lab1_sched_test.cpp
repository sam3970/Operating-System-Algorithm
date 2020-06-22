/*
*	DKU Operating System Lab
*	    Lab1 (Scheduler Algorithm Simulator)
*	    Student id : 32131706,32147356
*	    Student name : 백준하,이상규
*
*   lab1_sched.c :
*       - Lab1 source file.
*       - Must contains scueduler algorithm test code.
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

int main(int argc, char *argv[]){

//=====================FIFO,RR==================================
	srand((unsigned)time(NULL)); //시드함수

	createProcess(); //랜덤하게 프로세스를 생성하는 함수
	printProcess(); //프로세스의 정보를 출력할 함수

	/* 스케쥴 프로세스는 tat,awt 그리고 간트차트를 출력합니다. */
	FCFS();
	Roundrobin();
	//system("pause");
//=============================================================

//========================MLFQ,RM===============================
	MLFQScheduler *mysched;
	RMScheduler *myRM;
	int processNum=5;
	int simulatingTime = 30;




	for (int i = 0; i < 5; i++) {
		cout << "Please type number of processes for MLFQ(ex 5) : ";
		cin >> processNum;
		mysched = new MLFQScheduler(processNum);
		mysched->runScheduler();
		cout << "\n\n";
		delete mysched;
	}

	for (int i = 0; i < 5; i++) {
		cout << "Please type number of processes and simulTime for RM(ex 3 30) : ";
		cin >> processNum>>simulatingTime;
		myRM=new RMScheduler(processNum, simulatingTime);
		myRM->runScheduler();
		cout << "\n\n";
		delete myRM;
	}
	return 0;
	//=========================================================
}

