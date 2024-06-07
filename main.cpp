#include<bits/stdc++.h>

using namespace std;

int ICache[256], DCache[256], RF[16]; /* */
int PC = 0;

bool reg_valid[16]; /* checks if the Data in register is correct */

int num_instr = 0, num_cycle = 0;
int num_arith = 0, num_logic = 0, num_shift = 0, num_mem = 0, num_control = 0, num_halt = 0, num_stall = 0, num_data_stall = 0, num_control_stall = 0;
int numLoadImm = 0;
bool fetch = true;
int LMD;
int stall = 0;
bool halt = false;

int hex_to_dec(int & x) // Converts HEX to DEC
{
    if (x & (1 << 7)){x = x - (1 << 8);}

    return x;
}
int dec_to_hex(int & x) {
    if (x < 0) {
        x = x + (1 << 8);
    }
    return x;
}
class Instruction {

    public: int stalled = 0;
    int arg[4] = {0,0,0,0};
    int stage = 0;
    int A,B,ALUOutput;
    int IR = 0;
    void get_args(int a[]) {
        a[0] = (IR & (15 * (1 << 12))) / (1 << 12); // opcode
        a[1] = (IR & (15 * (1 << 8))) / (1 << 8); // Rd
        a[2] = (IR & (15 * (1 << 4))) / (1 << 4); // Rs
        a[3] = (IR & (15)); // Rt
    }

    void get_reg_val(int &R, int index) {
        R = hex_to_dec(RF[index]);
    }

    void IF() {
        // first  bits is opcode and last 8 bits source regs
        if (halt) {
            return;
        } // if halt then no new instruction 
        if (stall) {
            // if control hazard then wait 
            stall--;
            return;
        }
        IR = ICache[PC];
        IR = (IR << 8) + ICache[PC + 1];
        PC = PC + 2;
        stage = 1;
        
        A = 0; B = 0;
    }

    void ID() {
        if (halt) {
            return;
        }
        get_args(arg);

        // Check if the Registers are ready to use 
        switch (arg[0])
        {
            // ALU operations Rd, Rs, Rt
            case 0:
            case 1:
            case 2:
            case 4:
            case 5:
            case 6:
                if (!reg_valid[arg[2]] || !reg_valid[arg[3]]) {
                    num_stall++;
                    num_data_stall++;
                    stalled = 1;
                    return;
                }
                stalled = 0;
                num_arith++;
                get_reg_val(A, arg[2]);
                get_reg_val(B, arg[3]);
                reg_valid[arg[1]] = 0;
                break;

                // INC
            case 3:
                if (!reg_valid[arg[1]]) {
                    num_stall++;
                    num_data_stall++;
                    stalled = 1;
                    return;
                }
                stalled = 0;
                num_arith++;
                get_reg_val(A, arg[1]);
                B = 0;
                reg_valid[arg[1]] = 0;
                break;

                // NOT
            case 7:
                if (!reg_valid[arg[2]]) {
                    num_stall++;
                    num_data_stall++;
                    stalled = 1;
                    return;
                }
                stalled = 0;
                num_logic++;
                get_reg_val(A, arg[2]);
                B = 0;
                reg_valid[arg[1]] = 0;
                break;

                // SLLI, SRLI
            case 8:
            case 9:
                if (!reg_valid[arg[2]]) {
                    num_stall++;
                    num_data_stall++;
                     stalled = 1;
                }
                stalled = 0;
                num_logic++;
                get_reg_val(A, arg[2]);
                B = arg[3]; // 4bit immediate 
                reg_valid[arg[1]] = 0;
                break;


                // LOAD imm
            case 10:
                // check if immediate is negative 
                numLoadImm++;
                A = arg[2] * 16 + arg[3];
                A = hex_to_dec(A);
                B = 0;
                break;

                // LOAD double
            case 11:
                if (!reg_valid[arg[2]]) {
                    num_stall++;
                    num_data_stall++;
                    stalled = 1;
                    return;
                }
                stalled = 0;
                num_mem++;
                reg_valid[arg[1]] = 0;
                get_reg_val(A, arg[2]);
                B = arg[3]; // 4 Bit immediate  signed or unsigned 
                break;

            case 12: // ST rd rs imm 
                 if (!reg_valid[arg[2]]) {
                    num_stall++;
                    num_data_stall++;
                     stalled = 1;
                    return;
                }
                stalled = 0;
                num_mem++;
                get_reg_val(A, arg[2]);
                B = arg[3];
                break;

            case 13: // JMP
                num_stall += 2;
                num_control_stall += 2;
                num_control++;
                stalled = 0;
                fetch = false;
                A = arg[1] * 16 + arg[2];
                A = hex_to_dec(A);
                B = PC;
                break;

                // BEQZ
            case 14:
                if (!reg_valid[arg[1]]) {
                    num_stall++;
                    num_data_stall++;
                    stalled = 1;
                     return;
                }
                fetch = false;
                stalled = 0;
                num_stall += 2;
                num_control_stall += 2;
                num_control++;
                A = arg[2] * 16 + arg[3]; // A immediate 
                A = hex_to_dec(A);
                get_reg_val(B, arg[1]);
                break;

                // HLT
            case 15:
                halt = 1;
                break;
        }

        stage = 2;
    }

