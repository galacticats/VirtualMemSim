#include <string>
#include <iostream>
#include <fstream>
#include <istream>
#include <sstream>
#include <vector>
#include <iterator>
#include "physMem.h"
#include "TLB.h"

int main(int argc, char* argv[]) {
	//all of these files should be in the USB
	if (argc != 5) {
		std::cout << "Requires 4 extra inputs: init file, VA file, output file, 'T' or 'F' for TLB" << std::endl;
		std::cout << "Ex: program input1.txt input2.txt output.txt T" << std::endl;
		return 0;
	}
	std::string arg1 = argv[1];
	std::string arg2 = argv[2];
	std::string arg3 = argv[3];
	std::string arg4 = argv[4];
	std::string PMInitFile = arg1;//"E:\\CS143B\\sample-input.txt";
	std::string VAInputFile = arg2;//"E:\\CS143B\\sample-input2.txt";
	std::string outputFile = arg3;//"E:\\CS143B\\384786441.txt"; //change to "384786442.txt" for TLB
	bool useTLB = false;
	if (arg4 == "T") { useTLB = true; }
	try {
		std::ifstream PMFile;
		std::ifstream VAFile;
		std::ofstream oFile;
		PMFile.open(PMInitFile);
		VAFile.open(VAInputFile);
		oFile.open(outputFile);
		if (PMFile.is_open() && VAFile.is_open() && oFile.is_open()) {
			std::cout << "Files are open" << std::endl;
			//start off by reading PMFile, which initializes the Physical memory
			std::string line;
			std::vector<std::string> commands;
			physMem* pm = new physMem();
			TLB* TLBuff = new TLB();
			getline(PMFile, line);
			std::cout << "InitFile Line 1: " << line << std::endl;
			//INIT FILE LINE 1 IS SERIES OF PAIRS, (si fi)-------------------------------------------------------
			//si is the segment number, fi is the address that the segment's page table starts at
			//an fi value of -1 means that the segment's PT is not resident
			std::stringstream buf(line);
			std::string word;
			std::vector<std::string> tokens;
			while (getline(buf, word, ' ')) { //split line 1 up by spaces
				tokens.push_back(word);
			}
			if (tokens.size() % 2 == 0) { //if they come in pairs
				for (int i = 0; i < tokens.size() / 2; i++) {
					pm->addPT(stoi(tokens[i * 2]), stoi(tokens[(i * 2) + 1]));
				}
			}
			else {
				std::cout << "Line 1 of init file is incorrectly formatted." << std::endl;
				PMFile.close();
				VAFile.close();
				oFile.close();
				return 0;
			}
			//INIT FILE LINE 2 IS A SERIES OF TRIPLETS (pj sj fj)------------------------------------------------
			//pj = page number, sj = segment that the page belongs to, fj = address of the page pj of segment sj
			//if fj = -1, then the page is not resident in physical memory
			getline(PMFile, line);
			std::cout << "InitFile Line 2: " << line << std::endl;
			std::stringstream buf2(line);
			tokens.clear();
			while (getline(buf2, word, ' ')) {
				tokens.push_back(word);
			}
			if (tokens.size() % 3 == 0) { //if they come in triples
				for (int i = 0; i < tokens.size() / 3; i++) {
					pm->addPage(stoi(tokens[i * 3]), stoi(tokens[(i * 3) + 1]), stoi(tokens[(i*3) + 2]));
				}
			}
			else {
				std::cout << "Line 2 of init file is incorrectly formatted." << std::endl;
				PMFile.close();
				VAFile.close();
				oFile.close();
				return 0;
			}
			getline(VAFile, line);
			//ONE LINE VA FILE contains PAIRS of the format (oi VAi)-----------------------------------------
			//oi is 0 for reads, 1 for writes
			//VAi is the virtual address (s (9), p (10), w (9)) that must be translated
			std::stringstream buf3(line);
			std::cout << "VAFile line: " << line << std::endl;
			tokens.clear();
			while (getline(buf3, word, ' ')) {
				tokens.push_back(word);
			}
			int PA;
			if (tokens.size() % 2 == 0) { //if they come in pairs
				for (int i = 0; i < tokens.size() / 2; i++) {
					//break VAi into s, p, and w
					int VA = stoi(tokens[(i * 2) + 1]);
					std::cout << std::endl << "VA: " << VA << std::endl;
					int VAs, VAp, VAw;
					//W is the rightmost 9 bits -> mask out the left 23 bits
					int wsMask = 0b00000000000000000000000111111111;
					VAw = VA & wsMask;
					std::cout << "VAw: " << VAw << std::endl;
					VA = VA >> 9;
					int pMask = 0b00000000000000000000001111111111;
					VAp = VA & pMask;
					std::cout << "VAp: " << VAp << std::endl;
					VA = VA >> 10;
					VAs = VA & wsMask;
					std::cout << "VAs: " << VAs << std::endl;
					int sp = (VAs * 10) + VAp;
					//TLB translations
					bool foundInTLB = false;
					if (useTLB) {
						TLBuff->printTLB(); //debug
						std::cout << "Searching sp (" << sp << ") in TLB" << std::endl;
						int f = TLBuff->search(sp);
						if (f != -1) {
							foundInTLB = true;
							oFile << "h ";
							PA = f + VAw;
							std::cout << "TLB hit: " << PA << std::endl;
						}
						else {
							std::cout << "TLB miss" << std::endl;
							oFile << "m ";
						}
					}
					//OUTPUT: the physical addresses of each VA into file oFile
					//SINGLE LINE containing INTEGERS REPRESENTING PA, "pf", or "err", separated b blanks
					//For the TLB version, file contains the above, but each preceded by "m" or "h" for TLB misses and hits
					if (!useTLB || (useTLB && !foundInTLB)) {
						PA = pm->getPA(stoi(tokens[i*2]), VAs, VAp, VAw);
						std::cout << "Phys memory : " << PA << std::endl;
					}
					if (PA == -1) { //page fault "pf"
						std::cout << "Page Fault" << std::endl;
						oFile << "pf ";
					}
					else if (PA == -2) { //"error"
						std::cout << "Error" << std::endl;
						oFile << "err ";
					}
					else {
						//if TLB is being used and !foundInTLB update it
						if (useTLB && !foundInTLB) {
							std::cout << "Updating TLB" << std::endl;
							TLBuff->add(sp, pm->getSP(VAs, VAp));
						}
						oFile << PA << " ";
					}
				}
				delete pm;
				delete TLBuff;
			}
			else {
				std::cout << "VA file is incorrectly formatted." << std::endl;
				PMFile.close();
				VAFile.close();
				oFile.close();
				return 0;
			}
		}
		else {
			std::cout << "File(s) could not be opened." << std::endl;
		}
		std::cout << "Flushing and closing files." << std::endl;
		oFile.flush();
		PMFile.close();
		VAFile.close();
		oFile.close();
		std::cout << "Done" << std::endl;
	}
	catch (...){
		std::cout << "Exception opening/reading/closing file" << std::endl;
	}
	return 0;
}