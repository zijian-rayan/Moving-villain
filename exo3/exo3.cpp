//-----------------------------------------------------------------------------
// File: Exo3.cpp
//
// Desc: DirectX window application created by the DirectX AppWizard
//-----------------------------------------------------------------------------
#define STRICT
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <basetsd.h>
#include <math.h>
#include <stdio.h>
#include <d3dx9.h>
#include <dxerr9.h>
#include <tchar.h>
#include "DXUtil.h"
#include "D3DEnumeration.h"
#include "D3DSettings.h"
#include "D3DApp.h"
#include "D3DFont.h"
#include "D3DFile.h"
#include "D3DUtil.h"
#include "resource.h"
#include "Exo3.h"
#include <XInput.h>

INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, INT )
{
    CMyD3DApplication d3dApp;
    InitCommonControls();
    if( FAILED( d3dApp.Create( hInst ) ) )return 0;
    return d3dApp.Run();
}


//-----------------------------------------------------------------------------rouge
// Name: CMyD3DApplication()
// Desc: Application constructor.   Paired with ~CMyD3DApplication()
//       Member variables should be initialized to a known state here.  
//       The application window has not yet been created and no Direct3D device 
//       has been created, so any initialization that depends on a window or 
//       Direct3D should be deferred to a later stage. 
//-----------------------------------------------------------------------------
CMyD3DApplication::CMyD3DApplication(): cars(0), mt(0),room(1), rc(10), h_ang(0), v_ang(0), cull_mode(D3DCULL_NONE), fill_mode(D3DFILL_SOLID), shad_mode(D3DSHADE_GOURAUD),
                                        z_enb(1),spec_enb(0),light_enb(1),pnt_size(4.0f), spec_pow(30),m_terre(0),m_lune(0),Color(0x000000),Mode(D3DFOG_EXP),fogenable(0)
{
    m_dwCreationWidth           = 500;
    m_dwCreationHeight          = 375;
    m_strWindowTitle            = TEXT( "Exo3" );
    m_d3dEnumeration.AppUsesDepthBuffer   = TRUE;
	m_bStartFullscreen			= false;
	m_bShowCursorWhenFullscreen	= false;

    // Create a D3D font using d3dfont.cpp
    m_pFont                     = new CD3DFont( _T("Arial"), 12, D3DFONT_BOLD );
    m_bLoadingApp               = TRUE;
    m_pD3DXMesh                 = NULL;
}

