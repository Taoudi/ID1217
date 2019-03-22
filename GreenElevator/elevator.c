#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include "hwAPI/hardwareAPI.h"

#define MAXFLOORS 6
#define MAXCABINS 5
#define FB 0
#define CB 1
#define POS 2

int totalCabins, totalFloors;

void *delegateFloor(void *);
void *delegateCabin(void *);
void *delegatePos(void *);
void *elevatorControl(void *);
void listener();
void enqueue();

typedef struct Event Event;

struct Event {
	Event *next;
	EventDesc *ed;
	EventType et;
};

Event *dequeue();
double cost(Event*,int);
double activeCost(int);
EventDesc *ED[MAXCABINS];

Event *listFB, *lastFB;
Event *listP, *lastP;
Event *listCB[MAXCABINS], *lastCB[MAXCABINS],*readyFB, *readyCB[MAXCABINS];

pthread_cond_t elevCond[MAXCABINS], condPosEv[MAXCABINS], condDoor[MAXCABINS],
condEvCB[MAXCABINS], condCab[MAXCABINS];
pthread_cond_t condEvFB, condEvP, busyElevCond;

pthread_mutex_t queueLockCB[MAXCABINS], elevLock[MAXCABINS], posLock[MAXCABINS],doorLock[MAXCABINS];
pthread_mutex_t typeFloorLock, busyElevLock, queueLockP, queueLockFB, RMILock;

int busyCab[MAXCABINS], posBool[MAXCABINS], door[MAXCABINS],
doorBool[MAXCABINS], done[MAXCABINS], delayedC[MAXCABINS], eType[MAXCABINS],interupted[MAXCABINS];
int busyElev, available;
double position[MAXCABINS];


int main(int argc, char *argv[]) {
	if (argc != 3) {
		fprintf(stderr, "Usage: %s Cabins Floors\n", argv[0]);
		fflush(stderr);
		exit(-1);
	}
	totalCabins = atoi(argv[1]);
	if (totalCabins > MAXCABINS) totalCabins = MAXCABINS;
	totalFloors = atoi(argv[2]);
	if (totalFloors > MAXFLOORS) totalFloors = MAXFLOORS;
	totalFloors += 1;
	/* Initialization code */
	int i;
	for(i = 0; i<totalCabins;i++){
		ED[i] = malloc(sizeof(Event));
		ED[i] = NULL;
		pthread_mutex_init(&elevLock[i], NULL);
		pthread_cond_init(&elevCond[i], NULL);

		pthread_mutex_init(&posLock[i], NULL);
		pthread_cond_init(&condPosEv[i], NULL);

		pthread_mutex_init(&doorLock[i], NULL);
		pthread_cond_init(&condDoor[i], NULL);

		pthread_mutex_init(&queueLockCB[i], NULL);
		pthread_cond_init(&condEvCB[i], NULL);

		pthread_cond_init(&condCab[i], NULL);
		pthread_mutex_init(&RMILock, NULL);

		posBool[i] = 0;
		busyCab[i] = 0;
		door[i] = 0;
		doorBool[i] = 0;
		done[i] = 0;
		eType[i] = -1;
		delayedC[i] = 0;
		position[i] = 0.0;
		interupted[i] = 0;
	}

	/*
	listFB = malloc(sizeof(Event)+sizeof(EventDesc));
	listP = malloc(sizeof(Event)+sizeof(EventDesc));
	lastFB = malloc(sizeof(Event)+sizeof(EventDesc));
	lastP = malloc(sizeof(Event)+sizeof(EventDesc));
	*/
	pthread_cond_init(&condEvFB, NULL);
	pthread_cond_init(&condEvP, NULL);

	pthread_cond_init(&busyElevCond, NULL);

	pthread_mutex_init(&queueLockFB, NULL);

	pthread_mutex_init(&queueLockP, NULL);
	pthread_mutex_init(&typeFloorLock, NULL);
	pthread_mutex_init(&busyElevLock, NULL);


	busyElev = 0;
	available = 1;

  	/* Setting up threads */
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

	pthread_t elevator[totalCabins], delegateF, delegateC[totalCabins], delegateP;
	long l;
	for (l=0; l<totalCabins; l++) {
		pthread_create(&elevator[l], &attr, elevatorControl, (void *) l);
		pthread_create(&delegateC[l], &attr, delegateCabin, (void *) l);

	}

	pthread_create(&delegateF, &attr, delegateFloor, (void *) 0);
	pthread_create(&delegateP, &attr, delegatePos, (void *) 0);

	initHW("localhost", 4711);

	listener();

	return 0;
}

