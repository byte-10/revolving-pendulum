#include <stdio.h>
#include <math.h>

int main()
{
	int step = 25;
	const int maxv = 150;
	for (int i = 0; i < maxv; i++) {
		float rad = i / float(maxv) * (8.f * 3.14159f);
		constexpr float range = 100.f;
		printf("%d ", int(roundf((sinf(rad) * range) / step) * step));
	}
	return 0;
}