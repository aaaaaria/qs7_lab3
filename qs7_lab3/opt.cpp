#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iterator>
#include <math.h>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>

using namespace std;

class ilocInstruction {
	public:
		int lineNum = 0;
		string opCode = "";
		string label = "";
		vector<string> sourceOp;
		vector<string> targetOp;	
};


vector<ilocInstruction> instructions;
unordered_map<string, int> labelMap;
vector<int> block_start;
vector<int> block_end;
string inputFile = "";


void parseIloc();
void buildCFG();
int find_max_label();
int find_max_register();
void helper();
void LVN();
void LU();
void Write2File(string argc);



int main(int argc, char* argv[]) {
	if (argc != 3 && argc != 4) {
        cout << "Error: please fill in 2 or 3 parameters" << endl;
        cout << "----------Choice 1----------" << endl;
        cout << "argv[0]: ./opt "<< endl;
        cout << "argv[1]: -v or -u" << endl;
        cout << "argv[2]: the name of the ILOC input file" << endl;
        cout << "----------Choice 2----------" << endl;
        cout << "argv[0]: ./opt " << endl;
        cout << "argv[1]: -v or -u" << endl;
        cout << "argv[2]: -u or -v" << endl;
        cout << "argv[3]: the name of the ILOC input file" << endl;
        return 0;
    }

	string argc1 = "";
	string argc2 = "";

	if (argc == 3) {
		argc1 = argv[1];
		inputFile = argv[2];
	} else if (argc == 4) {
		argc1 = argv[1];
		argc2 = argv[2];
		inputFile = argv[3];
	}

	if (argc1 == "-i" || argc2 == "-i") {
		cout << "-i: code motion not implemented" << endl;
		return -1;
	}

    parseIloc();
	buildCFG();

	if (argc == 3) {
		if (argc1 == "-v") {
			cout << "choose value numbering optimization" << endl;	
			LVN();		
		} else if (argc1 == "-u") {
			cout << "choose loop unrolling optimization" << endl;
			LU();
		} else {
			cout << "wrong arugments" << endl;
			return -1;
		}
	} else if (argc == 4) {
		if (argc1 == "-v" && argc2 == "-u") {
			cout << "choose value numbering and loop unrolling optimizations" << endl;
			LVN();		
			helper();
			buildCFG();
			LU();
		} else if (argc1 == "-u" && argc2 == "-v") {
			cout << "loop unrolling & value numbering" << endl;
			LU();
			helper();
			buildCFG();
			LVN();
		} else {
			cout << "Please input correct arugments" << endl;
			return -1;
		}
	}

	Write2File(argc1 + argc2);

	return 0;
}



bool op1(string opcode){
	bool isopcode1 = (opcode == "not" || opcode == "loadI" || opcode == "load" || opcode == "cload" || opcode == "store" || opcode == "cstore"
				|| opcode == "i2i" || opcode == "c2c" || opcode == "i2c" || opcode == "c2i");
	return isopcode1;
}

bool op2(string opcode){
	bool isopcode2 = (opcode == "storeAI" || opcode == "storeAO" || opcode == "cstoreAI" || opcode == "cstoreAO" || opcode == "cbr");
    return isopcode2;
}

bool op3(string opcode){
	bool isopcode3 = (opcode == "br" || opcode == "read" || opcode == "cread");
    return isopcode3;
}

bool op4(string opcode){
	bool isopcode4 = (opcode == "output" || opcode == "coutput" || opcode == "write" || opcode == "cwrite");
    return isopcode4;
}


