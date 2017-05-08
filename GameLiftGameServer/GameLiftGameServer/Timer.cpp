#include "stdafx.h"
#include "ThreadLocal.h"
#include "Exception.h"
#include "Session.h"
#include "Timer.h"



Timer::Timer()
{
	LTickCount = GetTickCount64();
}


void Timer::PushTimerJob(SessionPtr owner, const TimerTask& task, uint32_t after)
{
	CRASH_ASSERT(LThreadType == THREAD_IO_WORKER);

	FastSpinlockGuard writeLock(mLock);

	int64_t dueTimeTick = after + LTickCount;

	mTimerJobQueue.push(TimerJobElement(owner, task, dueTimeTick));
}


void Timer::DoTimerJob()
{
	bool isExclusive = false;
	FastSpinlockGuard readLock(mLock, isExclusive);

	/// thread tick update
	LTickCount = GetTickCount64();

	while (!mTimerJobQueue.empty())
	{
		TimerJobElement timerJobElem = mTimerJobQueue.top();

		if (LTickCount < timerJobElem.mExecutionTick)
			break;

		timerJobElem.mTask();

		mTimerJobQueue.pop();
	}
}

