//---------------------------------------------------------------------------
#ifndef NVIDIADisplayClassH
#define NVIDIADisplayClassH
//---------------------------------------------------------------------------
#include "EDIDListClass.h"
//---------------------------------------------------------------------------
class NVIDIADisplayClass
{
private:
	int DisplayI2C(int, int *, int, unsigned char, unsigned char *, int);

public:
	bool LoadEDIDList(EDIDListClass &);
};
//---------------------------------------------------------------------------
#endif
