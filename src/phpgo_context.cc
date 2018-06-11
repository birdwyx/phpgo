#include "stdinc.h"
#include "php_phpgo.h"
#include "go.h"
#include "phpgo_context.h"

// the scheduler may be executed in multiple thread: 
// use thread local variable to store the scheduler EG's	
thread_local PhpgoSchedulerContext scheduler_ctx;