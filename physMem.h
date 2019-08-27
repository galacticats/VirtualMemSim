#pragma once
#include <vector>
#include <iostream>

class physMem
{
public:
	physMem();
	~physMem();

	int getPA(int mode, int s, int p, int w);
	int addPT(int segNum, int ptAddr);
	int addPage(int pgNum, int segNum, int pgAddr);;
	int getSP(int s, int p);
	int findBMSpace(int len);

private:
	int PM[524288] = { 0 }; //physical memory is an array of 1024 frames (each is 512 words/ints)
	bool bitmap[1024]; //one bit/bool per frame
};

