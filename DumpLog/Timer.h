/************************************************************
 * TestLogService.h
 * Generic Timer Implementation
 * Version History:
 * Author				Date		Version  What was modified?
 * SAFE	Engineering		26th Mar 2018	0.0.5    Official Release to Aurzion
 ************************************************************/

#ifndef __TIMER_H_
#define __TIMER_H_

#include <asio.hpp>

namespace aurizon {

/**
 * A wrapper for asio::basic_waitable_timer to make it more robust.
 * When a timer completes, the WaitHandler is added to the io_service queue. If the timer is
 * cancelled before the handler is called it will complete as if it was never cancelled. This
 * wrapper will return an asio::error::operation_aborted in the handler if it is cancelled after
 * the handler has been posted to the queue.
 */
template <typename Clock>
class Timer {
public:
	Timer(asio::io_service& io_service);
	virtual ~Timer();

	typedef asio::basic_waitable_timer<Clock> timer_type;

	std::size_t cancel(std::error_code& ec);

	std::size_t expires_at(const typename timer_type::time_point& expiry_time, std::error_code& ec);
	std::size_t expires_after(const typename timer_type::duration& expiry_time, std::error_code& ec);

	template <typename WaitHandler>
	void async_wait(WaitHandler&& handler);

private:
	timer_type asioTimer;

	uint32_t runCount = 0;      //<! The number of current wait handlers
	uint32_t cancelCount = 0;   //<! The number of handlers that need to be cancelled before go back to normal
};

template <typename Clock>
Timer<Clock>::Timer(asio::io_service& io_service) : asioTimer(io_service)
{
}

template <typename Clock>
Timer<Clock>::~Timer()
{
}

template <typename Clock>
std::size_t Timer<Clock>::cancel(std::error_code& ec)
{
	std::size_t cancelled = asioTimer.cancel(ec);
	assert(cancelled <= runCount);
	cancelCount += runCount;
	runCount = 0;

	return cancelled;
}

template <typename Clock>
std::size_t Timer<Clock>::expires_at(const typename timer_type::time_point& expiry_time, std::error_code& ec)
{
	std::size_t cancelled = asioTimer.expires_at(expiry_time, ec);
	assert(cancelled <= runCount);
	cancelCount += runCount;
	runCount = 0;

	return cancelled;
}

template <typename Clock>
std::size_t Timer<Clock>::expires_after(const typename timer_type::duration& expiry_time, std::error_code& ec)
{
	std::size_t cancelled = asioTimer.expires_from_now(expiry_time, ec);
	assert(cancelled <= runCount);
	cancelCount += runCount;
	runCount = 0;

	return cancelled;
}

template <typename Clock>
template <typename WaitHandler>
void Timer<Clock>::async_wait(WaitHandler&& handler)
{
	runCount++;
	asioTimer.async_wait([this, handler](std::error_code ec)
	{
		if (cancelCount > 0)
		{
			if (ec != asio::error::operation_aborted)
			{
				ec = asio::error::operation_aborted;
			}
			cancelCount--;
		}
		else if (runCount > 0)
		{
			runCount--;
		}
		handler(ec);
	});
}

} /* namespace aurizon */

#endif /* __TIMER_H_ */
