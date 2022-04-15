#include <stdio.h>

int main()
{
	int a;
	int b;
	int c;
	int d;

	scanf("%d", &a);
	scanf("%d", &b);

	c = 2097;
	d = 600;

	if ((a <= b) && (b <= c))
		d = d - 89 + c;
	else
		d = d - c * 3;

	printf("%d\n", d);
}
