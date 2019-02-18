#include <cstdlib>
#include <new>
#include <cstdint>

int main()
{
	auto align = new(std::align_val_t(4)) uint16_t;
	operator delete(align, std::align_val_t(4));

	auto p = malloc(1024);
	if (p) {
		free(p);
	}

	auto p1 = new int;
	if (p1) {
		delete p1;
	}

	auto p2 = new int[32];
	if (p2) {
		delete[] p2;
	}
	return 0;
}
