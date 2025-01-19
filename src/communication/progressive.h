#ifndef PROGRESSIVE_H
#define PROGRESSIVE_H

namespace protocol
{
	template <typename T>
	class Progressive
	{
		T object;
		typename T::processing_stages status;

	public:
		std::mutex cv_m;
		std::condition_variable processing;

		const typename T::processing_stages& get_status() const { return status; }
	};
}  // namespace protocol

#endif  // PROGRESSIVE_H