void parseIloc() {
	ifstream infile;
	infile.open(inputFile);
	string line = "";
	int lineNum = 0;

	while (getline(infile, line)) {
		size_t idx;
		idx = line.find("//");
		if (idx != string::npos) {
			line = line.substr(0, idx);
		}

		lineNum += 1;
		ilocInstruction instruction;
		instruction.lineNum = lineNum;
		int lineLength = line.length();
		idx = line.find(":");

		if (idx == string::npos) {
			int i = 1;

			while (i < lineLength && isblank(line[i])) {
				i++;
			}

			string opcode = "";

			while (i < lineLength && !isblank(line[i])) {
				opcode += line[i];
				i++;
			}

			instruction.opCode = opcode;
			bool isopcode1 = op1(opcode);
			bool isopcode2 = op2(opcode);
			bool isopcode3 = op3(opcode);
			bool isopcode4 = op4(opcode);
			if (isopcode1) {
				
				while (i < lineLength && isblank(line[i])) {
					i++;
				}

				string leftopcode = "";

				while (i < lineLength && isalnum(line[i])) {
					leftopcode += line[i];
					i++;
				}

				while (i < lineLength && !isalnum(line[i])) {
					i++;
				}

				string rightopcode = "";

				while (i < lineLength && isalnum(line[i])) {
					rightopcode += line[i];
					i++;
				}

				instruction.sourceOp.push_back(leftopcode);
				instruction.targetOp.push_back(rightopcode);
				instructions.push_back(instruction);
			} else if (isopcode2) {
				
				while (i < lineLength && isblank(line[i])) {
					i++;
				}

				string leftopcode = "";

				while (i < lineLength && isalnum(line[i])) {
					leftopcode += line[i];
					i++;
				}

				while (i < lineLength && !isalnum(line[i])) {
					i++;
				}

				string rightopcode1 = "";

				while (i < lineLength && isalnum(line[i])) {
					rightopcode1 += line[i];
					i++;
				}

				while (i < lineLength && !isalnum(line[i])) {
					i++;
				}

				string rightopcode2 = "";

				while (i < lineLength && isalnum(line[i])) {
					rightopcode2 += line[i];
					i++;
				}

				instruction.sourceOp.push_back(leftopcode);
				instruction.targetOp.push_back(rightopcode1);
				instruction.targetOp.push_back(rightopcode2);
				instructions.push_back(instruction);

			} else if (isopcode3) {
				
				while (i < lineLength && !isalnum(line[i])) {
					i++;
				}

				string rightopcode = "";
				
				while (i < lineLength && isalnum(line[i])) {
					rightopcode += line[i];
					i++;
				}

				instruction.targetOp.push_back(rightopcode);
				instructions.push_back(instruction);
			} else if (isopcode4) {
				
				while (i < lineLength && isblank(line[i])) {
					i++;
				}

				string leftopcode = "";

				while (i < lineLength && isalnum(line[i])) {
					leftopcode += line[i];
					i++;
				}

				instruction.sourceOp.push_back(leftopcode);
				instructions.push_back(instruction);
			} else if (opcode == "halt") {
				instructions.push_back(instruction);
			} else { 

				while (i < lineLength && isblank(line[i])) {
					i++;
				}

				string leftopcode1 = "";

				while (i < lineLength && isalnum(line[i])) {
					leftopcode1 += line[i];
					i++;
				}

				while (i < lineLength && !isalnum(line[i])) {
					i++;
				}

				string leftopcode2 = "";

				while (i < lineLength && isalnum(line[i])) {
					leftopcode2 += line[i];
					i++;
				}

				while (i < lineLength && !isalnum(line[i])) {
					i++;
				}

				string rightopcode1 = "";

				while (i < lineLength && isalnum(line[i])) {
					rightopcode1 += line[i];
					i++;
				}

				instruction.sourceOp.push_back(leftopcode1);
				instruction.sourceOp.push_back(leftopcode2);
				instruction.targetOp.push_back(rightopcode1);
				instructions.push_back(instruction);
			}
		} else {
			instruction.label = line.substr(0, idx);
			instruction.opCode = "nop";
			instructions.push_back(instruction);
			labelMap[instruction.label] = instruction.lineNum;
		}
	}
}



