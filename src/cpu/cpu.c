#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../libunibus/device_bus_mgr.h"

enum ProcessorStatusFlags {
	CARRYFLAG	= 0000001,
	OVERFLOWFLAG = 0000002,
	ZEROFLAG	 = 0000004,
	NEGATIVEFLAG = 0000010,
	TRAPFLAG	 = 0000020,
	IPLBITS	  = 0000340,
	PREVMODE	 = 0030000,
	CURRMODE	 = 0140000
};

enum OPCode {
	// Branch OPs
	BR_op   = 0000400,
	BNE_op  = 0001000,
	BEQ_op  = 0001400,
	BGE_op  = 0002000,
	BLT_op  = 0002400,
	BGT_op  = 0003000,
	BLE_op  = 0003400,
	BPL_op  = 0100000,
	BMI_op  = 0100400,
	BHI_op  = 0101000,
	BLOS_op = 0101400,
	BVC_op  = 0102000,
	BVS_op  = 0102400,
	BCC_op  = 0103000,
	BCS_op  = 0103400,

	// Double OP Register Source
	MUL_op  = 0070000,
	DIV_op  = 0071000,
	ASH_op  = 0072000,
	ASHC_op = 0073000,
	XOR_op  = 0074000,

	// Subroutine
	JSR_op  = 0004000,
	RTS_op  = 0000200,

	// Double OPs
	MOV_op  = 0010000,
	MOVB_op = 0110000,
	CMP_op  = 0020000,
	CMPB_op = 0120000,
	BIT_op  = 0030000,
	BITB_op = 0130000,
	BIC_op  = 0040000,
	BICB_op = 0140000,
	BIS_op  = 0050000,
	BISB_op = 0150000,
	ADD_op  = 0060000,
	SUB_op  = 0160000,

	// Single OPs
	JMP_op  = 0000100,
	SWAB_op = 0000300,
	CLR_op  = 0005000,
	CLRB_op = 0105000,
	COM_op  = 0005100,
	COMB_op = 0105100,
	INC_op  = 0005200,
	INCB_op = 0105200,
	DEC_op  = 0005300,
	DECB_op = 0105300,
	NEG_op  = 0005400,
	NEGB_op = 0105400,
	ADC_op  = 0005500,
	ADCB_op = 0105500,
	SBC_op  = 0005600,
	SBCB_op = 0105600,
	TST_op  = 0005700,
	TSTB_op = 0105700,
	ROR_op  = 0006000,
	RORB_op = 0106000,
	ROL_op  = 0006100,
	ROLB_op = 0106100,
	ASR_op  = 0006200,
	ASRB_op = 0106200,
	ASL_op  = 0006300,
	ASLB_op = 0106300,
	MTPS_op = 0106400,
	MFPI_op = 0006500,
	MFPD_op = 0106500,
	MTPI_op = 0006600,
	MTPD_op = 0106600,
	SXT_op  = 0006700,
	MFPS_op = 0106700
};

typedef struct _cpu {
	uint16_t r0;
	uint16_t r1;
	uint16_t r2;
	uint16_t r3;
	uint16_t r4;
	uint16_t r5;
	uint16_t sp;
	uint16_t pc;
	uint16_t psw;
	uint16_t alu;
	uint16_t ir;

	uint32_t src_address;
	uint32_t dest_address;

	uint8_t halted;
} cpu;

static void usage();
static void init_cpu();
static void execute();
static void dump_cpu();
static void inc_register(uint16_t, uint8_t);
static void dec_register(uint16_t, uint8_t);
static uint16_t fetch_register_contents(uint16_t);
static uint32_t translate_bus_addr(uint32_t);
static uint16_t fetch_data(uint32_t);
static uint16_t fetch_data_register(uint32_t);
static void store_data(uint32_t, uint16_t);
static void store_data_b(uint32_t, uint8_t);
static void store_data_register(uint32_t, uint16_t);
static void exec_instruction();
static int8_t get_branch_offset();
static uint8_t is_internal_bus_addr(uint32_t);
static uint32_t register_to_bus_addr(uint16_t);
static uint16_t* bus_addr_to_register(uint32_t);
static void setup_pc_addressing(uint32_t*, uint16_t, uint16_t);
static void setup_sp_addressing(uint32_t*, uint16_t, uint16_t);
static void setup_general_addressing(uint32_t*, uint16_t, uint16_t);
static void setup_general_byte_addressing(uint32_t*, uint16_t, uint16_t);
static void setup_dest_addressing(uint8_t);
static void setup_src_addressing(uint8_t);
static void set_zero_flag(uint16_t);
static void set_negative_flag_b(uint8_t);
static void set_negative_flag_w(uint16_t);

static void ADCB();
static void ADC();
static void ADD();
static void ASHC();
static void ASH();
static void ASLB();
static void ASL();
static void ASRB();
static void ASR();
static void BCC();
static void BCS();
static void BEQ();
static void BGE();
static void BGT();
static void BHI();
static void BICB();
static void BIC();
static void BISB();
static void BIS();
static void BITB();
static void BIT();
static void BLE();
static void BLOS();
static void BLT();
static void BMI();
static void BNE();
static void BPL();
static void BR();
static void BVC();
static void BVS();
static void CLRB();
static void CLR();
static void CMPB();
static void CMP();
static void COMB();
static void COM();
static void DECB();
static void DEC();
static void DIV();
static void HALT();
static void INCB();
static void INC();
static void JMP();
static void JSR();
static void MFPD();
static void MFPI();
static void MFPS();
static void MOVB();
static void MOV();
static void MTPD();
static void MTPI();
static void MTPS();
static void MUL();
static void NEGB();
static void NEG();
static void NOP();
static void ROLB();
static void ROL();
static void RORB();
static void ROR();
static void RTS();
static void SBCB();
static void SBC();
static void SUB();
static void SWAB();
static void SXT();
static void TSTB();
static void TST();
static void XOR();

static void cleanup();

cpu *CPU_STATE = NULL;
bus_state *BUS_REF = NULL;


void
usage()
{
	fprintf(stderr, "cpu -p <offset>\n");
}

