#include <stdio.h>
#include <stdint.h>

typedef enum {
    OR_NULO = 0, //agregado solo para la funcion Overflow 
    OR_CARRY = 1,
    OR_ZERO,
    OR_INTER = 4,
    OR_DEC = 8,
    OR_BREAK = 16,
    OR_IGNOR = 32,
    OR_OVERF = 64,
    OR_NEG = 128,
} or_mask_t;

// Aplicando un & resetea los flag correspondientes.
typedef enum {
    RESET_NEG = 127,
    RESET_OVERF = 191,
    RESET_BREAK = 239,
    RESET_DEC = 247,
    RESET_INTER = 251,
    RESET_ZERO = 253,
    RESET_CARRY = 254,
} resetF_t;


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
    cpu->reg.sr &= OR_INTER;
    cpu->reg.sp--;
    ram[cpu->reg.sp] = cpu->reg.sr;
    cpu->reg.pc = (ram[0xFFFF] << 8) | ram[0xFFFE];

  
}


/* La idea es implementar las instrucciones
   fundamentales y en otras funciones llamarlas
   según el modo de direccionamiento.*/

/* Una manera muy torpe, pero deberia resolver.
     al final resolver es lo primero, mejorar es lo siguiente.*/
or_mask_t overflow(uint8_t ac,uint8_t value, uint8_t carry)
{
    
    or_mask_t resul = OR_NULO;
    uint8_t sum = ac + value;
    /*Acá lo importante es que para la suma tengo Overflow
     si al sumar dos números negativos obtengo uno positivo
    y si sumo dos números positivos obtengo uno negativo.*/
    if (ac <= 127 && value <= 127) {
        if (sum >= 128) {
            resul = OR_OVERF;
        } else if (sum + carry == 128) resul = OR_OVERF;
    } else if (ac >= 128 && value >= 128) {
                if (sum <= 127) {
                    resul = OR_OVERF;
                } else if (sum + carry <= 127) resul = OR_OVERF;
    }
    
    return resul;
}

/* En caso de que el valor no sea cero
   el valor que retorna es cero. Si value
   es cero retor OR_ZERO que es igual a 2
   este valor sirve como mascara para el registro
   de estado */
or_mask_t zero(uint8_t value)
{
    or_mask_t resul = OR_NULO;
    if ((value & 0xFF) == 0) resul = OR_ZERO;
    return resul;
}

void adc(uint8_t *ac, uint8_t value, uint8_t *sr)
{  
    uint16_t resul = *ac + value + (*sr & OR_CARRY);
    uint8_t carry = (resul & 0x100) >> 8;
    *sr = *sr & 0xFE | carry; 
 
    (*sr) |= zero(resul);
 
    *sr &= (resul & OR_NEG);
    //TODO: falta setear el flag de overflow.
    *sr |= overflow(*ac,value,carry);
    
    *ac = resul & 0xFF;
}


void cmp(uint8_t ac,uint8_t value,uint8_t *sr)
{
    uint8_t resul = ac - value;
    (*sr) &= 0xFD;
    (*sr) |= zero(resul);
    (*sr) &= (OR_NEG - 1);
    (*sr) |= resul & OR_NEG;
    if (resul >= 0) (*sr) |= OR_CARRY;
    else (*sr) &= 0xFE;
}


/* A diferencia dea cpm esta instruccion
   trabaja con los registros X e Y ademas
   usa una direccion de memoria para acceder
   a un valor po lo demas se comporta como CMP*/
void cpmIndex(uint8_t reg,uint8_t *sr,uint8_t ram[],uint16_t dir)
{
    uint8_t value = ram[dir];
    cmp(reg,value,sr);
}


/* Overflow en caso de la resta, si mis operandos
   tienen distinto signo y el resultado tiene distinto
   signo que el acumulador tengo Overflow
   como zero o overflow este devuelve OR_ZERO en caso
   de que no haya overflow y OR_OVERF en caso de que lao haya*/
or_mask_t sbc_overf(uint8_t sR,uint8_t sA,uint8_t sMC)
{
    or_mask_t result = OR_ZERO;
    if (sA != sMC && sA != sR) result = OR_OVERF;
    return result;
}


/*FIXME: Puede que convenga separar el reset de
  flag en un procedimiento. Ademas el reset debe afectar
  tambien a acd, por lo que asi como lo tengo puede que adc
  este mal implementado.*/
void sbc(uint8_t *ac,uint8_t value,uint8_t *sr)
{
    /* Salvo que este pasando por alto algún capricho de C
     esde deberia permitirme quedarme con el opuesto del carry */
    uint8_t compl = ~(*sr & OR_CARRY);
    if (*ac >= (value + compl)) (*sr) |= OR_CARRY;
    else (*sr) &= 0xFE;
    uint16_t resul = *ac - value - compl;
    (*sr) &= RESET_ZERO;
    (*sr) |= zero(resul);
    (*sr) &= RESET_NEG;
    (*sr) |= (resul & OR_NEG);
    (*sr) &= RESET_OVERF;
    uint8_t sResul = resul & OR_NEG;
    uint8_t sAcu = *ac & OR_NEG;
    uint8_t sM = (value + compl) & OR_NEG;
    (*sr) |= sbc_overf(sResul,sAcu,sM);
    
}

/* fetch_inst lo que hace es cargar en el tercer parametro
   el contenido de ram en la posicion pc y auamenta pc
   por lo que para esta instruccion tambien deberia ser válido
   con la diferencia de que acá se carga el registro acumulador*/
void lda(uint8_t *ac,uint8_t *sr,uint16_t *pc,uint8_t ram[])
{
    
    fetch_inst(ram, pc, ac);
    (*sr) &= RESET_ZERO;
    (*sr) |= zero(*ac);
}


int main()
{

    printf("%d\n", overflow(127,127,0));
    printf("%d\n", overflow(127,127,1));
    printf("%d\n", overflow(64,63,0));    
    printf("%d\n", overflow(64,63,1));
    printf("%d\n", overflow(63,63,1));    
    printf("%d\n", overflow(192,255,0));
    printf("%d\n", overflow(128,255,1));


    // 11111100
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
