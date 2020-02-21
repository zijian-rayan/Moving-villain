//-----------------------------------------------------------------------------
// File: Exo3.h
//
// Desc: Header file Exo3 sample app
//-----------------------------------------------------------------------------
#pragma once
#include "Sphere.h"
#include "XFileModel.h"
#include "Cube.h"
//-----------------------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: Application class. The base class (CD3DApplication) provides the 
//       generic functionality needed in all Direct3D samples. CMyD3DApplication 
//       adds functionality specific to this sample program.
//-----------------------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
    BOOL                    m_bLoadingApp;          // TRUE, if the app is loading
    CD3DFont*               m_pFont;                // Font for drawing text
    ID3DXMesh*              m_pD3DXMesh;            // D3DX mesh to store teapot
    FLOAT                   rc,h_ang,v_ang;         //distance par rapport au centre, angle horizontal, vertical
    CSphere                *m_terre;
    CSphere                *m_lune;
    DWORD                   cars,mt,room,cull_mode,fill_mode, shad_mode, z_enb,spec_enb,light_enb,fogenable;
    FLOAT                   pnt_size, spec_pow;
    D3DXMATRIX				mat1,mat2,mat3,mat4,mat5,mat_room,mat_car1,mat_car2;
	DWORD					Color;
	DWORD					Mode;
	CD3DFile*				roomFile, *car1, *car2;
	CModel*					tinyModel;
	D3DXMATRIX				mat_tiny;
	bool					isStart,isForward;
	float					posTiny;
	double					periodAnim;

protected:
    virtual HRESULT OneTimeSceneInit();
    virtual HRESULT InitDeviceObjects();
    virtual HRESULT RestoreDeviceObjects();
    virtual HRESULT InvalidateDeviceObjects();
    virtual HRESULT DeleteDeviceObjects();
    virtual HRESULT Render();
    virtual HRESULT FrameMove();
    virtual HRESULT FinalCleanup();

    HRESULT RenderText();

    void UpdateRenderState();

public:
    LRESULT MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
    CMyD3DApplication();
    virtual ~CMyD3DApplication();
};