void
init_cpu()
{
	CPU_STATE = (cpu*) malloc(sizeof(cpu));
	if (CPU_STATE == NULL) {
		perror("init_cpu");
		return;
	}

	CPU_STATE->r0 = 0;
	CPU_STATE->r1 = 0;
	CPU_STATE->r2 = 0;
	CPU_STATE->r3 = 0;
	CPU_STATE->r4 = 0;
	CPU_STATE->r5 = 0;
	CPU_STATE->sp = 0;
	CPU_STATE->pc = 0;
	CPU_STATE->psw = 0;
	CPU_STATE->alu = 0;
	CPU_STATE->ir = 0;

	CPU_STATE->src_address = 0;
	CPU_STATE->dest_address = 0;

	CPU_STATE->halted = 0;
}

void
execute()
{
	while (!CPU_STATE->halted) {
		CPU_STATE->ir = fetch_data(CPU_STATE->pc);
		CPU_STATE->pc += 2; // advance by word.
		exec_instruction();
	}
}

void
dump_cpu()
{
	printf("ALU: %07o\n", CPU_STATE->alu);
	printf("PSW: %07o\n", CPU_STATE->psw);
	printf("IR : %07o\n", CPU_STATE->ir);
	printf("SP : %07o\n", CPU_STATE->sp);
	printf("PC : %07o\n", CPU_STATE->pc);
	printf("R0 : %07o\n", CPU_STATE->r0);
	printf("R1 : %07o\n", CPU_STATE->r1);
	printf("R2 : %07o\n", CPU_STATE->r2);
	printf("R3 : %07o\n", CPU_STATE->r3);
	printf("R4 : %07o\n", CPU_STATE->r4);
	printf("R5 : %07o\n", CPU_STATE->r5);
	printf("\n");
}

void
inc_register(uint16_t reg, uint8_t is_byte_addr)
{
	uint16_t *r = NULL;
	switch (reg) {
		case 0:
			r = &(CPU_STATE->r0);
			break;
		case 1:
			r = &(CPU_STATE->r1);
			break;
		case 2:
			r = &(CPU_STATE->r2);
			break;
		case 3:
			r = &(CPU_STATE->r3);
			break;
		case 4:
			r = &(CPU_STATE->r4);
			break;
		case 5:
			r = &(CPU_STATE->r5);
			break;
		case 6:
			r = &(CPU_STATE->sp);
			break;
		case 7:
			r = &(CPU_STATE->pc);
			break;
		case 8:
			r = &(CPU_STATE->psw);
			break;

		default:
			return;
	}

	if (is_byte_addr) {
		(*r)++;
	} else {
		*r += 2;
	}
}

void
dec_register(uint16_t reg, uint8_t is_byte_addr)
{
	uint16_t *r = NULL;
	switch (reg) {
		case 0:
			r = &(CPU_STATE->r0);
			break;
		case 1:
			r = &(CPU_STATE->r1);
			break;
		case 2:
			r = &(CPU_STATE->r2);
			break;
		case 3:
			r = &(CPU_STATE->r3);
			break;
		case 4:
			r = &(CPU_STATE->r4);
			break;
		case 5:
			r = &(CPU_STATE->r5);
			break;
		case 6:
			r = &(CPU_STATE->sp);
			break;
		case 7:
			r = &(CPU_STATE->pc);
			break;
		case 8:
			r = &(CPU_STATE->psw);
			break;

		default:
			return;
	}

	if (is_byte_addr) {
		(*r)--;
	} else {
		*r -= 2;
	}
}

uint16_t
fetch_register_contents(uint16_t reg)
{
	switch (reg) {
		case 0: return CPU_STATE->r0;
		case 1: return CPU_STATE->r1;
		case 2: return CPU_STATE->r2;
		case 3: return CPU_STATE->r3;
		case 4: return CPU_STATE->r4;
		case 5: return CPU_STATE->r5;
		case 6: return CPU_STATE->sp;
		case 7: return CPU_STATE->pc;
		case 8: return CPU_STATE->psw;

		default: return 0xffff;
	}
}

// Convert a high address to a bus address.
uint32_t
translate_bus_addr(uint32_t addr)
{
	if (addr & 0177000) {
		addr |= 0700000;
	}

	return addr;
}

uint16_t
fetch_data(uint32_t addr)
{
	uint16_t data;
	if (is_internal_bus_addr(addr)) {
		data = fetch_data_register(addr);
		return data;
	}

	addr = translate_bus_addr(addr);

	// Block for read.
	data = read_data_in(BUS_REF, addr);

	return data;
}

uint16_t
fetch_data_register(uint32_t addr)
{
	uint16_t *reg = bus_addr_to_register(addr);
	return *reg;
}

void
store_data(uint32_t addr, uint16_t data)
{
	if (is_internal_bus_addr(addr)) {
		store_data_register(addr, data);
		return;
	}

	addr = translate_bus_addr(addr);

	// Block for write.
	write_data_out(BUS_REF, addr, data);
}

void
store_data_b(uint32_t addr, uint8_t data)
{
	if (is_internal_bus_addr(addr)) {
		store_data_register(addr, data);
		return;
	}

	addr = translate_bus_addr(addr);

	// Block for write.
	write_data_out_b(BUS_REF, addr, data);
}

void
store_data_register(uint32_t addr, uint16_t data)
{
	uint16_t *reg = bus_addr_to_register(addr);
	*reg = data;
}

