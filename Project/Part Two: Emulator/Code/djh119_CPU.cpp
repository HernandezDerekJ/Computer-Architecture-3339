/******************************
 * Name:  Derek Hernandez (djh119)
 * CS 3339 - Spring 2019
 ******************************/
#include "CPU.h"

const string CPU::regNames[] = {"$zero","$at","$v0","$v1","$a0","$a1","$a2","$a3",
    "$t0","$t1","$t2","$t3","$t4","$t5","$t6","$t7",
    "$s0","$s1","$s2","$s3","$s4","$s5","$s6","$s7",
    "$t8","$t9","$k0","$k1","$gp","$sp","$fp","$ra"};

CPU::CPU(uint32_t pc, Memory &iMem, Memory &dMem) : pc(pc), iMem(iMem), dMem(dMem) {
    for(int i = 0; i < NREGS; i++) {
        regFile[i] = 0;
    }
    hi = 0;
    lo = 0;
    regFile[28] = 0x10008000; // gp
    regFile[29] = 0x10000000 + dMem.getSize(); // sp
    
    instructions = 0;
    stop = false;
}

void CPU::run() {
    while(!stop) {
        instructions++;
        
        fetch();
        decode();
        execute();
        mem();
        writeback();
        
        D(printRegFile());
    }
}

void CPU::fetch() {
    instr = iMem.loadWord(pc);
    pc = pc + 4;
}

