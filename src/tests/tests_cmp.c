
#include <stdio.h>

int main(int argc, char *argv[]) {
char *a=":SaTiRoWit!jirc@ACTrbs.DqGwLf.virtual PRIVMSG #trivia : 11,0`%0,11%,12,11`%11,12%,2,12`%12,2%,8,2 wIt TrIvIa 12,2`%2,12%,11,12`%12,11%,0,11`%11,0%, 5 Pregunta 23/30. TEMA: Historia.";
int k=0;

k++;
// FFFFFFC3 FFFFFFCA FFFFFFC6
while ( (a[k] != ':') && (k<(strlen(a))) ) { printf("(%s)\n", a+k); k++;}

// && (k<length) ) {k++; ioutput( OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "2 RPD (%s)", pregunta+k)
//        k+=2; //strip also the color code
	
//    printf("%08X %d\n", strstr(a, "tarivia"), strstr(a, "trivia"));


    return 0;
}