void
exec_instruction()
{
	dump_cpu();

	uint16_t ir = CPU_STATE->ir;
	switch (ir & 0177400) {
	// Branch OPS
	case BR_op:
		BR();
		break;
	case BNE_op:
		BNE();
		break;
	case BEQ_op:
		BEQ();
		break;
	case BGE_op:
		BGE();
		break;
	case BLT_op:
		BLT();
		break;
	case BGT_op:
		BGT();
		break;
	case BLE_op:
		BLE();
		break;
	case BPL_op:
		BPL();
		break;
	case BMI_op:
		BMI();
		break;
	case BHI_op:
		BHI();
		break;
	case BLOS_op:
		BLOS();
		break;
	case BVC_op:
		BVC();
		break;
	case BVS_op:
		BVS();
		break;
	case BCC_op:
		BCC();
		break;
	case BCS_op:
		BCS();
		break;

	default:
		// Double OP Register Source and JSR
		switch (ir & 0177000) {
		case MUL_op:
			MUL();
			break;
		case DIV_op:
			DIV();
			break;
		case ASH_op:
			ASH();
			break;
		case ASHC_op:
			ASHC();
			break;
		case XOR_op:
			XOR();
			break;
		case JSR_op:
			JSR();
			break;
		default:
			switch (ir & 0170000) {
			// Double OPs
			case MOV_op:
				MOV();
				break;
			case MOVB_op:
				MOVB();
				break;
			case CMP_op:
				CMP();
				break;
			case CMPB_op:
				CMPB();
				break;
			case BIT_op:
				BIT();
				break;
			case BITB_op:
				BITB();
				break;
			case BIC_op:
				BIC();
				break;
			case BICB_op:
				BICB();
				break;
			case BIS_op:
				BIS();
				break;
			case BISB_op:
				BISB();
				break;
			case ADD_op:
				ADD();
				break;
			case SUB_op:
				SUB();
				break;
			default:
				// Single OPs
				switch (ir & 0177700) {
				case JMP_op:
					JMP();
					break;
				case SWAB_op:
					SWAB();
					break;
				case CLR_op:
					CLR();
					break;
				case CLRB_op:
					CLRB();
					break;
				case COM_op:
					COM();
					break;
				case COMB_op:
					COMB();
					break;
				case INC_op:
					INC();
					break;
				case INCB_op:
					INCB();
					break;
				case DEC_op:
					DEC();
					break;
				case DECB_op:
					DECB();
					break;
				case NEG_op:
					NEG();
					break;
				case NEGB_op:
					NEGB();
					break;
				case ADC_op:
					ADC();
					break;
				case ADCB_op:
					ADCB();
					break;
				case SBC_op:
					SBC();
					break;
				case SBCB_op:
					SBCB();
					break;
				case TST_op:
					TST();
					break;
				case TSTB_op:
					TSTB();
					break;
				case ROR_op:
					ROR();
					break;
				case RORB_op:
					RORB();
					break;
				case ROL_op:
					ROL();
					break;
				case ROLB_op:
					ROLB();
					break;
				case ASR_op:
					ASR();
					break;
				case ASRB_op:
					ASRB();
					break;
				case ASL_op:
					ASL();
					break;
				case ASLB_op:
					ASLB();
					break;
				case MTPS_op:
					MTPS();
					break;
				case MFPI_op:
					MFPI();
					break;
				case MFPD_op:
					MFPD();
					break;
				case MTPI_op:
					MTPI();
					break;
				case MTPD_op:
					MTPD();
					break;
				case SXT_op:
					SXT();
					break;
				case MFPS_op:
					MFPS();
					break;
				case RTS_op:
					RTS();
					break;
				default:
					HALT();
					break;
				}
			}
		}
	}
}

int8_t
get_branch_offset()
{
	return (int8_t) (CPU_STATE->ir & 0377);
}

/*******************************
 * Addressing
 *******************************/

uint8_t
is_internal_bus_addr(uint32_t addr)
{
	switch (addr) {
		case 0777700: return 1;
		case 0777701: return 1;
		case 0777702: return 1;
		case 0777703: return 1;
		case 0777704: return 1;
		case 0777705: return 1;
		case 0777706: return 1;
		case 0777707: return 1;
		case 0777776: return 1;

		default: return 0;
	}
}

uint32_t
register_to_bus_addr(uint16_t reg)
{
	switch (reg) {
		case 0: return 0777700;
		case 1: return 0777701;
		case 2: return 0777702;
		case 3: return 0777703;
		case 4: return 0777704;
		case 5: return 0777705;
		case 6: return 0777706; // SP
		case 7: return 0777707; // PC
		case 8: return 0777776; // PSW

		default: return 0xffffffff;
	}
}

uint16_t*
bus_addr_to_register(uint32_t addr)
{
	switch (addr) {
		case 0777700: return &CPU_STATE->r0;
		case 0777701: return &CPU_STATE->r1;
		case 0777702: return &CPU_STATE->r2;
		case 0777703: return &CPU_STATE->r3;
		case 0777704: return &CPU_STATE->r4;
		case 0777705: return &CPU_STATE->r5;
		case 0777706: return &CPU_STATE->sp;
		case 0777707: return &CPU_STATE->pc;
		case 0777776: return &CPU_STATE->psw;

		default: return NULL;
	}
}

void
setup_pc_addressing(uint32_t *loc, uint16_t mode, uint16_t reg)
{
	uint32_t relative = 0;
	switch (mode) {
		// From register.
		case 0:
			*loc = register_to_bus_addr(reg);
			break;

		// From immediate.
		case 2:
			*loc = CPU_STATE->pc;
			CPU_STATE->pc += 2;
			break;

		// From absolute.
		case 3:
			*loc = fetch_data(CPU_STATE->pc);
			CPU_STATE->pc += 2;
			break;

		// Relative
		case 6:
			relative = fetch_data(CPU_STATE->pc);
			CPU_STATE->pc += 2;
			*loc = CPU_STATE->pc + relative;
			printf("loc: 0o%06o\n", *loc);
			break;
	}
}

void
setup_sp_addressing(uint32_t *loc, uint16_t mode, uint16_t reg)
{
	uint32_t index = 0;
	switch (mode) {
		// Register: SP
		case 0:
			*loc = register_to_bus_addr(reg);
			break;

		// Deferred: (SP), top of the stack
		case 1:
			*loc = CPU_STATE->sp;
			break;

		// Autoincrement: (Rn)+, get top of stack and then pop it
		case 2:
			*loc = CPU_STATE->sp;
			CPU_STATE->sp += 2;
			break;

		// Deferred autoincrement: @(SP)+, top of stack is a pointer to a value,
		// then pop it.
		case 3:
			*loc = fetch_data(CPU_STATE->sp);
			CPU_STATE->sp += 2;
			break;

		// Autodecrement: -(SP), push value onto stack.
		case 4:
			CPU_STATE->sp -= 2;
			*loc = CPU_STATE->sp;
			break;

		// Indexed: X(SP), access item X on the stack
		case 6:
			index = fetch_data(CPU_STATE->pc);
			CPU_STATE->pc += 2;
			*loc = CPU_STATE->sp + index;
			break;

		// Deferred index: @X(SP), access item pointed to by item X on the stack
		case 7:
			index = fetch_data(CPU_STATE->pc);
			CPU_STATE->pc += 2;
			*loc = fetch_data(index + CPU_STATE->sp);
			break;
	}
}

