#define INITGUID
#include <ddraw.h>
#include <windows.h>
#include <stdio.h>
#include <math.h>
#include "dsurf.h"


bool IsOK(HRESULT res, const char* msg="???")
{
	if(res==DD_OK) return true;
	printf("Erreur: 0x%08lX (%d) dans %s",res,res,msg);
	return false;
}

void test1()
{
  const DWORD DIMX=200,DIMY=200,PAS=4;
  IDirectDraw7* pdd=0;
  if(!IsOK(DirectDrawCreateEx(0,(void**)&pdd,IID_IDirectDraw7,0),"DirectDrawCreateEx")) 
    return;
  if(IsOK(pdd->SetCooperativeLevel(0,DDSCL_NORMAL),"SetCoopLevel"))
  {
      DSurfaceDesc2 dsurf1;
      dsurf1.SetCaps(DDSCAPS_PRIMARYSURFACE);
        IDirectDrawSurface7 *pdds=0;
      if(IsOK(pdd->CreateSurface(&dsurf1,&pdds,0),"Create Primar Surf"))
      {
        DSurfaceDesc2 dsurf1b(pdds);
        dsurf1b.InfoMsg();
        pdds->Release();
      }
      //LOCK...? shutdown dwm d'abord!
  }
  pdd->Release();
}
#if 0
int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
  ::MessageBox(0,"Exo1","Message",MB_OK);
  test1();
  return 0;
}
#else
void main() {test1();}
#endif