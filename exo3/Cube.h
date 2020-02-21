#pragma once

class CCube
{
  struct CUBEVERTEX
  {
    D3DXVECTOR3 p;
    D3DXVECTOR3 t;
  };
  static const DWORD D3DFVF_CUBEVERTEX= D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE3(0);
  IDirect3DDevice9* pD3DDev; 
  IDirect3DVertexBuffer9 *pVB;
  IDirect3DCubeTexture9 *pTex;
  IDirect3DStateBlock9* pBlk;
  D3DMATERIAL9 mtrl;
  D3DXMATRIX matWorld;
  float ray;
public:
  CCube(float _ray=100): pD3DDev(0), pVB(0), pTex(0), pBlk(0), ray(_ray)
  {
    D3DXMatrixIdentity(&matWorld);
    ZeroMemory( &mtrl, sizeof(D3DMATERIAL9) );
    mtrl.Diffuse.r = mtrl.Ambient.r = 1.0f;
    mtrl.Diffuse.g = mtrl.Ambient.g = 1.0f;
    mtrl.Diffuse.b = mtrl.Ambient.b = 1.0f;
    mtrl.Diffuse.a = mtrl.Ambient.a = 1.0f;
  }
  ~CCube()
  {
    DeleteDeviceObjects();
  }
  void InitDeviceObjects(IDirect3DDevice9* pDev)
  {
    pD3DDev=pDev;
  }
  void DeleteDeviceObjects()
  {
    InvalidateDeviceObjects();
    pD3DDev=0;
  }
  bool RestoreDeviceObjects(LPCTSTR ftex)
  {
    pD3DDev->CreateStateBlock(D3DSBT_PIXELSTATE,&pBlk);
    if(pD3DDev->CreateVertexBuffer(16*sizeof(CUBEVERTEX),0,D3DFVF_CUBEVERTEX, D3DPOOL_DEFAULT, &pVB, 0) < 0)
      return false;
    CUBEVERTEX* vtx;
    if(pVB->Lock( 0, 0, (VOID**)&vtx,0)<0) return false;
    vtx->p=vtx->t=D3DXVECTOR3(-1.000000,-1.000000,1.000000);vtx++;     // 0
    vtx->p=vtx->t=D3DXVECTOR3(-1.000000,1.000000,1.000000);vtx++;      // 4
    vtx->p=vtx->t=D3DXVECTOR3(-1.000000,-1.000000,-1.000000);vtx++;    // 1
    vtx->p=vtx->t=D3DXVECTOR3(-1.000000,1.000000,-1.000000);vtx++;     // 5
    vtx->p=vtx->t=D3DXVECTOR3(1.000000,-1.000000,-1.000000);vtx++;     // 2
    vtx->p=vtx->t=D3DXVECTOR3(1.000000,1.000000,-1.000000);vtx++;      // 6
    vtx->p=vtx->t=D3DXVECTOR3(1.000000,-1.000000,1.000000);vtx++;      // 3
    vtx->p=vtx->t=D3DXVECTOR3(1.000000,1.000000,1.000000);vtx++;       // 7

    vtx->p=vtx->t=D3DXVECTOR3(1.000000,1.000000,-1.000000);vtx++;      // 6
    vtx->p=vtx->t=D3DXVECTOR3(-1.000000,1.000000,-1.000000);vtx++;     // 10=5
    vtx->p=vtx->t=D3DXVECTOR3(1.000000,1.000000,1.000000);vtx++;       // 7
    vtx->p=vtx->t=D3DXVECTOR3(-1.000000,1.000000,1.000000);vtx++;      // 11=4
    vtx->p=vtx->t=D3DXVECTOR3(1.000000,-1.000000,1.000000);vtx++;      // 8=3
    vtx->p=vtx->t=D3DXVECTOR3(-1.000000,-1.000000,1.000000);vtx++;     // 12=0
    vtx->p=vtx->t=D3DXVECTOR3(1.000000,-1.000000,-1.000000);vtx++;     // 9=2
    vtx->p=vtx->t=D3DXVECTOR3(-1.000000,-1.000000,-1.000000);vtx++;    // 13=1
    for(int i=1; i<=16; i++) { vtx[-i].p*=ray; }
    pVB->Unlock();
    return D3DXCreateCubeTextureFromFile(pD3DDev,ftex,&pTex)>=0;
  }
  void InvalidateDeviceObjects()
  {
    SAFE_RELEASE(pVB);
    SAFE_RELEASE(pTex);
    SAFE_RELEASE(pBlk);
  }
  void Render()
  {
    pBlk->Capture();
    pD3DDev->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
    pD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    //pD3DDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    pD3DDev->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
    pD3DDev->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3);
    pD3DDev->SetTexture(0,pTex);
    pD3DDev->SetMaterial(&mtrl);
    pD3DDev->SetTransform(D3DTS_WORLD,&matWorld);
    pD3DDev->SetStreamSource(0, pVB, 0, sizeof(CUBEVERTEX) );
    pD3DDev->SetFVF(D3DFVF_CUBEVERTEX);
    pD3DDev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 6);
    pD3DDev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 8, 6);
    pBlk->Apply();
  }
};