void buildCFG() {
	block_start.push_back(1);
	unordered_set<int> block_startSet;
	block_startSet.insert(1);

	for (auto instruction : instructions) {
		string opcode = instruction.opCode;
		if (opcode == "br") {
			string label1 = instruction.targetOp[0];
			int lineNum1 = labelMap[label1];
			if (!block_startSet.count(lineNum1)) {
				block_start.push_back(lineNum1);
				block_startSet.insert(lineNum1);
			}
		} else if (opcode == "cbr") {
			string label1 = instruction.targetOp[0];
			int lineNum1 = labelMap[label1];
			if (!block_startSet.count(lineNum1)) {
				block_start.push_back(lineNum1);
				block_startSet.insert(lineNum1);
			}
			string label2 = instruction.targetOp[1];
			int lineNum2 = labelMap[label2];
			if (!block_startSet.count(lineNum2)) {
				block_start.push_back(lineNum2);
				block_startSet.insert(lineNum2);
			}
		}
	}

	int totalLineNum = instructions.size();

	for (auto l : block_start) {
		while (l <= totalLineNum && !block_startSet.count(l + 1)) {
			l++;
		}
		block_end.push_back(l);
	}
}


int find_max_label() {
	int labelMax = -1;
	for (auto instruction : instructions) {
		string irLabel = instruction.label;
		if (irLabel[0] == 'L') {
			int labelLength = irLabel.length();
			int lv = stoi(irLabel.substr(1, labelLength - 1));
			if (lv > labelMax) {
				labelMax = lv;
			}
		}
	}
	return labelMax;
}

int find_max_register() {
	int registerMax = -1;
	for (auto instruction : instructions) {
		if (instruction.sourceOp.size() > 0) {
			for (auto lo : instruction.sourceOp) {
				if (lo[0] != 'r') {
					continue;
				}
				int loLength = lo.length();
				int rv = stoi(lo.substr(1, loLength - 1));
				if (rv > registerMax) {
					registerMax = rv;
				}
			}
		}
		if (instruction.targetOp.size() > 0) {
			for (auto ro : instruction.targetOp) {
				if (ro[0] != 'r') {
					continue;
				}
				int roLength = ro.length();
				int rv = stoi(ro.substr(1, roLength - 1));
				if (rv > registerMax) {
					registerMax = rv;
				}
			}
		}
	}
	return registerMax;
}

void LVN() {
	int block_startNum = block_start.size();

	for (int i = 0; i < block_startNum; i++) {
		int begin = block_start[i] - 1;
		int end = block_end[i] - 1;
		unordered_map <string, int> equ_vn;
		unordered_map <string, int> operand_vn;
		unordered_map <int, string> vn_operand;
		int vn = 0;

		for (int j = begin; j <= end; j++) {
			string opcode = instructions[j].opCode;
			if (opcode == "halt") {
				break;
			}
			bool isopcode = (opcode == "nop" || opcode == "not" || opcode == "i2i" || opcode == "c2c" || opcode == "i2c" || opcode == "c2i" || opcode == "br" || opcode == "cbr"
				|| opcode == "read" || opcode == "cread" || opcode == "output" || opcode == "coutput" || opcode == "write" || opcode == "cwrite"
				|| opcode == "loadI" || opcode == "load" || opcode == "cload"
				|| opcode == "store" || opcode == "storeAI" || opcode == "storeAO" || opcode == "cstore" || opcode == "cstoreAI" || opcode == "cstoreAO");
			if (isopcode) {
				continue;
			} else {
				string l0 = instructions[j].sourceOp[0];
				string l1 = instructions[j].sourceOp[1];
				string r0 = instructions[j].targetOp[0];
				if (opcode == "multI") {
					int c1 = stoi(l1);
					if ((c1 & (c1 - 1)) == 0) {
						int shift = sqrt(c1);
						ilocInstruction instruction;
						instruction.opCode = "lshiftI";
						instruction.sourceOp.push_back(l0);
						instruction.sourceOp.push_back(to_string(shift));
						instruction.targetOp.push_back(r0);
						instructions[j] = instruction;
					}
				}
				if (operand_vn.find(l0) == operand_vn.end()) {
					operand_vn[l0] = vn;
					vn += 1;
				}
				if (operand_vn.find(l1) == operand_vn.end()) {
					operand_vn[l1] = vn;
					vn += 1;
				}
				string opcodeLeft = to_string(operand_vn[l0]);
				string opcodeRight = to_string(operand_vn[l1]);
				string equ = opcodeLeft + opcode + opcodeRight;
				if (equ_vn.count(equ)) {
					int equVn = equ_vn[equ];
					string vnopcode = vn_operand[equVn];
					if (operand_vn[vnopcode] == equVn) {
						ilocInstruction instruction;
						instruction.opCode = "i2i";
						instruction.sourceOp.push_back(vnopcode);
						instruction.targetOp.push_back(r0);
						instructions[j] = instruction;
					}
					operand_vn[r0] = equVn;
				} else {
					equ_vn[equ] = vn;
					operand_vn[r0] = vn;
					vn_operand[vn] = r0;
					vn++;
				}
			}
		}

	}

}

