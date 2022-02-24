
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#include "macros.h"



//#define fgetc(f) fgetc(f);printf("%c",c);


char *get_token0( FILE *f , char *token , int token_len ){

  int c;
  char *t=token;
  
restart:
  // skip leading spaces
  while(1){
    c = fgetc(f);
    if(c==EOF) return 0;
    if(!isspace(c))break;
  }

  // qui c != [space]

  if(c=='#'){ // commento?
    // butta tutto fino a fine linea
    while(1){
      c = fgetc(f);
      if(c==EOF) return 0;
      if(c=='\r')goto restart;
      if(c=='\n')goto restart;
    }
  }

  if( c=='"' || c=='\'' || c=='`' ){ // quot?
    // mangia tutto fino alla chiusura
    // il token è la frase tra quot
    char quot=c;
    while(1){
      c = fgetc(f);
      if(c==EOF)break;  // sgrammaticato, ma chissene
      if(c==quot)break;
      *t++ = c;
    }
    *t = 0;
    return token;
  }

  // token isolato
  while(1){
    *t++ = c;
    c = fgetc(f);
    if(c==EOF)break;
    if(isspace(c))break;
  }
  *t = 0;

  return token;
}


char *get_token( FILE *f , char *token , int token_len ){
  if(!get_token0(f,token,token_len))return 0;
  if(0!=strcmp(token,"end"))return token;
  fseek(f,0,SEEK_END);
  return 0;
}



int main( int argc , char *argv[] ){
  FILE *f = fopen("test.am","rb");
  assert(f);
  while(1)
  {
    char token[256];
    if(!get_token(f,token,sizeof(token)))break;
    printf("%s\n",token);
  }
  fclose(f);
  return 0 ;
}