//-----------------------------------------------------------------------------
// Name: ~CMyD3DApplication()
// Desc: Application destructor.  Paired with CMyD3DApplication()
//-----------------------------------------------------------------------------
CMyD3DApplication::~CMyD3DApplication()
{
  //la classe mère fait le nécessaire
}
//-----------------------------------------------------------------------------
// Name: OneTimeSceneInit()
// Desc: Paired with FinalCleanup().
//       The window has been created and the IDirect3D9 interface has been
//       created, but the device has not been created yet.  Here you can
//       perform application-related initialization and cleanup that does
//       not depend on a device.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::OneTimeSceneInit()
{
    // TODO: perform one time initialization

    // Drawing loading status message until app finishes loading
    SendMessage( m_hWnd, WM_PAINT, 0, 0 );
    m_bLoadingApp = FALSE;
    m_terre = new CSphere();
    m_lune = new CSphere();
	roomFile=new CD3DFile();
	car1=new CD3DFile();
	car2=new CD3DFile();
    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: FinalCleanup()
// Desc: Paired with OneTimeSceneInit()
//       Called before the app exits, this function gives the app the chance
//       to cleanup after itself.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::FinalCleanup()
{
    // TODO: Perform any final cleanup needed
    
    SAFE_DELETE( m_pFont );// Cleanup D3D font
    SAFE_DELETE( m_terre );
    SAFE_DELETE( m_lune );
    return S_OK;
}



//----------------------------------------------------------------------------- vert
// Name: InitDeviceObjects()
// Desc: Paired with DeleteDeviceObjects()
//       The device has been created.  Resources that are not lost on
//       Reset() can be created here -- resources in D3DPOOL_MANAGED,
//       D3DPOOL_SCRATCH, or D3DPOOL_SYSTEMMEM.  Image surfaces created via
//       CreateImageSurface are never lost and can be created here.  Vertex
//       shaders and pixel shaders can also be created here as they are not
//       lost on Reset().
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InitDeviceObjects()
{
    // TODO: create device objects
    //HRESULT hr;
    // Init the font
  m_terre->InitDeviceObjects(m_pd3dDevice,0);
  m_lune->InitDeviceObjects(m_pd3dDevice,0);

    m_pFont->InitDeviceObjects( m_pd3dDevice );
    D3DXCreateTeapot( m_pd3dDevice, &m_pD3DXMesh, NULL );
	
  tinyModel=new CModel(m_pd3dDevice, _T("tiny.x"));
  isStart=false;
  isForward=true;
  posTiny=0;
  periodAnim=0;
  tinyModel->SetTime(0);

		roomFile->Create(m_pd3dDevice,_T("droidroom.x"));
		car1->Create(m_pd3dDevice,_T("audi-a2.x"));
		car2->Create(m_pd3dDevice,_T("audi-a4.x"));
	
	return S_OK;
}
//-----------------------------------------------------------------------------
// Name: DeleteDeviceObjects()
// Desc: Paired with InitDeviceObjects()
//       Called when the app is exiting, or the device is being changed,
//       this function deletes any device dependent objects.  
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::DeleteDeviceObjects()
{
    // TODO: Cleanup any objects created in InitDeviceObjects()
    m_pFont->DeleteDeviceObjects();
    SAFE_RELEASE( m_pD3DXMesh );
	roomFile->Destroy();
	car1->Destroy();
	car2->Destroy();
	delete tinyModel;
    return S_OK;
}

//-----------------------------------------------------------------------------jaune

void CMyD3DApplication::UpdateRenderState()
{
	float Start   = 10.0f;    // For linear mode
    float End     = 30.2f;
    float Density = 0.16f;   // For exponential modes
 
  //parametres de la machine a rendre
  m_pd3dDevice->SetRenderState(D3DRS_CULLMODE,cull_mode);
  m_pd3dDevice->SetRenderState(D3DRS_FILLMODE,fill_mode);
  m_pd3dDevice->SetRenderState(D3DRS_SHADEMODE,shad_mode);
  m_pd3dDevice->SetRenderState(D3DRS_ZENABLE,z_enb);
  m_pd3dDevice->SetRenderState(D3DRS_SPECULARENABLE,spec_enb);
  m_pd3dDevice->LightEnable( 0, TRUE );
  m_pd3dDevice->SetRenderState(D3DRS_POINTSIZE, *((DWORD*)&pnt_size));
  m_pd3dDevice->SetRenderState(D3DRS_FOGENABLE,fogenable);
  m_pd3dDevice->SetRenderState(D3DRS_FOGCOLOR,Color);
  if( Mode == D3DFOG_LINEAR )
    {
        m_pd3dDevice->SetRenderState(D3DRS_FOGVERTEXMODE, Mode);
        m_pd3dDevice->SetRenderState(D3DRS_FOGSTART, *(DWORD *)(&Start));
        m_pd3dDevice->SetRenderState(D3DRS_FOGEND,   *(DWORD *)(&End));
    }
    else
    {
        m_pd3dDevice->SetRenderState(D3DRS_FOGVERTEXMODE, Mode);
        m_pd3dDevice->SetRenderState(D3DRS_FOGDENSITY, *(DWORD *)(&Density));
    }
  // Setup a material
  D3DMATERIAL9 mtrl;
  D3DUtil_InitMaterial( mtrl, 1.0f, 0.0f, 0.0f );
  mtrl.Power=spec_pow;
  mtrl.Specular.a=1.0f;mtrl.Specular.b=1.0f;mtrl.Specular.g=1.0f;mtrl.Specular.r=1.0f;
  m_pd3dDevice->SetMaterial( &mtrl );
  
}

//-----------------------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc: Paired with InvalidateDeviceObjects()
//       The device exists, but may have just been Reset().  Resources in
//       D3DPOOL_DEFAULT and any other device state that persists during
//       rendering should be set here.  Render states, matrices, textures,
//       etc., that don't change during rendering can be set once here to
//       avoid redundant state setting during Render() or FrameMove().
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::RestoreDeviceObjects()
{
    // TODO: setup render states
    UpdateRenderState();
    m_terre->RestoreDeviceObjects("earth.bmp");
    m_lune->RestoreDeviceObjects("MoonMap.bmp");
	roomFile->RestoreDeviceObjects(m_pd3dDevice);
	car1->RestoreDeviceObjects(m_pd3dDevice);
	car2->RestoreDeviceObjects(m_pd3dDevice);
    // Set up the textures
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );

    // Set miscellaneous render states
    m_pd3dDevice->SetRenderState( D3DRS_DITHERENABLE,   FALSE );
    //m_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE, FALSE );
    //m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,        TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_AMBIENT,        0x000F0F0F ); 

    // Set the projection matrix ==> caractéristiques optiques de la caméra
    D3DXMATRIX matProj;
    FLOAT fAspect = ((FLOAT)m_d3dsdBackBuffer.Width) / m_d3dsdBackBuffer.Height;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, fAspect, 1.0f, 100.0f );
    m_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

    // Set up lighting states
    D3DLIGHT9 light,light2;
    D3DUtil_InitLight( light, D3DLIGHT_DIRECTIONAL, -1.0f, -1.0f, 2.0f );
    D3DUtil_InitLight( light2, D3DLIGHT_DIRECTIONAL, 1.0f, 1.0f, -2.0f );
    light.Specular.r=0.2f;light.Specular.g=0.2f;light.Specular.b=0.2f;
    m_pd3dDevice->SetLight( 0, &light );
	m_pd3dDevice->SetLight( 1, &light2 );
    m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );

    // Restore the font
    m_pFont->RestoreDeviceObjects();

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc: Invalidates device objects.  Paired with RestoreDeviceObjects()
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InvalidateDeviceObjects()
{
    // TODO: Cleanup any objects created in RestoreDeviceObjects()
    m_pFont->InvalidateDeviceObjects();
    m_terre->InvalidateDeviceObjects();
    m_lune->InvalidateDeviceObjects();
	
	roomFile->InvalidateDeviceObjects();
	car1->InvalidateDeviceObjects();
	car2->InvalidateDeviceObjects();
	
    return S_OK;
}