void
setup_general_addressing(uint32_t *loc, uint16_t mode, uint16_t reg)
{
	uint16_t index = 0;
	uint32_t deferred = 0;
	switch (mode) {
		// Register: Rn
		case 0:
			*loc = register_to_bus_addr(reg);
			break;

		// Deferred: (Rn)
		case 1:
			*loc = fetch_register_contents(reg);
			break;

		// Autoincrement: (Rn)+
		//
		// NOTE: (incremented by one or two) to address the next word or
		// byte depending upon whether the instruction **operates on byte or word data**
		case 2:
			*loc = fetch_register_contents(reg);
			inc_register(reg, 0);
			break;

		// Deferred autoincrement: @(Rn)+
		case 3:
			deferred = fetch_register_contents(reg);
			index = fetch_data(deferred);
			inc_register(reg, 0);
			*loc = fetch_data(index);
			break;

		// Autodecrement: -(Rn)
		//
		// NOTE: (decremented by one or two) to address the next word or
		// byte depending upon whether the instruction **operates on byte or word data**
		case 4:
			*loc = fetch_register_contents(reg);
			dec_register(reg, 0);
			break;

		// Deferred autodecrement: @-(Rn)
		case 5:
			deferred = fetch_register_contents(reg);
			index = fetch_data(deferred);
			*loc = fetch_data(index);
			index -= 2;
			store_data(deferred, index);
			break;

		// Indexed: X(Rn)
		case 6:
			index = fetch_data(CPU_STATE->pc);
			CPU_STATE->pc += 2;
			deferred = fetch_register_contents(reg);
			*loc = fetch_data(index + deferred);
			break;

		// Deferred indexed: @X(Rn)
		case 7:
			index = fetch_data(CPU_STATE->pc);
			CPU_STATE->pc += 2;
			deferred = fetch_register_contents(reg);
			deferred = fetch_data(deferred);
			*loc = fetch_data(deferred + index);
			break;
	}
}

void
setup_general_byte_addressing(uint32_t *loc, uint16_t mode, uint16_t reg)
{
	uint16_t index = 0;
	uint32_t deferred = 0;
	switch (mode) {
		// Register: Rn
		case 0:
			*loc = register_to_bus_addr(reg);
			break;

		// Deferred: (Rn)
		case 1:
			*loc = fetch_register_contents(reg);
			break;

		// Autoincrement: (Rn)+
		//
		// NOTE: (incremented by one or two) to address the next word or
		// byte depending upon whether the instruction **operates on byte or word data**
		case 2:
			*loc = fetch_register_contents(reg);
			inc_register(reg, 1);
			break;

		// Deferred autoincrement: @(Rn)+
		case 3:
			deferred = fetch_register_contents(reg);
			index = fetch_data(deferred);
			inc_register(reg, 1);
			*loc = fetch_data(index);
			break;

		// Autodecrement: -(Rn)
		//
		// NOTE: (decremented by one or two) to address the next word or
		// byte depending upon whether the instruction **operates on byte or word data**
		case 4:
			*loc = fetch_register_contents(reg);
			dec_register(reg, 1);
			break;

		// Deferred autodecrement: @-(Rn)
		case 5:
			deferred = fetch_register_contents(reg);
			index = fetch_data(deferred);
			*loc = fetch_data(index);
			index -= 1;
			store_data(deferred, index);
			break;

		// Indexed: X(Rn)
		case 6:
			index = fetch_data(CPU_STATE->pc);
			CPU_STATE->pc += 2;
			deferred = fetch_register_contents(reg);
			*loc = fetch_data(index + deferred);
			break;

		// Deferred indexed: @X(Rn)
		case 7:
			index = fetch_data(CPU_STATE->pc);
			CPU_STATE->pc += 2;
			deferred = fetch_register_contents(reg);
			deferred = fetch_data(deferred);
			*loc = fetch_data(deferred + index);
			break;
	}
}

void
setup_dest_addressing(uint8_t use_byte_addr)
{
	uint16_t reg = (CPU_STATE->ir & 00007);
	uint16_t mode = (CPU_STATE->ir & 00070) >> 3;
	switch (reg) {
		case 7:
			setup_pc_addressing(&(CPU_STATE->dest_address), mode, reg);
			break;
		case 6:
			setup_sp_addressing(&(CPU_STATE->dest_address), mode, reg);
			break;
		default:
			if (use_byte_addr) {
				setup_general_byte_addressing(&(CPU_STATE->dest_address), mode, reg);
			} else {
				setup_general_addressing(&(CPU_STATE->dest_address), mode, reg);
			}
	}
}

void
setup_src_addressing(uint8_t use_byte_addr)
{
	uint16_t reg = (CPU_STATE->ir & 00700) >> 6;
	uint16_t mode = (CPU_STATE->ir & 07000) >> 9;
	printf("reg: %u, mode: %u\n", reg, mode);
	switch (reg) {
		case 7:
			setup_pc_addressing(&(CPU_STATE->src_address), mode, reg);
			break;
		case 6:
			setup_sp_addressing(&(CPU_STATE->src_address), mode, reg);
			break;
		default:
			if (use_byte_addr) {
				setup_general_byte_addressing(&(CPU_STATE->src_address), mode, reg);
			} else {
				setup_general_addressing(&(CPU_STATE->src_address), mode, reg);
			}
	}
}

void
set_zero_flag(uint16_t value)
{
	if (value == 0) {
		CPU_STATE->psw |= ZEROFLAG;
	} else {
		CPU_STATE->psw = (CPU_STATE->psw & ZEROFLAG) ? (CPU_STATE->psw ^ ZEROFLAG) : CPU_STATE->psw;
	}
}