Event *dequeueCB(int cabin) {
	Event *temp;
	if (listCB[cabin] == NULL)
		return NULL;

	if (listCB[cabin]==lastCB[cabin]) {
		temp = listCB[cabin];
		lastCB[cabin] = NULL;
		listCB[cabin] = NULL;
		temp->next = NULL;
		return temp;
	}

	temp = listCB[cabin];
	listCB[cabin] = listCB[cabin]->next;
	temp->next = NULL;
	return temp;
}
void printList(){
	Event *temp = listFB;
	while(temp!=NULL){
		printf("%d->\n", temp->ed->fbp.floor);
		temp=temp->next;
	}
	printf("done \n");
}

Event *unlinker(int type, Event* unlinked){
	Event* prev;
	Event* temp;
	switch(type){
		case FB:
		if(unlinked==NULL){
			return NULL;
		}
		if(listFB == unlinked){
			listFB= listFB->next;
			unlinked->next=NULL;
			return unlinked;
		}
		prev = NULL;
		temp = listFB;
		while(temp !=NULL){
			if(unlinked == temp){
				if(temp->next==NULL){
					lastFB = prev;
				}
				prev->next = prev->next->next;
				temp->next = NULL;
				return temp;
			}
			prev = temp;
			temp=temp->next;
		}
		break;

		int cabin = unlinked->ed->cbp.cabin;
		case CB:
		if(unlinked==NULL){
			return NULL;
		}
		if(listCB[cabin] == unlinked){
			listCB[cabin]= listCB[cabin]->next;
			unlinked->next=NULL;
			return unlinked;
		}
		 prev = NULL;
		temp = listCB[cabin];
		while(temp !=NULL){
			if(unlinked == temp){
				if(temp->next==NULL){
					listCB[cabin] = prev;
				}
				prev->next = temp->next;
				temp->next = NULL;
				return temp;
			}
			prev = temp;
			temp=temp->next;
		}
		break;
	}
	return NULL;
}



Event *dequeue(int type) {
	Event *temp;
	switch(type) {
		case FB:
		if(listFB == NULL)
			return NULL;


		if(listFB==lastFB){
			temp = listFB;
			lastFB=NULL;
			listFB=NULL;
			temp->next= NULL;
			return temp;
		}

		temp = listFB;
		listFB = listFB->next;
		temp->next = NULL;
		return temp;
		break;

		case POS:
		if(listP == NULL){
			return NULL;
		}

		if(listP==lastP){
			temp = listP;
			lastP=NULL;
			listP=NULL;
			temp->next= NULL;
			return temp;
		}

		temp = listP;
		listP = listP->next;
		temp->next = NULL;
		return temp;
		break;
	}
	return NULL;
}

