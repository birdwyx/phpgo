/*
 *  Copied from https://github.com/sismvg/___defer/tree/master/just_playing
 */
#ifndef __DEFER_H__
#define __DEFER_H__

#include <functional>

/*
* internal defer implementation, not expected to be used directly
*/
namespace _defer
{
	class __defer
	{

	public:
		__defer(){}
		~__defer(){ _fn(); }
		
		__defer(const __defer&)                  = delete;
		__defer*       operator &()              = delete;
		const __defer* operator &() const        = delete;
		__defer&       operator=(const __defer&) = delete;
		
		template<class Func>
		inline void operator << (Func fn){
			_fn = fn;
		}
		
		template<class Func,class... Arg>
		inline void operator()(Func fn, Arg... arg){
			_fn=std::bind(fn, arg...);
		}
		
	private:
		std::function<void(void)> _fn;
		static void* operator new(std::size_t);
		static void operator delete(void*);
	};
}

#define _DEFER_MAKE_NAME_IMPL(line) __defer_##line
#define _DEFER_MAKE_NAME(line) _DEFER_MAKE_NAME_IMPL(line)
#define _DEFER_RANDOM_OBJECT(class, line) class _DEFER_MAKE_NAME(line)

/*
* application should use defer / defer_call instead of using __defer directly
*/
#define defer \
	_DEFER_RANDOM_OBJECT(::_defer::__defer, __LINE__); \
	_DEFER_MAKE_NAME(__LINE__) << [&](void)
#define defer_call \
	_DEFER_RANDOM_OBJECT(::_defer::__defer, __LINE__);\
	_DEFER_MAKE_NAME(__LINE__)

#endif
