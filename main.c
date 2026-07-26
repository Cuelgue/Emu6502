#include <stdio.h>
#include <stdint.h>

typedef enum {
    M_CARRY = 1,
    M_ZERO,
    M_INTER = 4,
    M_DEC = 8,
    M_BREAK = 16,
    M_IGNOR = 32,
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
    ram[cpu->reg.sp] = (cpu->reg.pc & 0xFF00) >> 8;
    cpu->reg.sp--;
    ram[cpu->reg.sp] = (cpu->reg.pc & 0xFF);
    cpu->reg.sr &= M_INTER;
    cpu->reg.sp--;
    ram[cpu->reg.sp] = cpu->reg.sr;
    cpu->reg.pc = (ram[0xFFFF] << 8) | ram[0xFFFE];

  
}


/* La idea es implementar las instrucciones
   fundamentales y en otras funciones llamarlas
   según el modo de direccionamiento.*/

void adc(uint8_t *ac, uint8_t value, uint8_t *sr)
{  
    uint16_t resul = *ac + value + (*sr & M_CARRY);
    uint8_t carry = (resul & 0x100) >> 8;
    *sr = *sr & 0xFE | carry; 
    if (*sr == 0) {
        *sr |= M_ZERO;
    }
    *sr &= (resul & M_NEG);
    //TODO: falta setear el flag de overflow.
    *ac = resul & 0xFF;
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