void enqueue(Event *enqueued, int type){
	int foundSame = 0, i, cabin;

	switch(type) {
		case FB:
		if(lastFB==NULL){
			lastFB = enqueued;
			listFB = enqueued;
			enqueued->next = NULL;
		}
		else {
			for (Event *ev = listFB; ev != NULL; ev = ev->next) {
				if (ev->ed->fbp.floor == enqueued->ed->fbp.floor) {
					foundSame = 1;
					break;
				}
			}
			if (foundSame) {
			//	printf("FOUNDSAME\n");
				break;
			}
			lastFB->next = enqueued;
			lastFB = enqueued;
			enqueued->next = NULL;
		}
		break;

		case CB:
		cabin = (enqueued->ed->cbp.cabin) - 1;
		if (lastCB[cabin] == NULL) {
			lastCB[cabin] = enqueued;
			listCB[cabin] = enqueued;
		} else {
			for (Event *ev = listCB[cabin]; ev != NULL; ev = ev->next) {
				if (ev->ed->cbp.floor == enqueued->ed->cbp.floor) {
					foundSame = 1;
					break;
				}
			}
			lastCB[cabin]->next = enqueued;
			lastCB[cabin] = enqueued;
			enqueued->next = NULL;
		}
		break;

		case POS:
		if(lastP==NULL){
			lastP = enqueued;
			listP = enqueued;
			enqueued->next = NULL;
		}
		else {
			lastP->next = enqueued;
			lastP = enqueued;
			enqueued->next = NULL;
		}
		break;
	}
}
double cost( Event *checkedEvent, int cabin){
	int type =1;
	int activeEventFloor;
	int activeEventType;
	int checkedEventFloor;
	int checkedEventType;
	int activeEventDirection;
	int checkedEventDirection;

	switch(checkedEvent->et){
		case FB:
		checkedEventType=checkedEvent->ed->fbp.type;
		checkedEventFloor=checkedEvent->ed->fbp.floor;

		break;

		case CB:
		checkedEventFloor=checkedEvent->ed->cbp.floor;
		checkedEventType=0;
		break;

		case POS:
		break;
		case Speed:
		break;
		case Error:
		break;
	}

	if(ED[cabin]==NULL){
		double temp = (double) checkedEventFloor;
		return 1000*fabs(position[cabin]-temp);
	}

	switch(eType[cabin]){
		case FB:
		activeEventType= ED[cabin]->fbp.type;
		activeEventFloor= ED[cabin]->fbp.floor;
		if(ED[cabin]->fbp.floor < position[cabin]){
			activeEventDirection=-1;
		}
		else activeEventDirection=1;
		break;

		case CB:
		if( ED[cabin]->cbp.floor < position[cabin]){
			activeEventDirection=-1;
		}
		else activeEventDirection=1;
		activeEventFloor= ED[cabin]->cbp.floor;
		break;

	}

		double temp = (double) checkedEventFloor;
		if(checkedEventType != activeEventDirection && checkedEventType != 0){
			return 100000;
		}
		return 1000*fabs(position[cabin]-temp);
}

double activeCost(int cabin){
	if(ED[cabin]==NULL){
		return 200000;
	}
	int activeEventFloor = 0;
	if(eType[cabin]== FB){
		activeEventFloor=ED[cabin]->fbp.floor;
	}
	else if(eType[cabin] == CB){
		activeEventFloor=ED[cabin]->cbp.floor;
	}
	double temp = (double) activeEventFloor;
	return 1000*fabs(position[cabin] - temp);
}


Event *new() {
	/* Creating a new Event */
	Event *newEv;
	newEv = malloc(sizeof(Event));
	newEv->ed = malloc(sizeof(EventDesc));
	//newEv->et = malloc(sizeof(EventType));
	return newEv;
}

void moveElevator(int elevator, int bpFloor) {
	pthread_mutex_unlock(&doorLock[elevator]);
	double temp = (double) bpFloor;
	pthread_mutex_lock(&posLock[elevator]);

	while (fabs(temp-position[elevator]) > 0.04) {
		//printf("%f TEMP, %f POSITION\n", temp, ED[elevator]->cp.position);
		posBool[elevator] = 1;
		if (temp < position[elevator]) {
			pthread_mutex_lock(&RMILock);
			handleMotor(elevator+1, -1);
			pthread_mutex_unlock(&RMILock);
		}
		else {
			pthread_mutex_lock(&RMILock);
			handleMotor(elevator+1, 1);
			pthread_mutex_unlock(&RMILock);
		}

		pthread_cond_signal(&condPosEv[elevator]);
		pthread_mutex_unlock(&posLock[elevator]);

		pthread_mutex_lock(&posLock[elevator]);
	//	printf("TEMP = %f , ED[%d]->cp.position = %f , Floor = %d\n", temp, elevator,position[elevator], ED[elevator]->fbp.floor);
		while(posBool[elevator]) {
	//		printf("sover jag??\n");
			pthread_cond_wait(&condPosEv[elevator], &posLock[elevator]);
		}
		if(interupted[elevator]){
			break;
		}
	}
	pthread_mutex_unlock(&posLock[elevator]);
	pthread_mutex_lock(&RMILock);
	handleMotor(elevator+1, 0);
	pthread_mutex_unlock(&RMILock);

}

