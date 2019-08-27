#include "physMem.h"



physMem::physMem()
{
	PM[524288] = { 0 };
	bitmap[0] = true; //ST is always frame 0
	for (int i = 1; i < 1024; i++) {
		bitmap[i] = false;
	}
}


physMem::~physMem()
{
}

int physMem::getPA(int mode, int s, int p, int w) {
	//translate s, p, and w into a physical address
	std::cout << "getPA(" << mode << "," << s << "," << p << "," << w << ")" << std::endl;
	std::cout << "PM[" << s << "]: " << PM[s] << std::endl;
	if (PM[s] == -1) {
		std::cout << "PM[s] is 1. Page fault" << std::endl;
		return -1; //this is a page fault. output "pf"
	}
	else if (PM[s] == 0) {
		if (mode == 0) { //read
			std::cout << "PM[s] is 0. Error" << std::endl;
			return -2; //this is an error. output "error"
		}
		else { //write - create a blank PT
			std::cout << "PM[s] is 0. Creating new page table" << std::endl;
			int newPT = findBMSpace(2);
			addPT(s, newPT);
			//also have to create a blank page
			int newPage = findBMSpace(1);
			addPage(p, s, newPage);
			return newPage + w;
		}
	}
	else {
		std::cout << "PM[PM[" << s << "]+" << p << "]: " << PM[PM[s] + p] << std::endl;
		if (PM[PM[s] + p] == -1) {
			std::cout << "PM[PM[s]+p] is 1. Page fault" << std::endl;
			return -1;
		}
		else if (PM[PM[s] + p] == 0) {
			if (mode == 0) { //read
				std::cout << "PM[PM[s]+p] is 0. Error" << std::endl;
				return -2;
			}
			else { //write - create a blank page
				std::cout << "PM[PM[s]+p] is 0. Creating new page" << std::endl;
				int newPage = findBMSpace(1);
				addPage(p, s, newPage);
				return newPage + w;
			}
		}
		else {
			std::cout << "PM[PM[" << s << "]+" << p << "]+" << w << ": " << PM[PM[s] + p] + w << std::endl;
			return PM[PM[s] + p] + w; //the PHYSICAL ADDRESS. Not what's stored there.
		}
	}
}

int physMem::getSP(int s, int p) {
	return PM[PM[s] + p];
}

int physMem::addPT(int segNum, int ptAddr) {
	std::cout << "addPT(" << segNum << "," << ptAddr << ")" << std::endl;
	PM[segNum] = ptAddr;
	if (ptAddr != 0 && ptAddr != -1) {
		int bitmapIndex = ptAddr / 512; //PTs are 2 frames long
		std::cout << "bitmapIndex: " << bitmapIndex << std::endl;
		bitmap[bitmapIndex] = true;
		bitmap[bitmapIndex + 1] = true;
	}
	std::cout << "PM[" << segNum << "]: " << PM[segNum] << std::endl;
	return PM[segNum];
}

int physMem::addPage(int pgNum, int segNum, int pgAddr) {
	std::cout << "addPage(" << pgNum << "," << segNum << "," << pgAddr << ")" << std::endl;
	PM[PM[segNum] + pgNum] = pgAddr;
	if (pgAddr != -1 && pgAddr != 0) {
		int bitmapIndex = pgAddr / 512; //pages are only 1 frame long
		std::cout << "bitmapIndex: " << bitmapIndex << std::endl;
		bitmap[bitmapIndex] = true;
	}
	std::cout << "PM[PM[" << segNum << "] + " << pgNum << "]: " << PM[PM[segNum] + pgNum] << std::endl;
	return PM[PM[segNum] + pgNum];
}


int physMem::findBMSpace(int len) {
	//returns starting address of the section of the bitmap that has room for frames of len "len"
	int emptyChain = 0;
	int startFrame;
	for (int i = 0; i < 1024 && emptyChain < len; i++) {
		//std::cout << i << ": " << bitmap[i] << std::endl;
		if (bitmap[i] == false) {
			//std::cout << "Free frame at: " << i << std::endl;
			if (emptyChain == 0) {
				startFrame = i;
			}
			emptyChain++;
			//std::cout << emptyChain << "in a row" << std::endl;
		}
		else {
			emptyChain = 0;
		}
	}
	if (emptyChain == len) {
		std::cout << "Free frame starts at: " << (startFrame * 512) << std::endl;
		return startFrame * 512; //return the actual starting PA of the new page or PT
	}
	else { //no empty space of size "len" was found
		return -1; //failure
	}
}