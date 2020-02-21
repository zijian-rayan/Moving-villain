#include <ddraw.h>                                                                

class DSurfaceDesc2 : public DDSURFACEDESC2
{
public:
  void Clean()
  {
    ZeroMemory(this, sizeof(*this));
    dwSize=sizeof(*this);
  }
  void SetCaps(DWORD Caps, DWORD Caps2=0, DWORD Caps3=0, DWORD Caps4=0)
  {
    dwFlags |= DDSD_CAPS;
    ddsCaps.dwCaps=Caps; ddsCaps.dwCaps2=Caps2; ddsCaps.dwCaps3=Caps3; ddsCaps.dwCaps4=Caps4;
  }
  void SetBBCount(DWORD Count)
  {
    dwFlags |= DDSD_BACKBUFFERCOUNT;
    dwBackBufferCount=Count;
  }
  void SetWH(DWORD Width=0, DWORD Height=0)
  {
    dwFlags |= DDSD_WIDTH | DDSD_HEIGHT;
    dwWidth=Width; dwHeight=Height;
  }
  void SetPixFormat(DDPIXELFORMAT &Format)
  {
    dwFlags |= DDSD_PIXELFORMAT;
    ddpfPixelFormat=Format;
  }
  DSurfaceDesc2() { Clean(); }
  DSurfaceDesc2(IDirectDrawSurface7* pdds) { Clean(); pdds->GetSurfaceDesc(this); }
  void InfoMsg(HWND hw=0)
  {
    char buf[2048];
    wsprintf(buf,"Flags=0x%08lX\n",dwFlags);
    if(dwFlags & DDSD_CAPS)
      wsprintf(buf+strlen(buf),"Caps: %08lX %08lX %08lX %08lX \n",
        ddsCaps.dwCaps,ddsCaps.dwCaps2,ddsCaps.dwCaps3,ddsCaps.dwCaps4);
    if(dwFlags & DDSD_HEIGHT)
      wsprintf(buf+strlen(buf),"Height=%lu\n",dwHeight);
    if(dwFlags & DDSD_WIDTH)
      wsprintf(buf+strlen(buf),"Width=%lu\n",dwWidth);
    if(dwFlags & DDSD_PITCH)
      wsprintf(buf+strlen(buf),"Pitch=%ld\n",lPitch);
    if(dwFlags & DDSD_BACKBUFFERCOUNT)
      wsprintf(buf+strlen(buf),"BackBuffers=%ld\n",dwBackBufferCount);
    if(dwFlags & DDSD_LPSURFACE)
      wsprintf(buf+strlen(buf),"@Surface=%0x%08lX\n",lpSurface);
    if(dwFlags & DDSD_PIXELFORMAT)
    {
      if(ddpfPixelFormat.dwFlags & DDPF_RGB)
        wsprintf(buf+strlen(buf),"PixFormat:RGB%lu %08lu (%08lX:%08lX:%08lX:%08lX)\n",ddpfPixelFormat.dwRGBBitCount,
        ddpfPixelFormat.dwFlags,
        ddpfPixelFormat.dwRBitMask,ddpfPixelFormat.dwGBitMask,ddpfPixelFormat.dwBBitMask,
        ddpfPixelFormat.dwRGBAlphaBitMask);
      if(ddpfPixelFormat.dwFlags & DDPF_FOURCC)
        wsprintf(buf+strlen(buf),"PixFormat:%.4s\n",&ddpfPixelFormat.dwFourCC);
    }
    ::MessageBox(hw,buf,"Info Surface",MB_OK);
  }
  void Rotate180()
  {
    // seulement pour RGB32 et pitch=width*4 !!!
    DWORD sz=dwWidth*dwHeight;
    DWORD* p1=(DWORD*)lpSurface, *p2=p1+sz-1, tmp;
    for( ; p1<p2; p1++, p2--) { tmp=*p2; *p2=*p1; *p1=tmp;}
  }
  void FillYUV(DWORD clr)
  {
    DWORD* p1=(DWORD*)lpSurface;
    for(DWORD y=0; y<dwHeight; y++)
    {
      for(DWORD x=0; x<dwWidth; x+=2) *(p1++)=clr;
      p1= (DWORD*)(((BYTE*)p1)+lPitch-dwWidth*2);
    }
  }
};