/////////////////////////////////////////
// ALL YOUR CHANGES GO IN THIS FUNCTION
/////////////////////////////////////////
void CPU::decode() {
    uint32_t opcode;      // opcode field
    uint32_t rs, rt, rd;  // register specifiers
    uint32_t shamt;       // shift amount (R-type)
    uint32_t funct;       // funct field (R-type)
    uint32_t uimm;        // unsigned version of immediate (I-type)
    int32_t simm;         // signed version of immediate (I-type)
    uint32_t addr;        // jump address offset field (J-type)
    
    opcode = (instr >> 0x1A);           // Will shift right by 26 to get op
    rs = (instr >> 0x15) & 0x1F;        // Will shift right by 21, to get rs
    rt = (instr >> 0x10) & 0x1F;        // Will shift right by 16, to get rt
    rd = (instr >> 0xB) & 0x1F;         // Will shift right by 11, to get rd
    shamt = (instr >> 0x6) & 0x1F;      //Will shift right by 6, to isolate the shamt
    funct = instr & 0x3F;               // Will isolate function binary value
    uimm = instr & 0xFFFF;              // Will get binary unsigned value
    if ((instr & 0x8000) == 0)
        simm = instr & 0xFFFF;
    else
        simm = (instr & 0xFFFF) ^ 0xFFFF0000;
    addr = instr & 0x3FFFFFF;           //Will gather the 26 bit address using 26 1's
    
    // Hint: you probably want to give all the control signals some "safe"
    // default value here, and then override their values as necessary in each
    // case statement below!
    
    opIsLoad, opIsStore, opIsMultDiv, writeDest =  false;
    aluOp = ADD;
    storeData = 0;
    aluSrc1, aluSrc2, destReg,writeData = uint32_t(0);
    
    
    D(cout << "  " << hex << setw(8) << pc - 4 << ": ");
    switch(opcode) {
        case 0x00:
            switch(funct) {
                case 0x00: D(cout << "sll " << regNames[rd] << ", " << regNames[rs] << ", " << dec << shamt);
                    //Shift left by shamt
                    opIsStore = false;
                    opIsMultDiv = false;
                    opIsLoad = false;
                    writeDest = true;
                    aluOp = SHF_L;
                    aluSrc1 = regFile[rs];
                    aluSrc2 = shamt;
                    destReg = rd; //Reg to save to
                    storeData = 0;
                    break;
                case 0x03: D(cout << "sra " << regNames[rd] << ", " << regNames[rs] << ", " << dec << shamt);
                    //Shift right by shamt
                    opIsStore = false;
                    opIsMultDiv = false;
                    opIsLoad = false;
                    writeDest = true;
                    aluOp = SHF_R;
                    aluSrc1 = regFile[rs];
                    aluSrc2 = shamt;
                    destReg = rd; //Reg to save to
                    break;
                case 0x08: D(cout << "jr " << regNames[rs]);
                    opIsStore = false;
                    opIsMultDiv = false;
                    opIsLoad = false;
                    writeDest =  false;
                    pc = regFile[rs];
                    break;
                case 0x10: D(cout << "mfhi " << regNames[rd]);
                    // Access and save value to hi
                    opIsStore = false;
                    opIsMultDiv = false;
                    opIsLoad = false;
                    writeDest = true;
                    aluOp = ADD;
                    aluSrc1 = hi;
                    aluSrc2 = regFile[REG_ZERO];
                    destReg = rd;
                    break;
                case 0x12: D(cout << "mflo " << regNames[rd]);
                    // Access and save value to lo
                    opIsStore = false;
                    opIsMultDiv = false;
                    opIsLoad = false;
                    writeDest = true;
                    aluOp = ADD;
                    aluSrc1 = lo;
                    aluSrc2 = regFile[REG_ZERO];
                    destReg = rd;
                    break;
                case 0x18: D(cout << "mult " << regNames[rs] << ", " << regNames[rt]);
                    //Standard Multiply
                    opIsStore = false;
                    writeDest = false;
                    opIsLoad = false;
                    opIsMultDiv = true;
                    aluOp = MUL;
                    aluSrc1 = regFile[rs];
                    aluSrc2 = regFile[rt];
                    break;
                case 0x1a: D(cout << "div " << regNames[rs] << ", " << regNames[rt]);
                    //Standard Divide
                    opIsStore = false;
                    opIsMultDiv = true;
                    writeDest = false;
                    opIsLoad = false;
                    aluOp = DIV;
                    aluSrc1 = regFile[rs];
                    aluSrc2 = regFile[rt];
                    break;
                case 0x21: D(cout << "addu " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]);
                    //Unsigned Add
                    opIsStore = false;
                    opIsMultDiv = false;
                    opIsLoad = false;
                    writeDest = true;
                    aluOp = ADD;
                    aluSrc1 = regFile[rs];
                    aluSrc2 = regFile[rt];
                    destReg = rd;
                    break;
                case 0x23: D(cout << "subu " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]);
                    // Unsigned Sub
                    opIsStore = false;
                    opIsMultDiv = false;
                    opIsLoad = false;
                    writeDest = true;
                    aluOp = ADD;
                    aluSrc1 = regFile[rs];
                    aluSrc2 = -(regFile[rt]);
                    destReg = rd;
                    break;
                case 0x2a: D(cout << "slt " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]);
                    //Set less than, Compare fucntion rs<rt
                    opIsStore = false;
                    opIsMultDiv = false;
                    opIsLoad = false;
                    writeDest = true;
                    aluOp = CMP_LT;
                    aluSrc1 = regFile[rs];
                    aluSrc2 = regFile[rt];
                    destReg = rd;
                    break;
                default: cerr << "unimplemented instruction: pc = 0x" << hex << pc - 4 << endl;
            }
            break;
        case 0x02: D(cout << "j " << hex << ((pc & 0xf0000000) | addr << 2)); // P1: pc + 4
            // Jump to address
            opIsStore = false;
            opIsMultDiv = false;
            opIsLoad = false;
            pc = ((pc & 0xf0000000) | (addr << 2));
            
            break;
        case 0x03: D(cout << "jal " << hex << ((pc & 0xf0000000) | addr << 2)); // P1: pc + 4
            opIsStore = false;
            opIsMultDiv = false;
            opIsLoad = false;
            writeDest = true; destReg = REG_RA; // writes PC+4 to $ra
            aluOp = ADD; // ALU should pass pc thru unchanged
            aluSrc1 = pc;
            aluSrc2 = regFile[REG_ZERO]; // always reads zero
            pc = (pc & 0xf0000000) | addr << 2;
            break;
        case 0x04: D(cout << "beq " << regNames[rs] << ", " << regNames[rt] << ", " << pc + (simm << 2));
            // Branch On equal checks if rs and rt are =
            opIsStore = false;
            opIsMultDiv = false;
            opIsLoad = false;
            if (regFile[rs] == regFile[rt])
                pc = pc + (simm << 2);
            break;  // read the handout carefully, update PC directly here as in jal example
        case 0x05: D(cout << "bne " << regNames[rs] << ", " << regNames[rt] << ", " << pc + (simm << 2));
            // Branch On Not equal checks if rs and rt are !=
            opIsStore = false;
            opIsMultDiv = false;
            opIsLoad = false;
            if (regFile[rs] != regFile[rt])
                pc = pc + (simm << 2);
            break;  // same comment as beq
        case 0x09: D(cout << "addiu " << regNames[rt] << ", " << regNames[rs] << ", " << dec << simm);
            //Add Imm Unsigned. * remember its an signed value thats added
            opIsStore = false;
            opIsMultDiv = false;
            opIsLoad = false;
            writeDest = true;
            aluOp = ADD;
            aluSrc1 = regFile[rs];
            aluSrc2 = simm;
            destReg = rt;
            break;
        case 0x0c: D(cout << "andi " << regNames[rt] << ", " << regNames[rs] << ", " << dec << uimm);
            //And Imm RS & Sign
            opIsStore = false;
            opIsMultDiv = false;
            opIsLoad = false;
            writeDest = true;
            aluOp = AND;
            aluSrc1 = regFile[rs];
            aluSrc2 = simm;
            destReg = rt;
            break;
        case 0x0f: D(cout << "lui " << regNames[rt] << ", " << dec << simm);
            //Load Upper, load high 16 bit values, so <<
            opIsStore = false;
            opIsMultDiv = false;
            opIsLoad = false;
            writeDest = true;
            aluOp = SHF_L;
            aluSrc1 = simm;
            aluSrc2 = 16;
            destReg = rt;
            break;
        case 0x1a: D(cout << "trap " << hex << addr);
            switch(addr & 0xf) {
                case 0x0: cout << endl; break;
                case 0x1: cout << " " << (signed)regFile[rs];
                    break;
                case 0x5: cout << endl << "? "; cin >> regFile[rt];
                    break;
                case 0xa: stop = true; break;
                default: cerr << "unimplemented trap: pc = 0x" << hex << pc - 4 << endl;
                    stop = true;
            }
            break;
        case 0x23: D(cout << "lw " << regNames[rt] << ", " << dec << simm << "(" << regNames[rs] << ")");
            //Load word load sign and rs to rt
            opIsStore = false;
            opIsMultDiv = false;
            writeDest = true;
            opIsLoad =  true;
            aluOp = ADD;
            aluSrc1 = regFile[rs];
            aluSrc2 = simm;
            destReg = rt;
            break; // do not interact with memory here - setup control signals for mem() below
        case 0x2b: D(cout << "sw " << regNames[rt] << ", " << dec << simm << "(" << regNames[rs] << ")");
            //Store Word, load simm and rs to rt
            writeDest = false;
            opIsMultDiv = false;
            opIsLoad = false;
            opIsStore = true;
            storeData = regFile[rt];
            aluOp = ADD;
            aluSrc1 = regFile[rs];
            aluSrc2 = simm;
            break;  // same comment as lw
        default: cerr << "unimplemented instruction: pc = 0x" << hex << pc - 4 << endl;
    }
    D(cout << endl);
}

void CPU::execute() {
    aluOut = alu.op(aluOp, aluSrc1, aluSrc2);
}

void CPU::mem() {
    if(opIsLoad)
        writeData = dMem.loadWord(aluOut);
    else
        writeData = aluOut;
    
    if(opIsStore)
        dMem.storeWord(storeData, aluOut);
}

void CPU::writeback() {
    if(writeDest && destReg > 0) // skip if write is to zero register
        regFile[destReg] = writeData;
    
    if(opIsMultDiv) {
        hi = alu.getUpper();
        lo = alu.getLower();
    }
}

void CPU::printRegFile() {
    cout << hex;
    for(int i = 0; i < NREGS; i++) {
        cout << "    " << regNames[i];
        if(i > 0) cout << "  ";
        cout << ": " << setfill('0') << setw(8) << regFile[i];
        if( i == (NREGS - 1) || (i + 1) % 4 == 0 )
            cout << endl;
    }
    cout << "    hi   : " << setfill('0') << setw(8) << hi;
    cout << "    lo   : " << setfill('0') << setw(8) << lo;
    cout << dec << endl;
}

void CPU::printFinalStats() {
    cout << "Program finished at pc = 0x" << hex << pc << "  ("
    << dec << instructions << " instructions executed)" << endl;
}

