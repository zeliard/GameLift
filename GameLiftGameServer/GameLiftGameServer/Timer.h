#pragma once
#include "FastSpinlock.h"

class Session;
typedef std::function<void()> TimerTask;
typedef std::shared_ptr<Session> SessionPtr;

struct TimerJobElement
{
	TimerJobElement(SessionPtr owner, const TimerTask& task, int64_t execTick)
	: mOwner(owner), mTask(task), mExecutionTick(execTick)
	{}

	SessionPtr  mOwner;
	TimerTask	mTask;
	int64_t		mExecutionTick;
};

struct TimerJobComparator
{
	bool operator()(const TimerJobElement& lhs, const TimerJobElement& rhs)
	{
		return lhs.mExecutionTick > rhs.mExecutionTick;
	}
};


typedef std::priority_queue<TimerJobElement, std::vector<TimerJobElement>, TimerJobComparator> TimerJobPriorityQueue;

class Timer
{
public:

	Timer();

	void PushTimerJob(SessionPtr owner, const TimerTask& task, uint32_t after);

	void DoTimerJob();

private:

	TimerJobPriorityQueue	mTimerJobQueue;

	FastSpinlock	mLock;

};
