
#include "Scheduler.h"
#include "ClientSession.h"

std::unique_ptr<Scheduler> GScheduler(nullptr);

Scheduler::Scheduler()
{
	mBeginTickPoint = Clock::now();
	mCurrentTick = GetCurrentTick();
}

void Scheduler::PushTask(SessionPtr owner, Task&& task, uint32_t after)
{
	int64_t dueTimeTick = after + mCurrentTick;
	
	mTaskQueue.push(JobElement(owner, std::move(task), dueTimeTick));
}


void Scheduler::DoTasks()
{
	/// tick update
	mCurrentTick = GetCurrentTick();

	while (!mTaskQueue.empty())
	{
		JobElement timerJobElem = mTaskQueue.top();

		if (mCurrentTick < timerJobElem.mExecutionTick)
			break;

		/// do task!
		timerJobElem.mTask();

		mTaskQueue.pop();
	}
}
