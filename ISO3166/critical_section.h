#pragma once
class critical_section
{
public:
	critical_section(
		void
		)
	{
		InitializeCriticalSectionAndSpinCount(&m_cs, 5000);
	}

	virtual ~critical_section(
		void
		)
	{
		DeleteCriticalSection(&m_cs);
	}

	inline void lock()
	{
		EnterCriticalSection(&m_cs);
	}

	inline void unlock()
	{
		LeaveCriticalSection(&m_cs);
	}
private:
	CRITICAL_SECTION m_cs;
};

