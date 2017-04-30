#ifndef __TKE_STARTUPBOARD__
#define __TKE_STARTUPBOARD__

namespace tke
{
	namespace StartUpBoard
	{
		void setProgress(int which, float v);
		void setText(char which, const char *s);
		void run();
		void complete();
	}
}

#endif