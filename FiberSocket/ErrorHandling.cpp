#include <stdexcept>
#include <cstring>
#include "ErrorHandling.h"

void CheckErrno(int err)
{
	if (err != 0) {
		throw std::runtime_error("Function call failed! Reason: " + std::string(strerror(errno)));
	}
}