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
        
        stats.clock();
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
    
    opcode = (instr >> 0x1A);           // Value of 26 According to Opcode Chart
    rs = (instr >> 0x15) & 0x1F;        // Value of 21 According to Opcode Chart
    rt = (instr >> 0x10) & 0x1F;        // Value of 16 According to Opcode Chart
    rd = (instr >> 0xB) & 0x1F;         // Value of 11 According to Opcode Chart
    shamt = (instr >> 0x6) & 0x1F;      // Value of 06 According to Opcode Chart
    funct = (instr & 0x3F);             // Filling up with ones to satisfy conditions
    uimm = (instr & 0xFFFF);            // Setting the unsigned version (16 ones)
    simm = ((signed) uimm << 16) >> 16; // Setting the signed version (16 ones)
    addr = (instr & 0x3FFFFFF);         // To make 26 all ones
    
    // Hint: you probably want to give all the control signals some "safe"
    // default value here, and then override their values as necessary in each
    // case statement below!
    
    opIsLoad = false;             // False Values
    opIsStore = false;            // False Values
    opIsMultDiv = false;          // False Values
    writeDest = false;            // False Values
    destReg = 0;                  // False Values
    aluSrc1 = uint32_t(0);        // False Values
    aluSrc2 = uint32_t(0);        // False Values
    storeData = 0;      // False Values
    writeData = uint32_t(0);      // False Values
    aluOp = ADD;
    
    D(cout << "  " << hex << setw(8) << pc - 4 << ": ");
    
    switch(opcode) {
        case 0x00:
            switch(funct) {
                case 0x00: D(cout << "sll " << regNames[rd] << ", " << regNames[rs] << ", " << dec << shamt);
                    writeDest = true; destReg = rd;stats.registerDest(rd);
                    aluSrc1 = regFile[rs]; stats.registerSrc(rs);
                    aluSrc2 = shamt;
                    aluOp = SHF_L;
                    
                    break;
                case 0x03: D(cout << "sra " << regNames[rd] << ", " << regNames[rs] << ", " << dec << shamt);
                    writeDest = true; destReg = rd; stats.registerDest(rd);
                    aluSrc1 = regFile[rs]; stats.registerSrc(rs);
                    aluSrc2 = shamt;
                    aluOp = SHF_R;
                    
                    break;
                case 0x08: D(cout << "jr " << regNames[rs]);
                    pc = regFile[rs]; stats.registerSrc(rs);
                    stats.flush(2);
                    break;
                case 0x10: D(cout << "mfhi " << regNames[rd]);
                    writeDest = true; destReg = rd; stats.registerDest(rd);
                    aluSrc1 = hi; stats.registerSrc(REG_HILO)
                    aluSrc2 = regFile[REG_ZERO];
                    aluOp = ADD;
                    
                    break;
                case 0x12: D(cout << "mflo " << regNames[rd]);
                    writeDest = true; destReg = rd; stats.registerDest(rd);
                    aluSrc1 = lo; stats.registerSrc(REG_HILO)
                    aluSrc2 = regFile[REG_ZERO];
                    aluOp = ADD;
                    
                    break;
                case 0x18: D(cout << "mult " << regNames[rs] << ", " << regNames[rt]);
                    opIsMultDiv = true;
                    aluSrc1 = regFile[rs]; stats.registerSrc(rs);
                    aluSrc2 = regFile[rt]; stats.registerSrc(rt);
                    aluOp = MUL;
                    
                    break;
                case 0x1a: D(cout << "div " << regNames[rs] << ", " << regNames[rt]);
                    opIsMultDiv = true;
                    aluSrc1 = regFile[rs]; stats.registerSrc(rs);
                    aluSrc2 = regFile[rt]; stats.registerSrc(rt);
                    aluOp = DIV;
                    
                    break;
                case 0x21: D(cout << "addu " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]);
                    writeDest = true; destReg = rd; stats.registerDest(rd);
                    aluSrc1 = regFile[rs]; stats.registerSrc(rs);
                    aluSrc2 = regFile[rt]; stats.registerSrc(rt);
                    aluOp = ADD;
                    
                    break;
                case 0x23: D(cout << "subu " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]);
                    writeDest = true; destReg = rd; stats.registerDest(rd);
                    aluSrc1 = regFile[rs]; stats.registerSrc(rs);
                    aluSrc2 = -regFile[rt]; stats.registerSrc(rt);
                    aluOp = ADD;
                    
                    break;
                case 0x2a: D(cout << "slt " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]);
                    writeDest = true; destReg = rd; stats.registerDest(rd);
                    aluSrc1 = regFile[rs]; stats.registerSrc(rs);
                    aluSrc2 = regFile[rt]; stats.registerSrc(rt);
                    aluOp = CMP_LT;
                    
                    break;
                default: cerr << "unimplemented instruction: pc = 0x" << hex << pc - 4 << endl;
            }
            break;
            
        case 0x02: D(cout << "j " << hex << ((pc & 0xf0000000) | addr << 2)); // P1: pc + 4
            pc = (pc & 0xf0000000) | (addr << 2);
            stats.flush(2);
            break;
        case 0x03: D(cout << "jal " << hex << ((pc & 0xf0000000) | addr << 2)); // P1: pc + 4
            writeDest = true; destReg = REG_RA; // writes PC+4 to $ra
            aluOp = ADD; // ALU should pass pc thru unchanged
            aluSrc1 = pc;
            aluSrc2 = regFile[REG_ZERO]; stats.registerDest(REG_RA)// always reads zero
            pc = (pc & 0xf0000000) | addr << 2;
            stats.flush(2);
            break;
        case 0x04: D(cout << "beq " << regNames[rs] << ", " << regNames[rt] << ", " << pc + (simm << 2));
            stats.registerSrc(rs);
            stats.registerSrc(rt);
            if (regNames[rs] == regNames[rt]){
                pc = pc + (simm << 2);
                stats.flush(2);
                stats.countTaken();
            }
            stats.countBranch();
            break;
        case 0x05: D(cout << "bne " << regNames[rs] << ", " << regNames[rt] << ", " << pc + (simm << 2));
            stats.registerSrc(rs);
            stats.registerSrc(rt);
            if (regNames[rs] != regNames[rt]){
                pc = pc + (simm << 2);
                stats.flush(2);
                stats.countTaken();
            }
            stats.countBranch();
            break;
        case 0x09: D(cout << "addiu " << regNames[rt] << ", " << regNames[rs] << ", " << dec << simm);
            writeDest = true; destReg = rt; stats.registerDest(rt);
            aluSrc1 = regFile[rs]; stats.registerSrc(rs);
            aluSrc2 = simm;
            aluOp = ADD;
            
            break;
        case 0x0c: D(cout << "andi " << regNames[rt] << ", " << regNames[rs] << ", " << dec << uimm);
            writeDest = true; destReg = rt; stats.registerDest(rt);
            aluSrc1 = regFile[rs]; stats.registerSrc(rs);
            aluSrc2 = simm;
            aluOp = AND;
            
            break;
        case 0x0f: D(cout << "lui " << regNames[rt] << ", " << dec << simm);
            writeDest = true; destReg = rt; stats.registerDest(rt);
            aluOp = SHF_L;
            aluSrc1 = simm;
            aluSrc2 = 16;
            
            break;
        case 0x1a: D(cout << "trap " << hex << addr);
            switch(addr & 0xf) {
                case 0x0: cout << endl; break;
                case 0x1: cout << " " << (signed)regFile[rs]; stats.registerSrc(rs);
                    break;
                case 0x5: cout << endl << "? "; cin >> regFile[rt]; stats.registerSrc(rt);
                    break;
                case 0xa: stop = true; break;
                default: cerr << "unimplemented trap: pc = 0x" << hex << pc - 4 << endl;
                    stop = true;
            }
            break;
            
        case 0x23: D(cout << "lw " << regNames[rt] << ", " << dec << simm << "(" << regNames[rs] << ")");
            opIsLoad = true;destReg = rt; stats.registerDest(rt);
            writeDest = true;
            aluOp = ADD;
            aluSrc1 = regFile[rs]; stats.registerSrc(rs);
            aluSrc2 = simm;
            stats.countMemOp();
            break;
        case 0x2b: D(cout << "sw " << regNames[rt] << ", " << dec << simm << "(" << regNames[rs] << ")");
            opIsStore = true;
            storeData = regFile[rt]; stats.registerSrc(rt);
            aluOp = ADD;
            aluSrc1 = regFile[rs]; stats.registerSrc(rs);
            aluSrc2 = simm;
            stats.countMemOp();
            break;
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
    if(writeDest && destReg > 0) // skip if write is to zero reg
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
    
    cout << "Cycles: " << stats.getCycles() << endl;
    cout << "CPI: "<< fixed << setprecision(2) << double(stats.getCycles() / instructions)<< endl;
    
    cout << endl;
    
    cout << "Bubbles" << stats.getBubbles() << endl;
    cout << "Flushes" << stats.getFlushes() << endl;
    
    cout << endl;
    
    cout << "Mem Ops: "<< fixed << setprecision(1) << 100.0 * ( stats.getMemOps() / instructions ) << "% of Instructions" << endl;
    cout << "Branches: "<< fixed << setprecision(1) << 100.0 * (stats.getBranches() / instructions)<< "% of Instructions" << endl;
    cout << "% Taken: " << setprecision(1) << 100.0 * (stats.getTaken()/stats.getBranches()) << endl;
    
    cout << endl;
    
}
