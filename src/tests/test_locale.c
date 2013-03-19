#include <locale.h>
#include <stdio.h>
#include <errno.h>

int main(int argc, char *argv[]) {


printf("%08X\n", setlocale(LC_ALL, ""));
printf("%s\n", strerror(errno));
//es_ES@euro"));

printf("%c\n", tolower('Á'));

return 0;
}
