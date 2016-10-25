#include <StdInc.h>
#include <AyriaExtension.h>



static InitFunction initFunction([] ()
{
	InitStartEvent.Connect([] ()
	{
		
	}, -1000);
});