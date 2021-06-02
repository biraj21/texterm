// program to print prime numbers
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

bool is_prime(int n);

int main()
{
	int n;
	printf("Enter a number: ");
	scanf("%d", &n);

	for (int i = 2; i <= n; ++i)
	{
		if (is_prime(i))
			printf("%d ", i);
	}

	putchar('\n');
	return 0;
}

bool is_prime(int n)
{
	for (int i = 2, s = sqrt(n); i <= s; ++i)
	{
		if (n % i == 0)
			return false;
	}

	return n > 1;
}