void LU() {
	sort(block_start.begin(), block_start.end());
	sort(block_end.begin(), block_end.end());
	vector<ilocInstruction> instructionsNew;
	int labelMax = find_max_label();
	int registerMax = find_max_register();
	int block_startNum = block_start.size();

	for (int i = 0; i < block_startNum; i++) {
		int begin = block_start[i] - 1;
		int end = block_end[i] - 1;
		if (instructions[end - 1].opCode == "halt") {

			for (int j = begin; j <= end - 1; j++) {
				instructionsNew.push_back(instructions[j]);
			}

			instructions.swap(instructionsNew);
			continue;
		}
		if (instructions[begin].label != "" && instructions[end].opCode == "cbr") {
			if (instructions[begin].label == instructions[end].targetOp[0]) {
				if (instructions[end - 2].opCode == "addI") {
					ilocInstruction instruction0;
					instruction0.label = instructions[begin].label;
					instruction0.opCode = "nop";
					instructionsNew.push_back(instruction0);

					ilocInstruction instruction1;
					instruction1.opCode = "sub";
					instruction1.sourceOp.push_back(instructions[end - 1].sourceOp[1]);
					instruction1.sourceOp.push_back(instructions[end - 1].sourceOp[0]);
					string registerNew = "r" + to_string(++registerMax);
					instruction1.targetOp.push_back(registerNew);
					instructionsNew.push_back(instruction1);

					ilocInstruction instruction2;
					instruction2.opCode = "rshiftI";
					instruction2.sourceOp.push_back(registerNew);
					instruction2.sourceOp.push_back("2");
					instruction2.targetOp.push_back(registerNew);
					instructionsNew.push_back(instruction2);

					ilocInstruction instruction3;
					instruction3.opCode = "lshiftI";
					instruction3.sourceOp.push_back(registerNew);
					instruction3.sourceOp.push_back("2");
					instruction3.targetOp.push_back(registerNew);
					instructionsNew.push_back(instruction3);

					ilocInstruction instruction4;
					instruction4.opCode = "add";
					instruction4.sourceOp.push_back(instructions[end - 1].sourceOp[0]);
					instruction4.sourceOp.push_back(registerNew);
					registerNew = "r" + to_string(++registerMax);
					instruction4.targetOp.push_back(registerNew);
					instructionsNew.push_back(instruction4);

					ilocInstruction instruction5;
					instruction5.opCode = "cbr";
					instruction5.sourceOp.push_back(registerNew);
					string labelNew1 = "L" + to_string(++labelMax);
					instruction5.targetOp.push_back(labelNew1);
					string labelNew2 = "L" + to_string(++labelMax);
					instruction5.targetOp.push_back(labelNew2);
					instructionsNew.push_back(instruction5);

					ilocInstruction instruction6;
					instruction6.opCode = "nop";
					instruction6.label = labelNew1;
					instructionsNew.push_back(instruction6);

					for (int n = 0; n < 4; n++) {
						for (int j = begin + 1; j <= end - 2; j++) {
							instructionsNew.push_back(instructions[j]);
						}
					}

					ilocInstruction instruction7;
					if (instructions[end - 1].opCode == "cmp_LE") {
						instruction7.opCode = "cmp_LT";
					}
					else if (instructions[end - 1].opCode == "cmp_LT") {
						instruction7.opCode = "cmp_LE";
					}
					instruction7.sourceOp.push_back(instructions[end - 1].sourceOp[0]);
					instruction7.sourceOp.push_back(registerNew);
					instruction7.targetOp.push_back(instructions[end].sourceOp[0]);
					instructionsNew.push_back(instruction7);

					ilocInstruction instruction8;
					instruction8.opCode = "cbr";
					instruction8.sourceOp.push_back(instructions[end].sourceOp[0]);
					instruction8.targetOp.push_back(labelNew1);
					instruction8.targetOp.push_back(labelNew2);
					instructionsNew.push_back(instruction8);

					ilocInstruction instruction9;
					instruction9.label = labelNew2;
					instruction9.opCode = "nop";
					instructionsNew.push_back(instruction9);

					instructionsNew.push_back(instructions[end - 1]);

					ilocInstruction instruction10;
					instruction10.opCode = "cbr";
					instruction10.sourceOp.push_back(instructions[end].sourceOp[0]);
					string labelNew3 = "L" + to_string(++labelMax);
					instruction10.targetOp.push_back(labelNew3);
					instruction10.targetOp.push_back(instructions[end].targetOp[1]);
					instructionsNew.push_back(instruction10);

					ilocInstruction instruction11;
					instruction11.label = labelNew3;
					instruction11.opCode = "nop";
					instructionsNew.push_back(instruction11);

					for (int j = begin + 1; j <= end - 2; j++) {
						instructionsNew.push_back(instructions[j]);
					}

					instructionsNew.push_back(instructions[end - 1]);
					instructionsNew.push_back(instruction10);
				}
			} else {

				for (int j = begin; j <= end; j++) {
					instructionsNew.push_back(instructions[j]);
				}

				continue;
			}
		} else {

			for (int j = begin; j <= end; j++) {
				instructionsNew.push_back(instructions[j]);
			}

			continue;
		}
	}
}

