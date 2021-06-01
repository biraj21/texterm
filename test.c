// Program to print Fibonacci numbers
#include <stdio.h>

int main()
{
	int n;
	printf("Enter a number: ");
	scanf("%d", &n);

	int a = 0, b = 1;
	for (int i = 0; i < n; ++i)
	{
		printf("%d ", a);
		int c = a + b;
		a = b;
		b = c;
	}

	putchar('\n');
	return 0;
}
