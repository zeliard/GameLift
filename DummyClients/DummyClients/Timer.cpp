#include "stdafx.h"
#include "Timer.h"
#include "ThreadLocal.h"
#include "JobDispatcher.h"


Timer::Timer()
{
	LTickCount = GetTickCount64();
}


void Timer::PushTimerJob(AsyncExecutable* obj, uint32_t after, JobEntry* task)
{
	obj->AddRefForThis(); ///< for timer

	int64_t dueTimeTick = after + LTickCount;

	mTimerJobQueue.push(TimerJobElement(obj, task, dueTimeTick));
}


void Timer::DoTimerJob()
{
	/// thread tick update
	LTickCount = GetTickCount64();

	while (!mTimerJobQueue.empty())
	{
		const TimerJobElement& timerJobElem = mTimerJobQueue.top();

		if (LTickCount < timerJobElem.mExecutionTick)
			break;

		AsyncExecutable* owner = timerJobElem.mOwner; ///< 'cause mOwner can be changed in DoTask..
		owner->DoTask(timerJobElem.mTask); ///< pass to the dispatcher
		owner->ReleaseRefForThis(); ///< for timer

		mTimerJobQueue.pop();
	}
}