void *elevatorControl(void *id) {

	int elevator = (int) id;
	int floor;
	while(1) {

		pthread_mutex_lock(&elevLock[elevator]);
		while (!busyCab[elevator]) {
			printf("EC%d going to sleep\n", elevator);
			pthread_cond_wait(&elevCond[elevator], &elevLock[elevator]);
			printf("EC%d woke up! \n", elevator);
		}

		pthread_mutex_lock(&typeFloorLock);
		switch(eType[elevator]){
			case FB:
			floor = ED[elevator]->fbp.floor;
			break;

			case CB:
			floor = ED[elevator]->cbp.floor;
			break;
		}
		pthread_mutex_unlock(&typeFloorLock);

		//printf("FLOOR: %d\n", floor);


		pthread_mutex_lock(&posLock[elevator]);
		posBool[elevator] = 0;
		pthread_mutex_unlock(&posLock[elevator]);



		pthread_mutex_lock(&doorLock[elevator]);
		if (!door[elevator]) {
			doorBool[elevator]=0;
			moveElevator(elevator, floor);
		} else {
			doorBool[elevator]=0;
			pthread_mutex_lock(&RMILock);
			handleDoor(elevator+1, -1);
			pthread_mutex_unlock(&RMILock);
			pthread_mutex_unlock(&doorLock[elevator]);

			pthread_mutex_lock(&doorLock[elevator]);
			while(door[elevator]) {
				pthread_cond_wait(&condDoor[elevator], &doorLock[elevator]);
			}
			moveElevator(elevator, floor);
		}

		pthread_mutex_lock(&posLock[elevator]);
		if(interupted[elevator]){
			interupted[elevator]--;
			pthread_mutex_unlock(&posLock[elevator]);
			pthread_mutex_unlock(&elevLock[elevator]);
			continue;
		}
		pthread_mutex_unlock(&posLock[elevator]);

		pthread_mutex_lock(&RMILock);
		handleScale(elevator+1, floor);
		pthread_mutex_unlock(&RMILock);

		pthread_mutex_lock(&doorLock[elevator]);
		doorBool[elevator]=1;
		pthread_mutex_lock(&RMILock);
		handleDoor(elevator+1, 1);
		pthread_mutex_unlock(&RMILock);
		door[elevator]= 1;
		done[elevator] = 0;
		pthread_mutex_unlock(&doorLock[elevator]);


		while(!done[elevator]) {}
		pthread_mutex_lock(&busyElevLock);
		//printf("EC%d acquired busyElevLock, busyElev = %d\n", elevator, busyElev);
		if(!delayedC[elevator]) {
			available = 1;
		}
		else {
			available = 0;
		}
		pthread_cond_signal(&busyElevCond);
		pthread_cond_signal(&condCab[elevator]);

		busyElev--;
		busyCab[elevator] = 0; //Kanske sitter i fel plats
		ED[elevator] = NULL;
		pthread_mutex_unlock(&busyElevLock);
		pthread_mutex_unlock(&elevLock[elevator]);


	}

}