//-----------------------------------------------------------------------------bleu
// Name: FrameMove()
// Desc: Called once per frame, the call is the entry point for animating
//       the scene.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::FrameMove()
{
  const FLOAT ANG_SCALE=2.0f;
  const FLOAT Z_SCALE=2.0f;
XINPUT_STATE st;
static float vt=0,vl=0;

vt+=0.7*m_fElapsedTime;
vl+=0.3*m_fElapsedTime;

  //Sleep(10);
  //h_ang-=ANG_SCALE*m_fElapsedTime;



if(XInputGetState(0,&st)==ERROR_SUCCESS)
{
  if(st.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) v_ang+=ANG_SCALE*m_fElapsedTime;
  if(st.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) v_ang-=ANG_SCALE*m_fElapsedTime;
  if(st.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) h_ang-=ANG_SCALE*m_fElapsedTime;
  if(st.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) h_ang+=ANG_SCALE*m_fElapsedTime;
  if(st.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) rc*=(1+Z_SCALE*m_fElapsedTime);//on multiplie car la vitesse dépend de la distance au centre
  if(st.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) rc*=(1-Z_SCALE*m_fElapsedTime);
  XINPUT_VIBRATION xvib = {((rc<2.0f)?10000:0),((rc>20.0f)?50000:0)};
  XInputSetState(0,&xvib);
}
else{
  if(GetAsyncKeyState( VK_UP )    & 0x8000) v_ang+=ANG_SCALE*m_fElapsedTime;
  if(GetAsyncKeyState( VK_DOWN )    & 0x8000) v_ang-=ANG_SCALE*m_fElapsedTime;
  if(GetAsyncKeyState( VK_LEFT )    & 0x8000) h_ang-=ANG_SCALE*m_fElapsedTime;
  if(GetAsyncKeyState( VK_RIGHT )    & 0x8000) h_ang+=ANG_SCALE*m_fElapsedTime;
  
  if(GetAsyncKeyState( VK_NEXT )    & 0x8000) rc*=(1+Z_SCALE*m_fElapsedTime);//on multiplie car la vitesse dépend de la distance au centre
  if(GetAsyncKeyState( VK_PRIOR )    & 0x8000) rc*=(1-Z_SCALE*m_fElapsedTime);
 } 
if(v_ang>=D3DX_PI/2-0.01f) v_ang=D3DX_PI/2-0.01f;
  if(v_ang<=-D3DX_PI/2+0.01f) v_ang=-D3DX_PI/2+0.01f;

   // Set up our view matrix. A view matrix can be defined given an eye point,
   // a point to lookat, and a direction for which way is up. Here, we set the
   // eye five units back along the z-axis and up three units, look at the
   // origin, and define "up" to be in the y-direction.
   D3DXMATRIX matView;
   D3DXVECTOR3 vFromPt   = D3DXVECTOR3( rc*cos(h_ang)*cos(v_ang), rc*sin(v_ang), rc*sin(h_ang)*cos(v_ang));
   D3DXVECTOR3 vLookatPt = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
   D3DXVECTOR3 vUpVec    = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
   D3DXMatrixLookAtLH( &matView, &vFromPt, &vLookatPt, &vUpVec );
   m_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );
   // Update the world state according to user input
   
  
   //l'objet ne bouge plus : on va donc utiliser la matrice identité comme matWorld. On l'appelle ensuite mat1
   
   //D3DXMatrixRotationX(&mat2,50);
   D3DXMatrixIdentity(&mat1);
   m_pd3dDevice->SetTransform( D3DTS_WORLD, &mat1 );
   D3DXMatrixRotationY(&mat1,vt);
   
   D3DXMatrixTranslation(&mat2,50,0,0);
   
   D3DXMatrixRotationY(&mat3,vl);
  
   //scaling pour la taille
   D3DXMatrixScaling(&mat5,0.1,0.1,0.1); 
   mat4=mat2*mat3*mat5;

   
	//Matrix room
    //D3DXMatrixScaling(&mat_room,0.004f,0.004f,0.004f); 
	m_pd3dDevice->SetTransform( D3DTS_WORLD,  &mat_room ); 

	//Matrix car1
	D3DXMatrixIdentity(&mat_car1);
	m_pd3dDevice->SetTransform( D3DTS_WORLD,  &mat_car1 ); 
	//Translation car1
	D3DXMatrixTranslation(&mat_car1,-4.0f,0,0);
	//Revolution car1
	D3DXMATRIX matRevCar;
	D3DXMatrixRotationY(&matRevCar, -m_fTime);
	D3DXMatrixMultiply(&mat_car1,& mat_car1,& matRevCar); 

	//Matrix car2
	D3DXMATRIX matScaleCar2;
	D3DXMatrixScaling(&matScaleCar2,1.0f,0.9f,-1.3f);  
	//Translation car2
	D3DXMATRIX matTransCar2;
	D3DXMatrixTranslation(&matTransCar2,4.0f,0,0);
	D3DXMatrixMultiply(&mat_car2,&matScaleCar2,&matTransCar2); 
	//Revolution car2
	D3DXMatrixRotationY(&matRevCar, -m_fTime);
	D3DXMatrixMultiply(&mat_car2,& mat_car2,& matRevCar); 
	m_pd3dDevice->SetTransform( D3DTS_WORLD,  &mat_car2 ); 
   return S_OK;
}
//-----------------------------------------------------------------------------
// Name: UpdateInput()
// Desc: Update the user input.  Called once per frame 
//-----------------------------------------------------------------------------
//void CMyD3DApplication::UpdateInput( UserInput* pUserInput )
//{
//    pUserInput->bRotateUp    = ( m_bActive && (GetAsyncKeyState( VK_UP )    & 0x8000) == 0x8000 );
//    pUserInput->bRotateDown  = ( m_bActive && (GetAsyncKeyState( VK_DOWN )  & 0x8000) == 0x8000 );
//    pUserInput->bRotateLeft  = ( m_bActive && (GetAsyncKeyState( VK_LEFT )  & 0x8000) == 0x8000 );
//    pUserInput->bRotateRight = ( m_bActive && (GetAsyncKeyState( VK_RIGHT ) & 0x8000) == 0x8000 );
//}
//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Called once per frame, the call is the entry point for 3d
//       rendering. This function sets up render states, clears the
//       viewport, and renders the scene.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::Render()
{
    // Clear the viewport
    m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
                         0x000000ff, 1.0f, 0L );

    // Begin the scene
    if( SUCCEEDED( m_pd3dDevice->BeginScene() ) )
    {
        // TODO: render world
        
        // Render the teapot mesh
        //m_pd3dDevice->SetTransform( D3DTS_WORLD, &mat1 );
        //m_pD3DXMesh->DrawSubset(0);
        //m_pd3dDevice->SetTransform( D3DTS_WORLD, &mat2 );
        //m_pD3DXMesh->DrawSubset(0);
		if(mt)
		{
			m_terre->Render(&mat1);
			m_lune->Render(&mat4);
		}
        // Render stats and help text  
        RenderText();
		if(room)
		{
			m_pd3dDevice->LightEnable( 1, TRUE );
			
			m_pd3dDevice->LightEnable( 2, TRUE );
			D3DXMATRIX matScaleRoom,matTransRoom;
			D3DXMatrixScaling(&matScaleRoom,0.004f,0.004f,0.0081f);
			D3DXMatrixTranslation(&matTransRoom,0.0f,14.5,0);
			D3DXMatrixMultiply(&mat_room,&matScaleRoom,&matTransRoom); 
			m_pd3dDevice->SetTransform( D3DTS_WORLD,  &mat_room);

			roomFile->Render(m_pd3dDevice,&mat_room);
		}
		m_pd3dDevice->LightEnable( 1, FALSE );
		if(cars){
			car1->CD3DFrame::Render(m_pd3dDevice,true,false,&mat_car1);  
			car2->CD3DFrame::Render(m_pd3dDevice,true,false,&mat_car2);
			//Transparence car + vitre par alpha
			m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
			m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
			m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);
			car1->CD3DFrame::Render(m_pd3dDevice,false,true,&mat_car1);
			car2->CD3DFrame::Render(m_pd3dDevice,false,true,&mat_car2);
			m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);
			// End the scene.
		}


		if(isStart==true)
		{
		  tinyModel->SetCurrentAnimation(0);
		  tinyModel->SetTime(m_fTime/8);
		  if(isForward)
			posTiny-=0.0005f;
		  else
			posTiny+=0.0005f;
		}
		else
		{
		  tinyModel->SetCurrentAnimation();
		}
		 D3DXMATRIX matScaleTiny,matTransTiny;
		D3DXMatrixScaling(&matScaleTiny,0.005f,0.005f,0.005f);
		D3DXMatrixTranslation(&matTransTiny,0,-0.2f,posTiny);
		D3DXMatrixMultiply(&mat_tiny,&matScaleTiny,&matTransTiny); 
		m_pd3dDevice->SetTransform( D3DTS_WORLD,  &mat_tiny);
		tinyModel->Draw();

        m_pd3dDevice->EndScene();
    }
	

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RenderText()
// Desc: Renders stats and help text to the scene.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::RenderText()
{
    D3DCOLOR fontColor        = D3DCOLOR_ARGB(255,255,255,0);
    TCHAR szMsg[MAX_PATH] = TEXT("");

    // Output display stats
    FLOAT fNextLine = 40.0f; 

    lstrcpy( szMsg, m_strDeviceStats );
    fNextLine -= 20.0f;
    m_pFont->DrawText( 2, fNextLine, fontColor, szMsg );

    lstrcpy( szMsg, m_strFrameStats );
    fNextLine -= 20.0f;
    m_pFont->DrawText( 2, fNextLine, fontColor, szMsg );

    // Output statistics & help
    fNextLine = (FLOAT) m_d3dsdBackBuffer.Height; 

  //  wsprintf( szMsg, TEXT("Arrow keys: Up=%d Down=%d Left=%d Right=%d"), 
    //          m_UserInput.bRotateUp, m_UserInput.bRotateDown, m_UserInput.bRotateLeft, m_UserInput.bRotateRight );
    fNextLine -= 20.0f; m_pFont->DrawText( 2, fNextLine, fontColor, szMsg );

    lstrcpy( szMsg, TEXT("Use arrow keys to rotate object") );
    fNextLine -= 20.0f; m_pFont->DrawText( 2, fNextLine, fontColor, szMsg );

    lstrcpy( szMsg, TEXT("Press 'F2' to configure display") );
    fNextLine -= 20.0f; m_pFont->DrawText( 2, fNextLine, fontColor, szMsg );

    return S_OK;
}


	
//-----------------------------------------------------------------------------gui
// Name: MsgProc()
// Desc: Overrrides the main WndProc, so the sample can do custom message
//       handling (e.g. processing mouse, keyboard, or menu commands).
//-----------------------------------------------------------------------------
LRESULT CMyD3DApplication::MsgProc( HWND hWnd, UINT msg, WPARAM wParam,
                                    LPARAM lParam )
{
    switch( msg )
    {
        case WM_PAINT:
        {
            if( m_bLoadingApp )
            {
                // Draw on the window tell the user that the app is loading
                // TODO: change as needed
                HDC hDC = GetDC( hWnd );
                TCHAR strMsg[MAX_PATH];
                wsprintf( strMsg, TEXT("Loading... Please wait") );
                RECT rct;
                GetClientRect( hWnd, &rct );
                DrawText( hDC, strMsg, -1, &rct, DT_CENTER|DT_VCENTER|DT_SINGLELINE );
                ReleaseDC( hWnd, hDC );
            }
            break;
        }
        case WM_COMMAND:
          {
            bool do_update = true;
            switch(wParam){
              case 0/*ID crée dans la ressource*/  : break;
			  case ID_CULLMODE_NONE                : cull_mode= D3DCULL_NONE; break;
			  case ID_CULLMODE_CLOCKWISE           : cull_mode= D3DCULL_CW; break;
			  case ID_CULLMODE_COUNTERCLOCKWISE    : cull_mode= D3DCULL_CCW; break;
			  case ID_FILLMODE_POINT               : fill_mode= D3DFILL_POINT; break;
			  case ID_FILLMODE_WIREFRAME           : fill_mode= D3DFILL_WIREFRAME; break;
			  case ID_FILLMODE_SOLID               : fill_mode= D3DFILL_SOLID; break;
			  case ID_SHADEMODE_FLAT               : shad_mode = D3DSHADE_FLAT; break;
			  case ID_SHADEMODE_GAURAUD            : shad_mode = D3DSHADE_GOURAUD; break;
			  case ID_SHADEMODE_PHONG			   : shad_mode = D3DSHADE_PHONG; break;
			  case ID_RENDERSTATES_ZENABLE         : z_enb=!z_enb; break;
			  case ID_LIGHTS_SPECULARENABLE        : spec_enb = !spec_enb;break;
			  case ID_LIGHTS_LIGTHENABLE           : light_enb = !light_enb;break;
			  case ID_POINTSIZE_1PIXEL             : pnt_size= 1.f;break;
			  case ID_POINTSIZE_2PIXELS            : pnt_size= 2.f;break;
			  case ID_POINTSIZE_4PIXELS            : pnt_size= 4.f;break;
			  case ID_SPECULARPOWER_3              : spec_pow = 3;break;
			  case ID_SPECULARPOWER_5              : spec_pow = 10;break;
			  case ID_SPECULARPOWER_10             : spec_pow = 30;break;
			  case ID_FOG_FOGENABLE				   :fogenable=!fogenable;break;
			  case ID_MODE_LINEAR				   :Mode=D3DFOG_LINEAR;break;
			  case ID_MODE_NONE				       :Mode=D3DFOG_NONE;break;
			  case ID_MODE_EXP					   :Mode=D3DFOG_EXP;break;
			  case ID_MODE_EXP2					   :Mode=D3DFOG_EXP2;break;
			  case ID_DISPLAY_CARS				   :cars=!cars;break;
			  case ID_DISPLAY_MOON				   :mt=!mt;break;
			  case ID_DISPLAY_ROOM				   :room=!room;break;

				  
              default: do_update = false;
            }
            if(do_update){UpdateRenderState(); return 0; }
          } break;
		  case WM_KEYDOWN:
			{
			  switch (wParam)
			  {
			  case VK_SPACE: isStart=!isStart; break;
			  case VK_BACK:isForward=!isForward; break;
			  case 'R':
				tinyModel->SetTime(0);
				tinyModel->SetCurrentAnimation();
				isStart=false;
				posTiny=0; 
				break;
			  }
			}break;
    }

    return CD3DApplication::MsgProc( hWnd, msg, wParam, lParam );
}













