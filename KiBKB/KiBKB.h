/*
 * User defined literals for bytes scaling to Kib/KB, Mib/MB, etc
 */

#pragma once

constexpr uint64_t operator "" _KiB(unsigned long long int bytes)
{
	return bytes * 1024;
}

constexpr uint64_t operator "" _MiB(unsigned long long int bytes)
{
	return bytes * 1024_KiB;
}

constexpr uint64_t operator "" _GiB(unsigned long long int bytes)
{
	return bytes * 1024_MiB;
}

constexpr uint64_t operator "" _TiB(unsigned long long int bytes)
{
	return bytes * 1024_GiB;
}

constexpr uint64_t operator "" _KB(unsigned long long int bytes)
{
	return bytes * 1000;
}

constexpr uint64_t operator "" _MB(unsigned long long int bytes)
{
	return bytes * 1000_KB;
}

constexpr uint64_t operator "" _GB(unsigned long long int bytes)
{
	return bytes * 1000_MB;
}

constexpr uint64_t operator "" _TB(unsigned long long int bytes)
{
	return bytes * 1000_GB;
}