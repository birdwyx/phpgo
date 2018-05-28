#pragma once

namespace co
{

#define __interface__ struct

__interface__ Freeable{
	virtual void Free() = 0;
};

class FreeableImpl : public Freeable{
public:
	ALWAYS_INLINE void Free(){
		delete this;
	}
	
	virtual ~FreeableImpl(){
	}
};

}