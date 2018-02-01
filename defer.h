#ifndef ISU_DEFER_HPP
#define ISU_DEFER_HPP

#include <functional>

namespace isu
{
	class mynop
	{
	public: template<class... Arg> mynop(Arg... arg){}
	};

	class mydefer
	{

	public:
		mydefer()
		{}
		~mydefer()
		{_fn();}
		mydefer(const mydefer&) = delete;
		mydefer* operator &() = delete;
		const mydefer* operator &() const = delete;
		mydefer& operator=(const mydefer&) = delete;
		template<class Func>
		void operator -(Func fn)
		{
			_fn = fn;
		}
		template<class Func,class... Arg>
		void operator()(Func fn, Arg... arg)
		{
			_fn=std::bind(fn, arg...);
		}
	private:
		std::function<void(void)> _fn;
		static void* operator new(std::size_t);
		static void operator delete(void*);
	};

#define MAKE_NAME_IMPL(name,line) __JOJO## line
#define MAKE_NAME(name,line) MAKE_NAME_IMPL(name,line)

#define RANDOM_OBJECT(name,line) name MAKE_NAME(name,line) 

#ifndef DISABLE_MYDEFER
#define defer \
	RANDOM_OBJECT(isu::mydefer, __LINE__); \
	MAKE_NAME(isu::mydefer, __LINE__) - [&](void)
#define defer_fn \
	RANDOM_OBJECT(isu::mydefer, __LINE__);\
	MAKE_NAME(isu::mydefer, __LINE__)
#else
#define defer [=](void)
#define defer_fn mynop
#endif

}

#endif
