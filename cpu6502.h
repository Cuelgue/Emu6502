#include <stdint.h>

#ifndef CPU6502_H

#define CPU6502_H
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
    RESET_NOZ = 61, //Reset Negative, Overflow y Zero.
    RESET_NO = 63, //Reset Negative y Overflow.
    RESET_NC = 124,//Reset Negative y carry.
    RESET_NZ = 125, //Reset Negative y Zero.
    RESET_NEG = 127,
    RESET_NCZ = 131,
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

void fetch_inst(uint8_t *ram,uint16_t *pc,uint8_t *inst);

void brk(cpu_t *cpu, uint8_t *ram);

or_mask_t overflow(uint8_t ac,uint8_t value, uint8_t carry);

or_mask_t zero(uint8_t value);

void adc(uint8_t *ac, uint8_t value, uint8_t *sr);

void cmp(uint8_t ac,uint8_t value,uint8_t *sr);

void cmp_index(uint8_t reg,uint8_t *sr,uint8_t ram[],uint16_t dir);

or_mask_t sbc_overf(uint8_t sR,uint8_t sA,uint8_t sMC);

void sbc(uint8_t *ac,uint8_t value,uint8_t *sr);

void load(uint8_t *ac,uint8_t *sr,uint16_t *pc,uint8_t ram[]);

void and(uint8_t *ac,uint8_t *sr,uint8_t value);

void asl(uint8_t *value,uint8_t *sr);

void bcc(uint16_t dir,int8_t sr,uint16_t *pc);

void bcs(uint16_t dir,int8_t sr,uint16_t *pc);

void beq(uint16_t dir, uint8_t sr, uint16_t *pc);

void bit(uint8_t value,uint8_t *sr, uint8_t acc );

void bmi(uint16_t dir, uint8_t sr, uint16_t *pc);

void bne(uint16_t dir, uint8_t sr, uint16_t *pc);

void bpl(uint16_t dir, uint8_t sr, uint16_t *pc);

avoid bvc(uint16_t dir, uint8_t sr, uint16_t *pc);

void bvs(uint16_t dir, uint8_t sr, uint16_t *pc);

void clear_flag(uint8_t *sr,resetF_t reset);

uint16_t normalizar_sp(uint8_t sp);

void dec(uint8_t *value, uint8_t *sr);

void eor(uint8_t value, uint8_t *acc, uint8_t *sr);

void inc(uint8_t *value, uint8_t *sr);

void jump(uint16_t dir, uint8_t *pc);

void jsr(uint16_t *pc,uint8_t *sp,uint16_t dir, uint8_t ram[]);

void lsr(uint8_t *value, uint8_t *sr);

void ora(uint8_t value, uint8_t *ac, uint8_t *sr);

void push_reg(uint8_t value, uint8_t ram[], uint8_t *sp);

void pull_reg(uint8_t *value, uint8_t ram[], uint8_t *sp);
#endif
#ifdef IMPLEMENTATION


