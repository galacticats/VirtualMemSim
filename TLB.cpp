#include "TLB.h"



TLB::TLB()
{
	hasEmpty = true;
}


TLB::~TLB()
{
}

int TLB::search(int sp) {
	for (int i = 0; i < 4; i++) {
		if (TLBuff[i].sp == sp) {
			int oldLRU = TLBuff[i].LRU;
			for (int j = 0; j < 4; j++) { //decrement all LRU values greater than oldLRU by 1
				if (TLBuff[j].LRU > oldLRU) {
					TLBuff[j].LRU = TLBuff[j].LRU - 1;
				}
			}
			TLBuff[i].LRU = 3;
			return TLBuff[i].f; //TLB hit -> f is the frame number -> 
		}
	}
	//TLB miss
	return -1;
}

void TLB::add(int newSP, int newF) {
	if (hasEmpty) {
		bool noEmpty = true;
		for (int i = 0; i < 4 && noEmpty; i++) {//fill in any "empty" spots first
			if (TLBuff[i].LRU == -1) {
				TLBuff[i].LRU = 3;
				TLBuff[i].sp = newSP;
				TLBuff[i].f = newF;
				noEmpty = false;
			}
			else { //decrement the LRU values by 1
				TLBuff[i].LRU = TLBuff[i].LRU - 1;
			}
		}
	}
	if (!hasEmpty) {
		bool written = false;
		for (int i = 0; i < 4; i++) {
			if (TLBuff[i].LRU == 0) {
				TLBuff[i].LRU = 3;
				TLBuff[i].sp = newSP;
				TLBuff[i].f = newF;
			}
			else { //decrement the LRU values by 1
				TLBuff[i].LRU = TLBuff[i].LRU - 1;
			}
		}
	}
	bool foundEmpty = false;
	for (int j = 0; j < 4; j++) {
		if (TLBuff[j].LRU == -1) {
			foundEmpty = true;
		}
	}
	if (!foundEmpty) {
		std::cout << "No more empty" << std::endl;
		hasEmpty = false;
	}
}

void TLB::printTLB() {
	for (int i = 0; i < 4; i++) {
		std::cout << "Entry " << i << "---> LRU: " << TLBuff[i].LRU << ", SP: " << TLBuff[i].sp << ", F: " << TLBuff[i].f << std::endl;
	}
}

TLB::TLBEntry::TLBEntry() {
	sp = -1;
	f = -1;
	LRU = -1;
}

TLB::TLBEntry::~TLBEntry() {
}