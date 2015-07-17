#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

int main(int argc, char const *argv[])
{
	uint32_t i = 11664000;
	uint32_t y = i * 100;
	int normal = (int) i;
	printf("%d\n", y);
	return 0;
}