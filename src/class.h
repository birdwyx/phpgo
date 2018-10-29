#pragma once

#define CLASS(__class) struct __class 
#define DEFINE_CLASS_FUNC(__class, __func) void __##__class##__func(struct __class* __this)
#define DEFINE_EXTERN_CLASS_FUNC(__class, __func) extern void __##__class##__func(struct __class* __this)
#define DECLARE_CLASS_FUNC(__class, __func) void (*__func)(struct __class* __this)
#define CLASS_FUNC(__class, __func) __##__class##__func
#define NEW(__class) ( ( CLASS(__class)* )malloc( sizeof(CLASS(__class)) ) )