    void EXE() {

        switch (arg[0])
        {
            // ALU operations Rd, Rs, Rt
            case 0: //ADD
                ALUOutput = A + B;
                break;
            case 1: //SUB
                ALUOutput = A - B;
                break;
            case 2: //MUL
                ALUOutput = A * B;
                break;
            case 4: //AND
                ALUOutput = (A & B);
                break;
            case 5: // OR
                ALUOutput = (A | B);
                break;
            case 6: // XOR 
                ALUOutput = (A ^ B);
                break;
                // INC
            case 3:
                ALUOutput = A + 1;
                break;
                // NOT
            case 7:
                ALUOutput = (~A);
                break;
                // SLLI
            case 8:
                ALUOutput = (A << B);
                break;
                // SRLI
            case 9:
                ALUOutput = (A >> B);
                break;
                // LOAD imm
            case 10:
                ALUOutput = (A);
                break;
                // LOAD double
            case 11:
                ALUOutput = (A + B);
                break;
            case 12: // STORE
                ALUOutput = (A + B);
                get_reg_val(B, arg[1]);
                break;

            case 13: // JMP
                fetch = true;
                ALUOutput = A * 2 + B;
                PC = ALUOutput; // A(Immediate) * 2 + B -> PC value
                break;

                // BEQZ
            case 14:
                fetch = true;
                if (B == 0) {
                    PC = PC + A * 2;
                }
                break;
                // HLT
            case 15:
                break;
        }
        stage = 3;
    }

    void MEM() {
        switch (arg[0])
        {
            case 0:
            case 1:
            case 2:
            case 4:
            case 5:
            case 6:
            case 3:
            case 7:
            case 8:
            case 9:
            case 10:
            case 13:
            case 14:
            case 15:
                break;

                // LOAD double
            case 11:
                LMD = DCache[ALUOutput]; // 4 Bit immediate  signed or unsigned 
                break;

            case 12: // ST rd rs imm 
                DCache[ALUOutput] = B;
                reg_valid[arg[1]] = 1;
                break;
        }

        stage = 4;
    }

    void WB() {
        switch (arg[0])
        {
            case 0:
            case 1:
            case 2:
            case 4:
            case 5:
            case 6:
            case 3:
            case 7:
            case 8:
            case 9:
            case 10:
                RF[arg[1]] = dec_to_hex(ALUOutput);
                reg_valid[arg[1]] = 1;
                break;

            case 11:
                RF[arg[1]] = dec_to_hex(LMD);
                reg_valid[arg[1]] = 1;
                break;

            case 12:
            case 13:
            case 14:
            case 15:
                break;
        }
        stage = 5;
    }


};

void read_files() {
    ifstream inst("ICache.txt");
    ifstream data("DCache.txt");
    ifstream reg("RF.txt");

    string x;
    int i = 0;
    while (inst >> x) {
        ICache[i] = stoi(x, 0, 16);
        i++;
    }

    i = 0;
    while (data >> x) {
        DCache[i] = stoi(x, 0, 16);
        i++;
    }

    i = 0;
    while (reg >> x) {
        RF[i] = stoi(x, 0, 16);
        i++;
    }

    inst.close();
    data.close();
    reg.close();

    for (int i = 0; i < 16; i++) {
        reg_valid[i] = true;
        RF[i] = 0;
    }
}



void output() {
    ofstream data("DCache.txt");
    ofstream out("Output.txt");

    for (auto x: DCache) {
        data << hex << ((x & (15 * 16)) / 16) << hex << (x & 15) << endl;
    }

    num_instr = num_arith + num_logic + num_shift + num_mem + numLoadImm + num_control + halt;
    out << "Total number of instructions executed: " << num_instr << endl;
    out << "Number of instructions in each class" << endl;
    out << "Arithmetic instructions              : " << num_arith << endl;
    out << "Logical instructions                 : " << num_logic << endl;
    out << "Shift instructions                   : " << num_shift << endl;
    out << "Memory instructions                  : " << num_mem << endl;
    out << "Load immediate instructions          : " << numLoadImm << endl;
    out << "Control instructions                 : " << num_control << endl;
    out << "Halt instructions                    : " << halt << endl;
    out << "Cycles Per Instruction               : " << ((double) num_cycle / (1.0 * num_instr)) << endl;
    out << "Total number of stalls               : " << (num_data_stall + num_control_stall) << endl;
    out << "Data stalls (RAW)                    : " << num_data_stall << endl;
    out << "Control stalls                       : " << num_control_stall << endl;

    data.close();
    out.close();


}

int main() {
    read_files();
 
    queue < Instruction > q;
    while (q.size() || halt == 0) {
 
        bool stalled = false;
        num_cycle++;
        int n = q.size();
        for (int j = 0; j < n; j++) {
            Instruction t = q.front();
            q.pop();
            if (t.stage == 5) {
                continue;
            }
            else {
                if (t.stage == 1) {
                    t.ID();
                }
                else if (t.stage == 2) {
                    t.EXE();
                }
                else if (t.stage == 3) {
                    t.MEM();
                }
                else if (t.stage == 4) {
                    t.WB();
                }
                q.push(t);
            }
             if (t.stalled) {
                stalled = true;
            }
        }
         if (!halt && !stalled && fetch) {
            Instruction t;
            t.IF();
            q.push(t);
        }
      }


    output();
    return 0;
}