void
set_negative_flag_b(uint8_t value)
{
	uint8_t mask = 0200;
	if (value & mask) {
		CPU_STATE->psw |= NEGATIVEFLAG;
	} else {
		CPU_STATE->psw = (CPU_STATE->psw & NEGATIVEFLAG) ? (CPU_STATE->psw ^ NEGATIVEFLAG) : CPU_STATE->psw;
	}
}

void
set_negative_flag_w(uint16_t value)
{
	uint16_t mask = 0100000;
	if (value & mask) {
		CPU_STATE->psw |= NEGATIVEFLAG;
	} else {
		CPU_STATE->psw = (CPU_STATE->psw & NEGATIVEFLAG) ? (CPU_STATE->psw ^ NEGATIVEFLAG) : CPU_STATE->psw;
	}
}

void
ADCB()
{
	setup_dest_addressing(1);

	CPU_STATE->alu = fetch_data(CPU_STATE->dest_address) & 0377;
	CPU_STATE->alu += CPU_STATE->psw & CARRYFLAG;
	store_data_b(CPU_STATE->dest_address, CPU_STATE->alu);

	set_zero_flag(CPU_STATE->alu);
	set_negative_flag_b(CPU_STATE->alu);
}

void
ADC()
{
	setup_dest_addressing(0);

	CPU_STATE->alu = fetch_data(CPU_STATE->dest_address);
	CPU_STATE->alu += CPU_STATE->psw & CARRYFLAG;
	store_data(CPU_STATE->dest_address, CPU_STATE->alu);

	set_zero_flag(CPU_STATE->alu);
	set_negative_flag_w(CPU_STATE->alu);
}

void
ADD()
{
	setup_src_addressing(0);
	setup_dest_addressing(0);

	CPU_STATE->alu = fetch_data(CPU_STATE->src_address);
	CPU_STATE->alu += fetch_data(CPU_STATE->dest_address);
	store_data(CPU_STATE->dest_address, CPU_STATE->alu);

	set_negative_flag_w(CPU_STATE->alu);
	set_zero_flag(CPU_STATE->alu);
}

void
ASHC()
{
	NOP();
}

void
ASH()
{
	NOP();
}

void
ASLB()
{
	setup_dest_addressing(1);

	CPU_STATE->alu = fetch_data(CPU_STATE->dest_address) & 0377;
	CPU_STATE->alu <<= 1;
	store_data(CPU_STATE->dest_address, CPU_STATE->alu);

	set_zero_flag(CPU_STATE->alu);
	set_negative_flag_b(CPU_STATE->alu);
}

void
ASL()
{
	setup_dest_addressing(0);

	CPU_STATE->alu = fetch_data(CPU_STATE->dest_address);
	CPU_STATE->alu <<= 1;
	store_data(CPU_STATE->dest_address, CPU_STATE->alu);

	set_zero_flag(CPU_STATE->alu);
	set_negative_flag_w(CPU_STATE->alu);
}

void
ASRB()
{
	setup_dest_addressing(1);

	/* FIXME? fetch_data_b? */
	CPU_STATE->alu = fetch_data(CPU_STATE->dest_address) & 0377;
	CPU_STATE->alu >>= 1;
	store_data_b(CPU_STATE->dest_address, CPU_STATE->alu);

	set_zero_flag(CPU_STATE->alu);
	set_negative_flag_b(CPU_STATE->alu);
}

void
ASR()
{
	setup_dest_addressing(0);

	CPU_STATE->alu = fetch_data(CPU_STATE->dest_address);
	CPU_STATE->alu >>= 1;
	store_data(CPU_STATE->dest_address, CPU_STATE->alu);

	set_zero_flag(CPU_STATE->alu);
	set_negative_flag_w(CPU_STATE->alu);
}

void
BCC()
{
	int8_t offset = get_branch_offset();
	CPU_STATE->alu = CPU_STATE->psw & CARRYFLAG;
	if (CPU_STATE->alu == 0)
		CPU_STATE->pc = CPU_STATE->pc + (2 * offset);
}

void
BCS()
{
	int8_t offset = get_branch_offset();
	CPU_STATE->alu = CPU_STATE->psw & CARRYFLAG;
	if (CPU_STATE->alu)
		CPU_STATE->pc = CPU_STATE->pc + (2 * offset);
}

void
BEQ()
{
	int8_t offset = get_branch_offset();
	CPU_STATE->alu = CPU_STATE->psw & ZEROFLAG;
	if (CPU_STATE->alu == ZEROFLAG)
		CPU_STATE->pc = CPU_STATE->pc + (2 * offset);
}

void
BGE()
{
	int8_t offset = get_branch_offset();
	CPU_STATE->alu = (CPU_STATE->psw & NEGATIVEFLAG) >> 3;
	CPU_STATE->alu ^= (CPU_STATE->psw & OVERFLOWFLAG) >> 1;
	if (CPU_STATE->alu == 0)
		CPU_STATE->pc = CPU_STATE->pc + (2 * offset);
}

void
BGT()
{
	int8_t offset = get_branch_offset();
	CPU_STATE->alu = (CPU_STATE->psw & ZEROFLAG) >> 2;
	CPU_STATE->alu |= ((CPU_STATE->psw & NEGATIVEFLAG) >> 3) ^ ((CPU_STATE->psw & OVERFLOWFLAG) >> 1);
	if (CPU_STATE->alu == 0)
		CPU_STATE->pc = CPU_STATE->pc + (2 * offset);
}

void
BHI()
{
	int8_t offset = get_branch_offset();
	CPU_STATE->alu = CPU_STATE->psw & CARRYFLAG;
	CPU_STATE->alu |= (CPU_STATE->psw & ZEROFLAG) >> 2;
	if (CPU_STATE->alu == 0)
		CPU_STATE->pc = CPU_STATE->pc + (2 * offset);
}

void
BICB()
{
	setup_src_addressing(1);
	setup_dest_addressing(1);

	CPU_STATE->alu = fetch_data(CPU_STATE->dest_address) & 0377;
	CPU_STATE->alu &= ~(fetch_data(CPU_STATE->src_address) & 0377);
	store_data_b(CPU_STATE->dest_address, CPU_STATE->alu);

	set_negative_flag_b(CPU_STATE->alu);
	set_zero_flag(CPU_STATE->alu);
}

