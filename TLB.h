#pragma once
#include <iostream>
class TLB
{
public:
	TLB();
	~TLB();

	int search(int sp);
	void add(int sp, int f);
	void printTLB();

private:
	class TLBEntry {
	public:
		TLBEntry();
		~TLBEntry();
		int LRU; //0-3, 0 is least recently accessed
		int sp;
		int f; //address
	};

	TLB::TLBEntry TLBuff[4];
	bool hasEmpty;
};

