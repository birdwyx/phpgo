#include "go_timer.h"
#include "defer.h"
#include "zend_API.h"
#include "go_chan.h"
#include <libgo/coroutine.h>

void GoTimer::Tick(zval* z_chan, void* h_chan, uint64_t micro_seconds TSRMLS_DC){
	// must add ref the z_chan so that it won't be release before the go routine returns
	zval_add_ref(&z_chan);
	go_stack(4*1024) [=] ()mutable {
		defer{
			zval_ptr_dtor(&z_chan);   //release the z_chan even if the followed loop exits on exception
		};
		while(true){
			zval* z;
			MAKE_STD_ZVAL(z);
			ZVAL_LONG(z, 1);
			
			usleep(micro_seconds);
			GoChan::Push(h_chan, z TSRMLS_CC);
		}
	};
}

void GoTimer::After(zval* z_chan, void* h_chan, uint64_t micro_seconds TSRMLS_DC){
	// must add ref the z_chan so that it won't be release before the go routine returns
	zval_add_ref(&z_chan);
	go_stack(4*1024) [=] ()mutable {
		defer{
			zval_ptr_dtor(&z_chan);   //release the z_chan even if the go routine exits on exception
		};
		zval* z;
		MAKE_STD_ZVAL(z);
		ZVAL_LONG(z, 1);
		
		usleep(micro_seconds);
		GoChan::Push(h_chan, z TSRMLS_CC);
	};
}
