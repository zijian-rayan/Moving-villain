#pragma once

class CSphereMap
{
  struct SPHEREVERTEX
  {
    D3DXVECTOR3 p;
    D3DXVECTOR3 n;
    FLOAT       tu1, tv1;
  };
  static const DWORD D3DFVF_SPHEREVERTEX = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;
  IDirect3DDevice9* pD3DDev; 
  IDirect3DVertexBuffer9 *pVB;
  IDirect3DCubeTexture9 *pTex;
  IDirect3DStateBlock9* pBlk;
  DWORD rings, segs;
  D3DMATERIAL9 mtrl;
  D3DXMATRIX matWorld;
public:
  CSphereMap(DWORD _rings=30, DWORD _segs=30): pD3DDev(0), pVB(0), pTex(0), pBlk(0), rings(_rings), segs(_segs)
  {
    D3DXMatrixIdentity( &matWorld );
    ZeroMemory( &mtrl, sizeof(D3DMATERIAL9) );
    mtrl.Diffuse.r = mtrl.Ambient.r = 1.0f;
    mtrl.Diffuse.g = mtrl.Ambient.g = 1.0f;
    mtrl.Diffuse.b = mtrl.Ambient.b = 1.0f;
    mtrl.Diffuse.a = mtrl.Ambient.a = 1.0f;
    mtrl.Power=20;
  }
  ~CSphereMap()
  {
    DeleteDeviceObjects();
  }
  void InitDeviceObjects(IDirect3DDevice9* pDev, D3DCOLORVALUE *pspec=0)
  {
    pD3DDev=pDev;
    if(pspec) mtrl.Specular=*pspec;
    else { mtrl.Specular.r=mtrl.Specular.g=1.0f; }
  }
  void DeleteDeviceObjects()
  {
    InvalidateDeviceObjects();
    pD3DDev=0;
  }
  void RestoreDeviceObjects(LPCTSTR fname)
  {
    //fname=_T("cube_city1.dds");
    pD3DDev->CreateStateBlock(D3DSBT_PIXELSTATE,&pBlk);
    if(pD3DDev->CreateVertexBuffer(rings*(segs+1)*2*sizeof(SPHEREVERTEX),0,D3DFVF_SPHEREVERTEX,
      D3DPOOL_DEFAULT, &pVB, 0) < 0) return;
    SPHEREVERTEX* vtx;
    if(pVB->Lock( 0, 0, (VOID**)&vtx,0)<0) return;
    FLOAT ra = D3DX_PI/rings, sa  = 2.0f*D3DX_PI/segs;
    // Generate the group of rings for the sphere
    for( DWORD ring = 0; ring < rings; ring++ )
    {
      FLOAT r0 = sinf(ring*ra), r1 = sinf((ring+1)*ra);
      FLOAT y0 = cosf(ring*ra), y1 = cosf((ring+1)*ra);
      // Generate the group of segments for the current ring
      for( DWORD seg = 0; seg < (segs+1); seg++ )
      {
        FLOAT segsin=sinf(seg*sa), segcos=cosf(seg*sa); 
        vtx->p   = vtx->n   = D3DXVECTOR3(r0*segsin,y0,r0*segcos);
        vtx->tu1 = -((FLOAT)seg)/segs;
        vtx->tv1 = ring/(FLOAT)rings;
        vtx++;
        vtx->p   = vtx->n   = D3DXVECTOR3(r1*segsin,y1,r1*segcos);
        vtx->tu1 = (vtx-1)->tu1;
        vtx->tv1 = (ring+1)/(FLOAT)rings;
        vtx++;
      }
    }
    pVB->Unlock();
    if(D3DXCreateCubeTextureFromFile(pD3DDev,fname,&pTex)<0) return;
  }
  void InvalidateDeviceObjects()
  {
    SAFE_RELEASE(pVB);
    SAFE_RELEASE(pTex);
    SAFE_RELEASE(pBlk);
  }
  void Render(D3DXMATRIX *pm)
  {
    pBlk->Capture();
    //pD3DDev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    //pD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    //pD3DDev->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    //pD3DDev->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
    pD3DDev->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
    pD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    //pD3DDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    pD3DDev->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
    pD3DDev->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3);
    pD3DDev->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0 | D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);
//    pD3DDev->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0 | D3DTSS_TCI_CAMERASPACEPOSITION);
//    pD3DDev->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0 | D3DTSS_TCI_CAMERASPACENORMAL);
    pD3DDev->SetSamplerState( 0, D3DSAMP_ADDRESSU,  D3DTADDRESS_CLAMP/*D3DTADDRESS_MIRROR*/ );
    pD3DDev->SetSamplerState( 0, D3DSAMP_ADDRESSV,  D3DTADDRESS_CLAMP/*D3DTADDRESS_MIRROR*/ );
    pD3DDev->SetSamplerState( 0, D3DSAMP_ADDRESSW,  D3DTADDRESS_CLAMP/*D3DTADDRESS_MIRROR*/ );
    pD3DDev->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
    pD3DDev->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
    pD3DDev->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_NONE );
    
    pD3DDev->SetTexture(0,pTex);
    pD3DDev->SetMaterial( &mtrl );
    //pD3DDev->SetRenderState(D3DRS_AMBIENT, 0x00404040);
    pD3DDev->SetTransform( D3DTS_WORLD,pm ? pm : &matWorld );
    pD3DDev->SetStreamSource(0, pVB, 0, sizeof(SPHEREVERTEX) );
    pD3DDev->SetFVF(D3DFVF_SPHEREVERTEX);
    pD3DDev->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, rings*(segs+1)*2-2);
    pBlk->Apply();
  }
};