void
BIC()
{
	setup_src_addressing(0);
	setup_dest_addressing(0);

	CPU_STATE->alu = fetch_data(CPU_STATE->dest_address);
	CPU_STATE->alu &= ~(fetch_data(CPU_STATE->src_address));
	store_data(CPU_STATE->dest_address, CPU_STATE->alu);

	set_negative_flag_w(CPU_STATE->alu);
	set_zero_flag(CPU_STATE->alu);
}

void
BISB()
{
	setup_src_addressing(1);
	setup_dest_addressing(1);

	CPU_STATE->alu = fetch_data(CPU_STATE->dest_address) & 0377;
	CPU_STATE->alu |= fetch_data(CPU_STATE->src_address) & 0377;
	store_data_b(CPU_STATE->dest_address, CPU_STATE->alu);

	set_negative_flag_b(CPU_STATE->alu);
	set_zero_flag(CPU_STATE->alu);
}

void
BIS()
{
	setup_src_addressing(0);
	setup_dest_addressing(0);

	CPU_STATE->alu = fetch_data(CPU_STATE->dest_address);
	CPU_STATE->alu |= fetch_data(CPU_STATE->src_address);
	store_data(CPU_STATE->dest_address, CPU_STATE->alu);

	set_negative_flag_w(CPU_STATE->alu);
	set_zero_flag(CPU_STATE->alu);
}

void
BITB()
{
	setup_src_addressing(1);
	setup_dest_addressing(1);

	CPU_STATE->alu = fetch_data(CPU_STATE->src_address) & 0377;
	CPU_STATE->alu &= fetch_data(CPU_STATE->dest_address) & 0377;

	set_negative_flag_b(CPU_STATE->alu);
	set_zero_flag(CPU_STATE->alu);
}

void
BIT()
{
	setup_src_addressing(0);
	setup_dest_addressing(0);

	CPU_STATE->alu = fetch_data(CPU_STATE->src_address);
	CPU_STATE->alu &= fetch_data(CPU_STATE->dest_address);

	set_zero_flag(CPU_STATE->alu);
	set_negative_flag_w(CPU_STATE->alu);
}

void
BLE()
{
	int8_t offset = get_branch_offset();
	CPU_STATE->alu = (CPU_STATE->psw & ZEROFLAG) >> 2;
	CPU_STATE->alu |= ((CPU_STATE->psw & NEGATIVEFLAG) >> 3) ^ ((CPU_STATE->psw & OVERFLOWFLAG) >> 1);
	if (CPU_STATE->alu)
		CPU_STATE->pc = CPU_STATE->pc + (2 * offset);
}

void
BLOS()
{
	int8_t offset = get_branch_offset();
	CPU_STATE->alu = CPU_STATE->psw & CARRYFLAG;
	CPU_STATE->alu |= (CPU_STATE->psw & ZEROFLAG) >> 2;
	if (CPU_STATE->alu)
		CPU_STATE->pc = CPU_STATE->pc + (2 * offset);
}

void
BLT()
{
	int8_t offset = get_branch_offset();
	CPU_STATE->alu = (CPU_STATE->psw & NEGATIVEFLAG) >> 3;
	CPU_STATE->alu ^= (CPU_STATE->psw & OVERFLOWFLAG) >> 1;
	if (CPU_STATE->alu)
		CPU_STATE->pc = CPU_STATE->pc + (2 * offset);
}

void
BMI()
{
	int8_t offset = get_branch_offset();
	CPU_STATE->alu = CPU_STATE->psw & NEGATIVEFLAG;
	if (CPU_STATE->alu)
		CPU_STATE->pc = CPU_STATE->pc + (2 * offset);
}

void
BNE()
{
	int8_t offset = get_branch_offset();
	CPU_STATE->alu = CPU_STATE->psw & ZEROFLAG;
	if (CPU_STATE->alu == 0)
		CPU_STATE->pc = CPU_STATE->pc + (2 * offset);
}

void
BPL()
{
	int8_t offset = get_branch_offset();
	CPU_STATE->alu = CPU_STATE->psw & NEGATIVEFLAG;
	if (CPU_STATE->alu == 0)
		CPU_STATE->pc = CPU_STATE->pc + (2 * offset);
}

void
BR()
{
	CPU_STATE->pc = CPU_STATE->pc + (2 * get_branch_offset());
}

void
BVC()
{
	int8_t offset = get_branch_offset();
	CPU_STATE->alu = CPU_STATE->psw & OVERFLOWFLAG;
	if (CPU_STATE->alu == 0)
		CPU_STATE->pc = CPU_STATE->pc + (2 * offset);
}

void
BVS()
{
	int8_t offset = get_branch_offset();
	CPU_STATE->alu = CPU_STATE->psw & OVERFLOWFLAG;
	if (CPU_STATE->alu)
		CPU_STATE->pc = CPU_STATE->pc + (2 * offset);
}

void
CLRB()
{
	setup_dest_addressing(1);
	CPU_STATE->alu = 0;
	store_data_b(CPU_STATE->dest_address, CPU_STATE->alu);

	set_negative_flag_b(CPU_STATE->alu);
	set_zero_flag(CPU_STATE->alu);
}

void
CLR()
{
	setup_dest_addressing(0);
	CPU_STATE->alu = 0;
	store_data(CPU_STATE->dest_address, CPU_STATE->alu);

	set_negative_flag_w(CPU_STATE->alu);
	set_zero_flag(CPU_STATE->alu);
}

void
CMPB()
{
	setup_src_addressing(1);
	setup_dest_addressing(1);

	CPU_STATE->alu = fetch_data(CPU_STATE->dest_address) & 0377;
	CPU_STATE->alu -= fetch_data(CPU_STATE->src_address) & 0377;

	set_zero_flag(CPU_STATE->alu);
	set_negative_flag_b(CPU_STATE->alu);
}

void
CMP()
{
	setup_src_addressing(0);
	setup_dest_addressing(0);

	CPU_STATE->alu = fetch_data(CPU_STATE->dest_address);
	CPU_STATE->alu -= fetch_data(CPU_STATE->src_address);

	set_zero_flag(CPU_STATE->alu);
	set_negative_flag_w(CPU_STATE->alu);
}

