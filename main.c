#include <stdio.h>
#include <stdint.h>

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

int main()
{
    printf("Hola aqui estamos de nuevo");
    return 0;
}
