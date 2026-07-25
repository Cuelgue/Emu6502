#include <stdio.h>
#include <stdint.h>

typedef enum {
    M_CARRY = 1,
    M_ZERO,
    M_INTER = 4,
    M_DEC = 8,
    M_BREAK = 16,
    M_OVERF = 64,
    M_NEG = 128,
} mask_t;

typedef struct {
   uint16_t pc; 
   /* Puede que sea mejor un array pero voy a dejarlo separado con sus nombres */
   uint8_t ac;
   uint8_t x;
   uint8_t y;
   /* Status register 

    - : significa que ese bit se ignora.
    Negative | Overflow | - | Break | Decimal (use BCD for arimetic) | Interrump | Zero | Carry */
   uint8_t sr; 
   uint8_t sp;
} reg_t;

typedef struct {
    uint8_t inst;
    reg_t reg;
} cpu_t;

void fetch_inst(uint8_t *ram,uint16_t *pc,uint8_t *inst)
{
   *inst = ram[*pc];
   (*pc)++;
}



void brk(cpu_t *cpu, uint8_t *ram)
{
  /* Aclaracion: break suma 2 al program counter, pero
     yo ya estoy sumando 1 en fetch_inst por lo quea con
     sumarle uno mas esta abien.                        */
  (cpu->reg.pc)++;
  cpu->reg.sp--;
  cpu->reg.sp = (cpu->reg.pc & 0xFF00) >> 8;
  cpu->reg.sp--;
  cpu->reg.sp = (cpu->reg.pc & 0xFF);
  cpu->reg.sr &= M_INTER;
  cpu->reg.sp--;
  cpu->reg.sp = cpu->reg.sr;
  cpu->reg.sp = (ram[0xFFFF] << 8) | ram[0xFFFE];

  
}

void adc()
{
    
}


int main()
{
    printf("%d\n", 0 | M_NEG);
    return 0;
}



/*
#define MAX_ARR_FUN 167

// No se cual es el nombre que mejor le va
typedef int (*fn_t)(cpu_t *cpu);

typedef struct {
    // Opcode como Key
    uint8_t key;
    // Funcion a cargar/ejecutar según opcode
    fn_t fun;  
}kv_t;

struct nodo_t;

typedef struct nodo_t *list_t;

typedef struct {
    kv_t kv;
    list_t sig;
} nodo_t;



int hash(uint8_t inst)
{
    return inst % MAX_ARR_FUN;
}

*/
