#pragma once

#ifdef ZTS
#define THREAD_LOCAL thread_local
#else
#define THREAD_LOCAL
#endif

#define PHPGO_MAX_GO_ARGS  (256 + 2)  // goo($option, $callable, $arg1,...,arg256);