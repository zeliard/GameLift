#include "CircularBuffer.h"
#include <assert.h>
#include <string.h>


bool CircularBuffer::Peek(OUT char* destbuf, size_t bytes) const
{
	assert( mBuffer != nullptr );

	if( mARegionSize + mBRegionSize < bytes )
		return false ;

	size_t cnt = bytes;
	size_t aRead = 0;

	if ( mARegionSize > 0 )
	{
		aRead = (cnt > mARegionSize) ? mARegionSize : cnt;
		memcpy(destbuf, mARegionPointer, aRead);
		cnt -= aRead;
	}

	if ( cnt > 0 && mBRegionSize > 0 )
	{
		assert(cnt <= mBRegionSize);

		size_t bRead = cnt;

		memcpy(destbuf+aRead, mBRegionPointer, bRead);
		cnt -= bRead;
	}

	assert( cnt == 0 );

	return true;

}

bool CircularBuffer::Read(OUT char* destbuf, size_t bytes)
{
	assert( mBuffer != nullptr );

	if( mARegionSize + mBRegionSize < bytes )
		return false;

	size_t cnt = bytes;
	size_t aRead = 0;

	if ( mARegionSize > 0 )
	{
		aRead = (cnt > mARegionSize) ? mARegionSize : cnt;
		memcpy(destbuf, mARegionPointer, aRead);
		mARegionSize -= aRead;
		mARegionPointer += aRead;
		cnt -= aRead;
	}
	
	if ( cnt > 0 && mBRegionSize > 0 )
	{
		assert(cnt <= mBRegionSize);

		size_t bRead = cnt;

		memcpy(destbuf+aRead, mBRegionPointer, bRead);
		mBRegionSize -= bRead;
		mBRegionPointer += bRead;
		cnt -= bRead;
	}

	assert( cnt == 0 );

	if ( mARegionSize == 0 )
	{
		if ( mBRegionSize > 0 )
		{
			if ( mBRegionPointer != mBuffer )
				memmove(mBuffer, mBRegionPointer, mBRegionSize);

			mARegionPointer = mBuffer;
			mARegionSize = mBRegionSize;
			mBRegionPointer = nullptr;
			mBRegionSize = 0;
		}
		else
		{
			mBRegionPointer = nullptr;
			mBRegionSize = 0;
			mARegionPointer = mBuffer;
			mARegionSize = 0;
		}
	}

	return true;
}




bool CircularBuffer::Write(const char* data, size_t bytes)
{
	assert( mBuffer != nullptr );

	if( mBRegionPointer != nullptr )
	{
		if ( GetBFreeSpace() < bytes )
			return false;

		memcpy(mBRegionPointer + mBRegionSize, data, bytes);
		mBRegionSize += bytes;

		return true;
	}

	if ( GetAFreeSpace() < GetSpaceBeforeA() )
	{
		AllocateB();

		if ( GetBFreeSpace() < bytes )
			return false;

		memcpy(mBRegionPointer + mBRegionSize, data, bytes);
		mBRegionSize += bytes;

		return true;
	}
	else
	{
		if ( GetAFreeSpace() < bytes )
			return false;

		memcpy(mARegionPointer + mARegionSize, data, bytes);
		mARegionSize += bytes;

		return true;
	}
}


void CircularBuffer::Remove(size_t len)
{
	size_t cnt = len;
	
	if ( mARegionSize > 0 )
	{
		size_t aRemove = (cnt > mARegionSize) ? mARegionSize : cnt;
		mARegionSize -= aRemove;
		mARegionPointer += aRemove;
		cnt -= aRemove;
	}

	if ( cnt > 0 && mBRegionSize > 0 )
	{
		size_t bRemove = (cnt > mBRegionSize) ? mBRegionSize : cnt;
		mBRegionSize -= bRemove;
		mBRegionPointer += bRemove;
		cnt -= bRemove;
	}

	if ( mARegionSize == 0 )
	{
		if ( mBRegionSize > 0 )
		{
			/// 앞으로 당겨 붙이기
			if ( mBRegionPointer != mBuffer )
				memmove(mBuffer, mBRegionPointer, mBRegionSize);
	
			mARegionPointer = mBuffer;
			mARegionSize = mBRegionSize;
			mBRegionPointer = nullptr;
			mBRegionSize = 0;
		}
		else
		{
			mBRegionPointer = nullptr;
			mBRegionSize = 0;
			mARegionPointer = mBuffer;
			mARegionSize = 0;
		}
	}
}