void *delegatePos(void *p) {
	while(1) {
		pthread_mutex_lock(&queueLockP);
		while (listP == NULL) {
			pthread_cond_wait(&condEvP, &queueLockP);
		}
		Event *recEv = dequeue(POS);
		//printf("[POS]: received event\n");
		pthread_mutex_unlock(&queueLockP);

		int cabin;

		cabin = (recEv->ed->cp.cabin) - 1;
		pthread_mutex_lock(&doorLock[cabin]);
		if(!doorBool[cabin]){
			if(!door[cabin]){
				pthread_mutex_lock(&posLock[cabin]);
				while(!posBool[cabin]) {
					pthread_cond_wait(&condPosEv[cabin], &posLock[cabin]);
				}
				posBool[cabin] = 0;

				pthread_mutex_lock(&typeFloorLock);
				if(ED[cabin]!=NULL)
					ED[cabin]->cp.position = recEv->ed->cp.position;
				//eType[cabin] = POS;
				position[cabin] = recEv->ed->cp.position;


				Event* minEvent;
				int found = 0;

				pthread_mutex_lock(&queueLockFB);
				double costMin = activeCost(cabin);
				int index = 0;
				if(listFB!=NULL){
					Event* temp = listFB;
					if(readyFB != NULL && cost(readyFB,cabin) < costMin){
						//printf("IM IN BOY FB %d\n", cabin);
						costMin= cost(readyFB,cabin);
						minEvent=readyFB;
						found++;
					}
					//printf("COST COMPARE %f < %f\n", cost(temp,cabin),costMin);
					while(temp!=NULL){
						index++;
					//	printf("FLOOR BUTTON COMPARE [%d]: COST IS SMALLER %f < %f\n", index,cost(temp,cabin),costMin);
						if(cost(temp,cabin) < costMin){
						//	printf("IM IN BOY FB %d\n", cabin);
							costMin= cost(temp,cabin);
							minEvent=temp;
							found++;
						}
						temp=temp->next;
					}
				}
				pthread_mutex_lock(&queueLockCB[cabin]);
				index=0;
				if(listCB[cabin]!=NULL){
					Event *temp = listCB[cabin];
					 if(readyCB[cabin]!=NULL && cost(readyCB[cabin],cabin) < costMin){
				//		printf("IM IN BOY CB %d\n", cabin);
						costMin= cost(readyCB[cabin],cabin);
						minEvent=temp;
						found++;
					}

					while(temp!=NULL){
						index++;
					//	printf("CABIN COMPARE[%d]: COST IS SMALLER %f < %f\n",index, cost(temp,cabin),costMin);
						if(cost(temp,cabin) < costMin){
					//		printf("IM IN BOY CB %d\n", cabin);
							costMin= cost(temp,cabin);
							minEvent=temp;
							found++;
						}
						temp=temp->next;
					}
				}
				if(found){
					Event *tempEv = new();
					switch(minEvent->et){
						case FB:
						tempEv->ed = ED[cabin];
						tempEv->et = eType[cabin];
						enqueue(tempEv,FB);
						printList();
						unlinker(FB, minEvent);
						printList();
						break;

						case CB:
						tempEv->ed = ED[cabin];
						tempEv->et = eType[cabin];
						enqueue(tempEv,CB);
						unlinker(CB, minEvent);
						break;
						default:
						break;

					}
					ED[cabin]=	minEvent->ed;
					eType[cabin] = minEvent->et;
					interupted[cabin]++;
				}
				pthread_mutex_unlock(&queueLockCB[cabin]);
				pthread_mutex_unlock(&queueLockFB);
				pthread_mutex_unlock(&typeFloorLock);

				//printf("----------------Cabin[%d]: UPDATING POS--------------------------\n", cabin);
				pthread_cond_signal(&condPosEv[cabin]);
				pthread_mutex_unlock(&posLock[cabin]);
			} else {
				door[cabin] = 0;
				pthread_cond_signal(&condDoor[cabin]);
			}
		} else
			done[cabin] = 1;
		pthread_mutex_unlock(&doorLock[cabin]);

	}
}

void *delegateCabin(void *elev) {
	int cabin = (int) elev;
	while(1) {
		pthread_mutex_lock(&queueLockCB[cabin]);
		while (listCB[cabin] == NULL) {
			pthread_cond_wait(&condEvCB[cabin], &queueLockCB[cabin]);
		}
		Event *recEv = dequeueCB(cabin);
		readyCB[cabin] = recEv;
		pthread_mutex_unlock(&queueLockCB[cabin]);

		pthread_mutex_lock(&busyElevLock);
		delayedC[cabin] = 1;
		while(busyCab[cabin]) {
			pthread_cond_wait(&condCab[cabin], &busyElevLock);
		}
		delayedC[cabin] = 0;
	//	printf("FB: %f, ACTIVE COST, %f QUEUED COST, %f current position\n", activeCost(cabin), cost(recEv,cabin), position[cabin]);

		busyElev++;
		if (busyElev == totalCabins)
			available = 0;
		pthread_mutex_unlock(&busyElevLock);

		pthread_mutex_lock(&elevLock[cabin]);

		pthread_mutex_lock(&typeFloorLock);
		ED[cabin] = recEv->ed;
		eType[cabin] = CB;
		pthread_mutex_unlock(&typeFloorLock);

		busyCab[cabin] = 1;
		pthread_cond_signal(&elevCond[cabin]);
		pthread_mutex_unlock(&elevLock[cabin]);
	}
	return NULL;
}

int closestElevator(int intfloor) {
	int turn;
	double floor = (double) intfloor;
	double currentMin = MAXFLOORS+10;
	int minTurn;
	pthread_mutex_lock(&typeFloorLock);
	for(turn = 0;turn < totalCabins; turn++){
		if(fabs(position[turn] - floor) < currentMin && !busyCab[turn]){
			minTurn = turn;
			currentMin = fabs(position[turn] - floor);
		}
	}
	pthread_mutex_unlock(&typeFloorLock);
	return minTurn;
}