void
COMB()
{
	setup_dest_addressing(0);

	CPU_STATE->alu = ~(fetch_data(CPU_STATE->dest_address) & 0377);
	store_data_b(CPU_STATE->dest_address, CPU_STATE->alu);

	set_negative_flag_b(CPU_STATE->alu);
	set_zero_flag(CPU_STATE->alu);
}

void
COM()
{
	setup_dest_addressing(0);

	CPU_STATE->alu = ~fetch_data(CPU_STATE->dest_address);
	store_data(CPU_STATE->dest_address, CPU_STATE->alu);

	set_negative_flag_w(CPU_STATE->alu);
	set_zero_flag(CPU_STATE->alu);
}

void
DECB()
{
	setup_dest_addressing(1);

	CPU_STATE->alu = fetch_data(CPU_STATE->dest_address) & 0377;
	CPU_STATE->alu--;
	store_data_b(CPU_STATE->dest_address, CPU_STATE->alu);

	set_zero_flag(CPU_STATE->alu);
	set_negative_flag_b(CPU_STATE->alu);
}

void
DEC()
{
	setup_dest_addressing(0);
	CPU_STATE->alu = fetch_data(CPU_STATE->dest_address);
	CPU_STATE->alu--;
	store_data(CPU_STATE->dest_address, CPU_STATE->alu);

	set_zero_flag(CPU_STATE->alu);
	set_negative_flag_w(CPU_STATE->alu);
}

void
DIV()
{
	NOP();
}

void
HALT()
{
	CPU_STATE->halted = 1;
}

void
INCB()
{
	setup_dest_addressing(1);
	CPU_STATE->alu = fetch_data(CPU_STATE->dest_address) & 0377;
	CPU_STATE->alu++;
	store_data_b(CPU_STATE->dest_address, CPU_STATE->alu);

	set_zero_flag(CPU_STATE->alu);
	set_negative_flag_b(CPU_STATE->alu);
}

void
INC()
{
	setup_dest_addressing(0);
	CPU_STATE->alu = fetch_data(CPU_STATE->dest_address);
	CPU_STATE->alu++;
	store_data(CPU_STATE->dest_address, CPU_STATE->alu);

	set_zero_flag(CPU_STATE->alu);
	set_negative_flag_w(CPU_STATE->alu);
}

void
JMP()
{
	setup_dest_addressing(0);
	CPU_STATE->pc = fetch_data(CPU_STATE->dest_address);
}

void
JSR()
{
	uint32_t reg = (CPU_STATE->ir & 0700) >> 6;
	 setup_dest_addressing(0);

	uint16_t tmp = CPU_STATE->dest_address;
	uint16_t reg_contents = fetch_register_contents(reg);

	store_data(CPU_STATE->sp, reg_contents);
	dec_register(CPU_STATE->sp, 0);

	store_data_register(register_to_bus_addr(reg), CPU_STATE->pc);
	CPU_STATE->pc = tmp;
}

void
MFPD()
{
	NOP();
}

void
MFPI()
{
	NOP();
}

void
MFPS()
{
	setup_dest_addressing(0);
	CPU_STATE->alu = CPU_STATE->psw;
	store_data(CPU_STATE->dest_address, CPU_STATE->alu);
}

void
MOVB()
{
	setup_src_addressing(1);
	setup_dest_addressing(1);

	CPU_STATE->alu = fetch_data(CPU_STATE->src_address) & 0377;
	store_data_b(CPU_STATE->dest_address, CPU_STATE->alu);

	set_zero_flag(CPU_STATE->alu);
	set_negative_flag_b(CPU_STATE->alu);
}

void
MOV()
{
	setup_src_addressing(0);
	setup_dest_addressing(0);

	CPU_STATE->alu = fetch_data(CPU_STATE->src_address);
	store_data(CPU_STATE->dest_address, CPU_STATE->alu);

	set_zero_flag(CPU_STATE->alu);
	set_negative_flag_w(CPU_STATE->alu);
}

void
MTPD()
{
	NOP();
}

void
MTPI()
{
	NOP();
}

void
MTPS()
{
	setup_dest_addressing(0);
	CPU_STATE->alu = fetch_data(CPU_STATE->dest_address);
	CPU_STATE->psw = CPU_STATE->alu;
}

void
MUL()
{
	NOP();
}

void
NEGB()
{
	setup_dest_addressing(1);

	CPU_STATE->alu = fetch_data(CPU_STATE->dest_address) & 0377;
	CPU_STATE->alu = (~CPU_STATE->alu + 1) & 0377;
	store_data_b(CPU_STATE->dest_address, CPU_STATE->alu);

	set_zero_flag(CPU_STATE->alu);
	set_negative_flag_b(CPU_STATE->alu);
}

void
NEG()
{
	setup_dest_addressing(0);

	CPU_STATE->alu = fetch_data(CPU_STATE->dest_address);
	CPU_STATE->alu = ~CPU_STATE->alu + 1;
	store_data(CPU_STATE->dest_address, CPU_STATE->alu);

	set_zero_flag(CPU_STATE->alu);
	set_negative_flag_w(CPU_STATE->alu);
}

void
NOP()
{
	return;
}

void
ROLB()
{
	setup_dest_addressing(1);

	CPU_STATE->alu = fetch_data(CPU_STATE->dest_address) & 0377;
	CPU_STATE->alu = (CPU_STATE->alu << 7) | (CPU_STATE->alu >> 1);
	store_data_b(CPU_STATE->dest_address, CPU_STATE->alu);

	set_zero_flag(CPU_STATE->alu);
	set_negative_flag_b(CPU_STATE->alu);
}

void
ROL()
{
	setup_dest_addressing(0);

	CPU_STATE->alu = fetch_data(CPU_STATE->dest_address);
	CPU_STATE->alu = (CPU_STATE->alu << 15) | (CPU_STATE->alu >> 1);
	store_data(CPU_STATE->dest_address, CPU_STATE->alu);

	set_zero_flag(CPU_STATE->alu);
	set_negative_flag_w(CPU_STATE->alu);
}