uint16_t normalizar_sp(uint8_t sp)
{
    return 0x0100 | sp;

//TODO: Reemplazar uint8_t con iint8_t

void fetch_inst(uint8_t *ram,uint16_t *pc,uint8_t *inst)
{
    *inst = ram[*pc];
    (*pc)++;
}


/*Aprovechando que ya se tiene disponible resetF_t
  esta funcion resea el flog que recibe como parametro
  en reset*/
void clear_flag(uint8_t *sr,resetF_t reset)
{    /*inst clc: opcode 0x18. cld opcode 0xDB
       cli: opcode 0x58. clv: opcode 0xB8
       clc: reset carry. cld: reset decimal.
       cli: reset interrump. clv: reset Overflow
     */
    *sr = *sr & reset;
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
    clear_flag(sr,RESET_CARRY);
    *sr = *sr | carry;

    (*sr) |= zero(resul);

    *sr &= (resul & OR_NEG);
    //TODO: falta setear el flag de overflow.
    *sr |= overflow(*ac,value,carry);

    *ac = resul & 0xFF;
}


void cmp(uint8_t ac,uint8_t value,uint8_t *sr)
{
    uint8_t resul = ac - value;
    clear_flag(sr,RESET_NZ);
    (*sr) |= zero(resul);
    (*sr) |= resul & OR_NEG;
    if (resul >= 0) (*sr) |= OR_CARRY;
    else clear_flag(sr,RESET_CARRY);
}


/* A diferencia dea cmp esta instruccion
   trabaja con los registros X e Y ademas
   usa una direccion de memoria para acceder
   a un valor po lo demas se comporta como CMP*/
void cmp_index(uint8_t reg,uint8_t *sr,uint8_t ram[],uint16_t dir)
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


/*FIXME:Ademas el reset debe afectar
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
    clear_flag(sr,RESET_NOZ);
    (*sr) |= zero(resul);
    (*sr) |= (resul & OR_NEG);
    uint8_t sResul = resul & OR_NEG;
    uint8_t sAcu = *ac & OR_NEG;
    uint8_t sM = (value + compl) & OR_NEG;
    (*sr) |= sbc_overf(sResul,sAcu,sM);

}

/* fetch_inst lo que hace es cargar en el tercer parametro
   el contenido de ram en la posicion pc y auamenta pc
   por lo que para esta instruccion tambien deberia ser válido
   con la diferencia de que acá se carga el registro acumulador*/
void load(uint8_t *ac,uint8_t *sr,uint16_t *pc,uint8_t ram[])
{
    //Util para la instrucciones LDA,LDX y LDY
    fetch_inst(ram, pc, ac);
    clear_flag(sr,RESET_NZ);
    (*sr) |= zero(*ac);
    (*sr) |= (*ac & OR_NEG);
}


void and(uint8_t *ac,uint8_t *sr,uint8_t value)
{
    clear_flag(sr,RESET_NZ);
    *ac &= value;
    (*sr) |= zero(*ac);
    (*sr) |= (*ac & OR_NEG);
}


/* Desplaza a la izquierda un bit,
   el bit desplazado se convierte en carry
   y de setea el bit de negativo si como resualtado
   el valor debe interpretarse como tal.*/
void asl(uint8_t *value,uint8_t *sr)
{
    clear_flag(sr,RESET_NCZ);
    // Setea como carry el bit que va a ser desplazado.
    (*sr) |= ((*value & OR_NEG) >> 7);
    *value = *value << 1;
    (*sr) |= (*value & OR_NEG);
    (*sr) |= zero(*value);
}


void bcc(uint16_t dir,int8_t sr,uint16_t *pc)
{
    int8_t carry = sr & OR_CARRY;
    if (!carry) *pc = dir;
}

void bcs(uint16_t dir,int8_t sr,uint16_t *pc)
{
    int8_t carry = sr & OR_CARRY;
    if (carry) *pc = dir;
}

void beq(uint16_t dir, uint8_t sr, uint16_t *pc)
{
    if (zero(sr)) *pc = dir;
}

/*Value es el contenido de una direccion de memoria*/
void bit(uint8_t value,uint8_t *sr, uint8_t acc )
{
    uint8_t resul = value & acc;
    //FIXME: estoy repiendo mucho esto, tengo que mejorarlo.
    clear_flag(sr,RESET_NOZ);
    *sr |= zero(resul);
    *sr |= resul;
    *sr |= resul;
}

/*bmi,bne,bpl,bvc son esencialmente la misma opearacion
  dejo un FIXME para centraalizarlo en un opracion que reciva
  directamente el valor a procesar.*/
void bmi(uint16_t dir, uint8_t sr, uint16_t *pc)
{
    if (sr & OR_NEG == OR_NEG) {
        *pc = dir;
    }
}


void bne(uint16_t dir, uint8_t sr, uint16_t *pc)
{
    if(!zero(sr)) *pc = dir;
}


void bpl(uint16_t dir, uint8_t sr, uint16_t *pc)
{
    if (sr & OR_NEG == OR_NULO) {
        *pc = dir;
    }
}


void bvc(uint16_t dir, uint8_t sr, uint16_t *pc)
{
    if (sr & OR_OVERF) {
        *pc = dir;
    }
}

void bvs(uint16_t dir, uint8_t sr, uint16_t *pc)
{
    if (sr & OR_OVERF == OR_NULO) {
        *pc = dir;
    }
}



/* dec - dex - dey. Son la misma operacion
   con la diferencia que dec es un valor en
   memoria y dex e y son los registros*/
void dec(uint8_t *value, uint8_t *sr)
{
    clear_flag(sr,RESET_NO);
    (*value) -= 1;
    *sr = zero(*value) | *sr;
    *sr = (*value & OR_NEG) | *sr;

}

void eor(uint8_t value, uint8_t *acc, uint8_t *sr)
{
    clear_flag(sr,RESET_NO);
    *acc = *acc ^ value;
    *sr = zero(value) | *sr;
    *sr = (value & OR_NEG) | *sr;
}

/*La funcion realiza la mima operacioan tanto
 para INC como para INX y INY*/
void inc(uint8_t *value, uint8_t *sr)
{
    clear_flag(sr,RESET_NZ);
    (*value)++;
    (*sr) |= (*value & OR_NEG);
    (*sr) |= zero(*value);
}

/*Se supone que estoy recibiendo
 la direccion ya ensamblada.
la direccion esta constituida
como el primer byte como la parte
baja del program counter PC = XX + Primer Byte;
segundo byte como la parte alta
PC = Segundo Byte + XX <- (Primer Byte)*/
void jump(uint16_t dir, uint8_t *pc)
{
    *pc = dir;
}

/*TODO: Corroborar sp*/
void jsr(uint16_t *pc,uint8_t *sp,uint16_t dir, uint8_t ram[])
{
    (*sp)--;
    ram[normalizar_sp(*sp)] = (*pc & 0xFF00) >> 8;
    (*sp)--;
    ram[*sp] = *pc & 0xFF;
    *pc = dir;

}

/*Desplaza a la derecha a1 bit y setea flag
 carry como el valor de bit desplazado
negativo como cero y cero si el valor es cero
TODO: corroborar el tema de los reset.*/
void lsr(uint8_t *value, uint8_t *sr)
{
    clear_flag(sr,RESET_NCZ);
    (*sr) |= (*value & OR_CARRY); //Sete el valor del carry.
    (*value) >>= 1;
    (*sr) |= zero(*value);

}

/*La operacion NOT no hace nada, vamos a dejarla vacia.*/
void not()
{
}

void ora(uint8_t value, uint8_t *ac, uint8_t *sr)
{
    clear_flag(sr,RESET_NC);
    *ac = *ac ^ value;
    *sr |= zero(*ac);
    *sr |= (*ac & OR_NEG);

}

/* PHA - PHP.
 al final lo que hace es pusher un registro al stack
y decrementar el stack pointer.*/
void push_reg(uint8_t value, uint8_t ram[], uint8_t *sp)
{
    ram[normalizar_sp(*sp)] = value;
    (*sp)--;
}


/*PLA - PLP
 incrementa sp y inicializa value con el contenido
de ram en la posicion sp normalizada.*/
void pull_reg(uint8_t *value, uint8_t ram[], uint8_t *sp)
{
    (*sp)++;
    *value = ram[normalizar_sp(*sp)];
}



#endif
