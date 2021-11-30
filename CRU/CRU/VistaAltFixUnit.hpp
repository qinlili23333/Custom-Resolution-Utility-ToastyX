// CodeGear C++Builder
// Copyright (c) 1995, 2021 by Embarcadero Technologies, Inc.
// All rights reserved

// (DO NOT EDIT: machine generated header) 'VistaAltFixUnit.pas' rev: 34.00 (Windows)

#ifndef VistaaltfixunitHPP
#define VistaaltfixunitHPP

#pragma delphiheader begin
#pragma option push
#pragma option -w-      // All warnings off
#pragma option -Vx      // Zero-length empty class member 
#pragma pack(push,8)
#include <System.hpp>
#include <SysInit.hpp>
#include <Winapi.Windows.hpp>
#include <System.Classes.hpp>

//-- user supplied -----------------------------------------------------------

namespace Vistaaltfixunit
{
//-- forward type declarations -----------------------------------------------
class DELPHICLASS TVistaAltFix;
//-- type declarations -------------------------------------------------------
class PASCALIMPLEMENTATION TVistaAltFix : public System::Classes::TComponent
{
	typedef System::Classes::TComponent inherited;
	
private:
	bool FInstalled;
	bool __fastcall VistaWithTheme();
	
public:
	__fastcall virtual TVistaAltFix(System::Classes::TComponent* AOwner);
	__fastcall virtual ~TVistaAltFix();
};


//-- var, const, procedure ---------------------------------------------------
extern DELPHI_PACKAGE void __fastcall Register();
}	/* namespace Vistaaltfixunit */
#if !defined(DELPHIHEADER_NO_IMPLICIT_NAMESPACE_USE) && !defined(NO_USING_NAMESPACE_VISTAALTFIXUNIT)
using namespace Vistaaltfixunit;
#endif
#pragma pack(pop)
#pragma option pop

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// VistaaltfixunitHPP