void Write2File(string argc) {
	for (auto instruction : instructions) {
		string outputFile = "opt" + argc + "-" + inputFile;
		ofstream outfile;
		outfile.open(outputFile, ios::app);
		string opcode = instruction.opCode;
		bool isopcode1 = (opcode == "not" || opcode == "loadI" || opcode == "load" || opcode == "cload" || opcode == "store" || opcode == "cstore"
			|| opcode == "i2i" || opcode == "c2c" || opcode == "i2c" || opcode == "c2i");
		bool isopcode2 = (opcode == "storeAI" || opcode == "storeAO" || opcode == "cstoreAI" || opcode == "cstoreAO");
		bool isopcode3 = (opcode == "read" || opcode == "cread");
		bool isopcode4 = (opcode == "output" || opcode == "coutput" || opcode == "write" || opcode == "cwrite");

		if (opcode == "nop") {
			outfile << instruction.label << ": " << opcode << endl;
		} else if (isopcode1) {
			outfile << opcode << " " << instruction.sourceOp[0] << " => " << instruction.targetOp[0] << endl;
		} else if (isopcode2) {
			outfile << opcode << " " << instruction.sourceOp[0] << " => " << instruction.targetOp[0] << ", " << instruction.targetOp[1] << endl;
		} else if (isopcode3) {
			outfile << opcode << " => " << instruction.targetOp[0] << endl;
		} else if (isopcode4) {
			outfile << opcode << " " << instruction.sourceOp[0] << endl;
		} else if (opcode == "br") {
			outfile << opcode << " -> " << instruction.targetOp[0] << endl;
		} else if (opcode == "cbr") {
			outfile << opcode << " " << instruction.sourceOp[0] << " -> " << instruction.targetOp[0] << ", " << instruction.targetOp[1] << endl;
		} else if (opcode == "halt") {
			outfile << opcode << endl;
		} else if (opcode == "") {
			outfile << opcode << endl;
		} else {
			outfile << opcode << " " << instruction.sourceOp[0] << ", " << instruction.sourceOp[1] << " => " << instruction.targetOp[0] << endl;
		}
		outfile.close();
	}

}

void helper() {
	int lineNum = 1;
	for (auto instruction : instructions) {
		instruction.lineNum = lineNum;
		string irLabel = instruction.label;
		if (irLabel != "") {
			labelMap[irLabel] = instruction.lineNum;
		}
		lineNum++;
	}
}