void
RORB()
{
	setup_dest_addressing(1);

	CPU_STATE->alu = fetch_data(CPU_STATE->dest_address) & 0377;
	CPU_STATE->alu = (CPU_STATE->alu << 1) | (CPU_STATE->alu >> 7);
	store_data_b(CPU_STATE->dest_address, CPU_STATE->alu);

	set_zero_flag(CPU_STATE->alu);
	set_negative_flag_b(CPU_STATE->alu);
}

void
ROR()
{
	setup_dest_addressing(0);

	CPU_STATE->alu = fetch_data(CPU_STATE->dest_address);
	CPU_STATE->alu = (CPU_STATE->alu << 1) | (CPU_STATE->alu >> 15);
	store_data(CPU_STATE->dest_address, CPU_STATE->alu);

	set_zero_flag(CPU_STATE->alu);
	set_negative_flag_w(CPU_STATE->alu);
}

void
RTS()
{
	uint32_t reg = CPU_STATE->ir & 07;
	uint16_t reg_contents = fetch_register_contents(reg);
	CPU_STATE->pc = reg_contents;

	inc_register(CPU_STATE->sp, 0);
	uint16_t top_stack = fetch_data(CPU_STATE->sp);
	store_data_register(register_to_bus_addr(reg), top_stack);
}

void
SBCB()
{
	setup_dest_addressing(1);

	CPU_STATE->alu = fetch_data(CPU_STATE->dest_address) & 0377;
	CPU_STATE->alu -= (CPU_STATE->psw & CARRYFLAG);
	store_data_b(CPU_STATE->dest_address, CPU_STATE->alu);

	set_zero_flag(CPU_STATE->alu);
	set_negative_flag_b(CPU_STATE->alu);
}

void
SBC()
{
	setup_dest_addressing(0);

	CPU_STATE->alu = fetch_data(CPU_STATE->dest_address);
	CPU_STATE->alu -= (CPU_STATE->psw & CARRYFLAG);
	store_data(CPU_STATE->dest_address, CPU_STATE->alu);

	set_zero_flag(CPU_STATE->alu);
	set_negative_flag_w(CPU_STATE->alu);
}

void
SUB()
{
	setup_src_addressing(0);
	setup_dest_addressing(0);

	CPU_STATE->alu = fetch_data(CPU_STATE->src_address);
	CPU_STATE->alu -= fetch_data(CPU_STATE->dest_address);
	store_data(CPU_STATE->dest_address, CPU_STATE->alu);

	set_negative_flag_w(CPU_STATE->alu);
	set_zero_flag(CPU_STATE->alu);
}

void
SWAB()
{
	setup_dest_addressing(0);

	CPU_STATE->alu = fetch_data(CPU_STATE->dest_address);
	CPU_STATE->alu = ((CPU_STATE->alu & 0177400) >> 8) | ((CPU_STATE->alu & 000377) << 8);
	store_data(CPU_STATE->dest_address, CPU_STATE->alu);

	set_negative_flag_w(CPU_STATE->alu);
	set_zero_flag(CPU_STATE->alu);
}

void
SXT()
{
	NOP();
}

void
TSTB()
{
	setup_dest_addressing(1);

	CPU_STATE->alu = fetch_data(CPU_STATE->dest_address) & 0377;

	set_zero_flag(CPU_STATE->alu);
	set_negative_flag_b(CPU_STATE->alu);
}

void
TST()
{
	setup_dest_addressing(0);

	CPU_STATE->alu = fetch_data(CPU_STATE->dest_address);

	set_zero_flag(CPU_STATE->alu);
	set_negative_flag_w(CPU_STATE->alu);
}

void
XOR()
{
	NOP();
}

void
cleanup()
{
	if (CPU_STATE != NULL) {
		printf("FINAL STATE\n");
		printf("-----------\n");
		dump_cpu(CPU_STATE);
		free(CPU_STATE);
		CPU_STATE = NULL;
	}

	/* Signal and join bus threads. */
	if (BUS_REF != NULL) {
		cleanup_bus(BUS_REF);
		BUS_REF = NULL;
	}
}

void
handler(int signo, siginfo_t *info, void *context)
{
	cleanup();
}


int main(int argc, char **argv)
{
	int ret, opt;
	char *sock_l = NULL;
	char *sock_name = "cpu";
	char *sock_r = "mem";
	struct sigaction act;

	/* Setup signal handler. */
	act.sa_flags = SA_SIGINFO;
	act.sa_sigaction = &handler;
	if (sigaction(SIGHUP, &act, NULL) == -1) {
		perror("sigaction");
		exit(EXIT_FAILURE);
	}
	if (sigaction(SIGINT, &act, NULL) == -1) {
		perror("sigaction");
		exit(EXIT_FAILURE);
	}

	pid_t pid = getpid();
	fprintf(stderr, "cpu is starting [%d]\n", pid);

	/* Initialize CPU. */
	init_cpu();
	if (CPU_STATE == NULL) {
		fprintf(stderr, "failed to initialize CPU\n");
		exit(EXIT_FAILURE);
	}

	while ((opt = getopt(argc, argv, "p:l:r:")) != -1) {
		switch (opt) {
		case 'p':
			CPU_STATE->pc = strtoul(optarg, NULL, 10);
			break;
		default:
			usage();
			free(CPU_STATE);
			exit(EXIT_FAILURE);
		}
	}

	/* Initialize bus connections. */
	BUS_REF = init_bus(sock_l, sock_name, sock_r);
	if (BUS_REF == NULL) {
		free(CPU_STATE);
		fprintf(stderr, "failed to initialize bus\n");
		exit(EXIT_FAILURE);
	}

	/* Attempt to connect to bus. */
	fprintf(stderr, "cpu connecting to bus...\n");
	ret = connect_cpu_bus(BUS_REF);
	if (ret != 0) {
		free(CPU_STATE);
		free(BUS_REF);
		fprintf(stderr, "failed to initialize bus connections\n");
		exit(EXIT_FAILURE);
	}

	fprintf(stderr, "cpu connected\n");
	execute();

	cleanup();

	return 0;
}