void *delegateFloor(void *p) {
	while(1) {
		pthread_mutex_lock(&queueLockFB);
		while (listFB == NULL) {
			pthread_cond_wait(&condEvFB, &queueLockFB);
		}
		printf("dequeued\n");
		Event *recEv = listFB;
		//readyFB = recEv;
		pthread_mutex_unlock(&queueLockFB);
		int turn;

		pthread_mutex_lock(&busyElevLock);

		//printf("DelegateFloor acquired busyElevLock, busyElev = %d\n", busyElev);
		while (!available) {
			printf("DelegateFloor going to sleep\n");
			pthread_cond_wait(&busyElevCond, &busyElevLock);
		}
		pthread_mutex_lock(&queueLockFB);
		if(recEv == listFB){
			Event *recEv = dequeue(FB);
		}
		else{
			pthread_mutex_unlock(&queueLockFB);
			pthread_mutex_unlock(&busyElevLock);
			continue;

		}
		//readyFB = recEv;
		pthread_mutex_unlock(&queueLockFB);
		//printf("DelegateFloor UPDATE: busyElev = %d, totalCabins = %d\n", busyElev, totalCabins);
		turn = closestElevator(recEv->ed->fbp.floor);
	 	printf("FB: %f, ACTIVE COST, %f QUEUED COST, %f current position \n", activeCost(turn), cost(recEv,turn), position[turn]);

		busyElev++;
		if (busyElev == totalCabins) {
			available = 0;
		}
		pthread_mutex_unlock(&busyElevLock);
		//printf("DelegateFloor released busyElevLock\n");

		pthread_mutex_lock(&elevLock[turn]);

		pthread_mutex_lock(&typeFloorLock);
		ED[turn] = recEv->ed;
		eType[turn] = FB;
		pthread_mutex_unlock(&typeFloorLock);

		busyCab[turn] = 1;
		pthread_cond_signal(&elevCond[turn]);
		pthread_mutex_unlock(&elevLock[turn]);
	}
}

void listener() {
	EventType e;


	while(1) {
		Event *newEv = new();
		e = waitForEvent(newEv->ed);
		newEv->et = e;
		int foundSame = 0, i, cabin;
		switch(e) {
			case FloorButton:

			pthread_mutex_lock(&typeFloorLock);
			for (i = 0; i < totalCabins; i++) {
				if (ED[i] != NULL) {
					if (ED[i]->fbp.floor == newEv->ed->fbp.floor && ED[i]->fbp.type == newEv->ed->fbp.type) {
						pthread_mutex_unlock(&typeFloorLock);
						foundSame = 1;
						break;
					}
				}
			}
			pthread_mutex_unlock(&typeFloorLock);
			if (foundSame) break;

			pthread_mutex_lock(&queueLockFB);
			if (listFB == NULL) {
				pthread_cond_signal(&condEvFB);
			}
			//printf("Enqueue FB\n");
			enqueue(newEv, FB);
			pthread_mutex_unlock(&queueLockFB);
			break;

			case CabinButton:
			cabin = (newEv->ed->cbp.cabin) - 1;
		/*	pthread_mutex_lock(&typeFloorLock);
			for (i = 0; i < totalCabins; i++) {
				if (ED[i] != NULL) {
					if (ED[i]->cbp.floor == newEv->ed->cbp.floor ) {
						pthread_mutex_unlock(&typeFloorLock);
						foundSame = 1;
						break;
					}
				}
			}*/
			pthread_mutex_unlock(&typeFloorLock);
			if (foundSame) break;

			pthread_mutex_lock(&queueLockCB[cabin]);
			if (listCB[cabin] == NULL)
				pthread_cond_signal(&condEvCB[cabin]);
			enqueue(newEv, CB);
			pthread_mutex_unlock(&queueLockCB[cabin]);
			break;

			case Position:
			pthread_mutex_lock(&queueLockP);
			if (listP == NULL) {
				pthread_cond_signal(&condEvP);
			}
			enqueue(newEv, POS);
			//printf("Enqueue POS\n");
			pthread_mutex_unlock(&queueLockP);
			break;

			case Speed:

			break;

			case Error:

			break;
		}
	}

}
