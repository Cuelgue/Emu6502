#include <stdio.h>
#define IMPLEMENTATION
#include "cpu6502.h"
#include <stdint.h>
#include <assert.h>

//TODO: Preparar test para las funcioanes implementadas.

int main()
{

    printf("%d\n", overflow(127,127,0));
    printf("%d\n", overflow(127,127,1));
    printf("%d\n", overflow(64,63,0));
    printf("%d\n", overflow(64,63,1));
    printf("%d\n", overflow(63,63,1));
    printf("%d\n", overflow(192,255,0));
    printf("%d\n", overflow(128,255,1));
    uint8_t a = 0 - 1;
    printf("%d\n",a);
    uint16_t pc = 0;
    int8_t sr = 25;
    uint16_t dir = 17999;
    bcc(dir,sr,&pc);
    assert(pc == dir && "Malio Sal");
    printf("decime que es lo que pasa\n");
    printf("%d\n",pc);
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
