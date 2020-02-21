#ifndef _XFILE_MODEL_H
#define _XFILE_MODEL_H
// (c) 2003-2004 J. Jurecka, (c) 2009-2015 M. Vasiliu
#pragma comment(lib, "d3dx9")
#include <d3dx9.h>
#include <d3dx9anim.h>
#include "dxutil.h"
#include <stdio.h>

////Delete an Array safely
//#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p); (p)=NULL; } }
////Delete an object pointer
//#define SAFE_DELETE(p) { if(p) { delete (p); (p)=NULL; } }
////Release an object pointer
//#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

// Custom mesh container derived from the default one
typedef struct _D3DXMESHCONTAINER_DERIVED: public D3DXMESHCONTAINER
{
  //Mesh variables
  IDirect3DTexture9 **ppTextures;		      // Textures of the mesh
  D3DMATERIAL9       *pMaterials9;	      // Use the DirectX 9 Material type
  //Skinned mesh variables
  ID3DXMesh          *pSkinMesh;			    // The skin mesh
  D3DXMATRIX		     *pBoneOffsets;		    // The bone matrix Offsets
  D3DXMATRIX        **ppFrameMatrices;	  // Pointer to the Frame Matrix
  // Attribute table stuff
  D3DXATTRIBUTERANGE *pAttributeTable;	  // The attribute table
  DWORD               NumAttributeGroups; // The number of attribute groups
}MESHCONTAINER, *LPMESHCONTAINER;

// Custom frame struct derived from D3DXFRAME
typedef struct _D3DXFRAME_DERIVED: public D3DXFRAME
{
  D3DXMATRIX matCombined;	//Combined Transformation Matrix
}FRAME, *LPFRAME;

// This is an Allocation class that is used with the D3DXLoadMeshHierarchyFromX function.
// It handles the Creation and Deletion of Frames and Mesh Containers. The overloaded
// functions are callbacks so there is no need to call any of the functions in written code
// just pass an Object of this class to the function
class CAllocateHierarchy: public ID3DXAllocateHierarchy
{
public:
  // Create a frame
  //1. The name of the frame
  //2. The output new frame
  STDMETHOD(CreateFrame)(THIS_ LPCSTR Name, LPD3DXFRAME *ppNewFrame)
  {    
    //printf("CreateFrame: \"%s\"\n",Name);
    // Create a frame using custom struct
    LPFRAME pFrame = new FRAME;
    ZeroMemory(pFrame, sizeof(FRAME));
    *ppNewFrame = (LPD3DXFRAME)pFrame;; // Set the output frame to the one that we created
    // Put the name in the frame
    pFrame->Name= Name ? _strdup(Name) : 0;
    // Initialize the rest of the frame
    pFrame->pFrameFirstChild = NULL;
    pFrame->pFrameSibling = NULL;
    pFrame->pMeshContainer = NULL;
    D3DXMatrixIdentity(&pFrame->matCombined);
    D3DXMatrixIdentity(&pFrame->TransformationMatrix);
    return S_OK;
  }
  // Create a Mesh Container
  //1. Name of the Mesh
  //2. The mesh Data
  //3. that materials of the mesh
  //4. the effects on the mesh
  //5. the number of meterials in the mesh
  //6. the adjacency array for the mesh
  //7. the skin information for the mesh
  //8. the output mesh container
  STDMETHOD(CreateMeshContainer)(THIS_ LPCSTR Name, LPD3DXMESHDATA pMeshData, LPD3DXMATERIAL pMaterials, 
    LPD3DXEFFECTINSTANCE /*pEffectInstances*/, DWORD NumMaterials, DWORD *pAdjacency, LPD3DXSKININFO pSkinInfo,
    LPD3DXMESHCONTAINER *ppNewMeshContainer)
    {
      //printf("CreateMeshCont: \"%s\"\n",Name);
      // Create a mesh container using custom struct
      LPMESHCONTAINER pMeshContainer = new MESHCONTAINER;
      ZeroMemory(pMeshContainer, sizeof(MESHCONTAINER));
      *ppNewMeshContainer = pMeshContainer; // Set the output mesh container to the one we've created

      pMeshContainer->Name= Name ? _strdup(Name) : 0;
      pMeshContainer->MeshData.Type = D3DXMESHTYPE_MESH;
      // Get the number of Faces for adjacency
      DWORD dwFaces = pMeshData->pMesh->GetNumFaces();
      //Get Initialize all the other data
      pMeshContainer->NumMaterials = NumMaterials;
      //Create the arrays for the materials and the textures
      pMeshContainer->pMaterials9 = new D3DMATERIAL9[pMeshContainer->NumMaterials];
      // Multiply by 3 because there are three adjacent triangles
      pMeshContainer->pAdjacency = new DWORD[dwFaces*3];
      memcpy(pMeshContainer->pAdjacency, pAdjacency, sizeof(DWORD) * dwFaces*3);
      
      LPDIRECT3DDEVICE9 pd3dDevice = NULL;  //Get the Direct3D Rendering device to use
      pMeshData->pMesh->GetDevice(&pd3dDevice);
      pMeshData->pMesh->CloneMeshFVF(D3DXMESH_MANAGED, pMeshData->pMesh->GetFVF(), pd3dDevice, &pMeshContainer->MeshData.pMesh);
      pMeshContainer->ppTextures  = new LPDIRECT3DTEXTURE9[NumMaterials];
      for(DWORD dw = 0; dw < NumMaterials; dw++)
      {
        pMeshContainer->pMaterials9[dw]=pMaterials[dw].MatD3D;
        pMeshContainer->ppTextures[dw] = NULL;
        if(pMaterials[dw].pTextureFilename && strlen(pMaterials[dw].pTextureFilename) > 0)
        {
          if(FAILED(D3DXCreateTextureFromFile(pd3dDevice, pMaterials[dw].pTextureFilename, &pMeshContainer->ppTextures[dw])))
            pMeshContainer->ppTextures [dw] = NULL;
          else
          {
            D3DMATERIAL9& mat=pMeshContainer->pMaterials9[dw];
            mat.Diffuse.a=mat.Diffuse.r=mat.Diffuse.g=mat.Diffuse.b=1.0f;
            mat.Ambient=mat.Diffuse;
            mat.Specular.a=mat.Specular.r=mat.Specular.g=mat.Specular.b=0.0f;
          }
        }
      }
      SAFE_RELEASE(pd3dDevice); //Release the device
      if(pSkinInfo)
      {
        // first save off the SkinInfo and original mesh data
        pMeshContainer->pSkinInfo = pSkinInfo;
        pSkinInfo->AddRef();
        // Will need an array of offset matrices to move the vertices from the figure space to the bone's space
        UINT uBones = pSkinInfo->GetNumBones();
        pMeshContainer->pBoneOffsets = new D3DXMATRIX[uBones];
        //Create the arrays for the bones and the frame matrices
        pMeshContainer->ppFrameMatrices = new D3DXMATRIX*[uBones];
        // get each of the bone offset matrices so that we don't need to get them later
        for (UINT i = 0; i < uBones; i++)
          pMeshContainer->pBoneOffsets[i] = *(pMeshContainer->pSkinInfo->GetBoneOffsetMatrix(i));
      }
      else
      {
        pMeshContainer->pSkinInfo = NULL;
        pMeshContainer->pBoneOffsets = NULL;
        pMeshContainer->pSkinMesh = NULL;
        pMeshContainer->ppFrameMatrices = NULL;
      }
      pMeshContainer->pMaterials = NULL;
      pMeshContainer->pEffects = NULL;
      //pMeshContainer->MeshData.pMesh->OptimizeInplace(
      //	D3DXMESHOPT_VERTEXCACHE|D3DXMESHOPT_COMPACT|D3DXMESHOPT_ATTRSORT,
      //	pMeshContainer->pAdjacency,NULL,NULL,NULL);
      return S_OK;
  }
  // Destroy a frame
  //1. The frame to delete
  STDMETHOD(DestroyFrame)(THIS_ LPD3DXFRAME pFrameToFree)
  {
    //printf("DestroyFrame: \"%s\"\n",pFrameToFree->Name);
    LPFRAME pFrame = (LPFRAME)pFrameToFree; //Convert the frame
    SAFE_DELETE_ARRAY(pFrame->Name);        // Delete the name
    SAFE_DELETE(pFrame);                    // Delete the frame
    return S_OK; 
  }
  // Destroy a mesh container
  //1. The container to destroy
  STDMETHOD(DestroyMeshContainer)(THIS_ LPD3DXMESHCONTAINER pMeshContainerBase)
  {
    //printf("DestroyMeshCont: \"%s\"\n",pMeshContainerBase->Name);
    LPMESHCONTAINER pMeshContainer = (LPMESHCONTAINER)pMeshContainerBase; //Convert to my derived struct type
    SAFE_DELETE_ARRAY(pMeshContainer->Name); // if there is a name
    SAFE_DELETE_ARRAY(pMeshContainer->pMaterials9); //if there are materials
    if(pMeshContainer->ppTextures) //Release the textures
      for(UINT i = 0; i < pMeshContainer->NumMaterials; ++i)
        SAFE_RELEASE(pMeshContainer->ppTextures[i]);
    
    SAFE_DELETE_ARRAY(pMeshContainer->ppTextures);   //if there are textures
    SAFE_DELETE_ARRAY(pMeshContainer->pAdjacency);   // if there is adjacency data 
    SAFE_DELETE_ARRAY(pMeshContainer->pBoneOffsets); // if there are bone parts
    SAFE_DELETE_ARRAY(pMeshContainer->ppFrameMatrices); //if there are frame matrices
    SAFE_DELETE_ARRAY(pMeshContainer->pAttributeTable);
    SAFE_RELEASE(pMeshContainer->pSkinMesh); //if there is a copy of the mesh here
    SAFE_RELEASE(pMeshContainer->MeshData.pMesh); //if there is a mesh
    SAFE_RELEASE(pMeshContainer->pSkinInfo); // if there is skin information
    SAFE_DELETE(pMeshContainer); //Delete the mesh container
    return S_OK;
  }
};

//#define SHADOWFVF D3DFVF_XYZRHW | D3DFVF_DIFFUSE //The FVF for the shadow vertex

// This is a DirectX9 wrapper for the loading skinned model from XFile recipient (one model/instance)
// It allows loading, drawing, texturing, and model animation.
class CModel  
{
private:
  static const DWORD NO_ANIMATION=0xFFFFFFFF;
  LPDIRECT3DDEVICE9	m_pd3dDevice;			      // The d3d device to use
  //Model
  LPMESHCONTAINER		m_pFirstMesh;			      // The first mesh in the hierarchy
  LPD3DXFRAME			  m_pFrameRoot;			      // Frame hierarchy of the model
  LPD3DXMATRIX		  m_pBoneMatrices;		    // Used when calculating the bone position
  D3DXVECTOR3			  m_vecCenter;			      // Center of bounding sphere of object
  float             m_fRadius;				      // Radius of bounding sphere of object
  UINT				      m_uMaxBones;			      // The Max number of bones for the model
  //Animation
  DWORD				      m_dwCurrentAnimation;	  // The current animation
  DWORD				      m_dwAnimationSetCount;	// Number of animation sets
  DOUBLE            m_dAnimPeriod;
  ID3DXAnimationController *m_pAnimController;// Controller for the animations

  // Purpose: go through each frame and draw the ones that have a mesh container that is not NULL
  // Parameters: LPFRAME pFrame	: The frame root
  // Notes: Called in the Draw Function
  void DrawFrame(LPFRAME pFrame);
  // Purpose: set up the bone matrices
  // Parameters:
  //	LPFRAME pFrame                // The frame root 
  //	LPD3DXMATRIX pParentMatrix	  // parent matrix
  void SetupBoneMatrices(LPFRAME pFrame, LPD3DXMATRIX pParentMatrix);
  // Purpose: Update the frame matrices after animation
  // Parameters:
  //	LPFRAME pFrame				      // Frame to use
  //	LPD3DXMATRIX pParentMatrix	// Matrix passed in
  // Notes: When called for frame root you can pass a null pParentMatrix
  void UpdateFrameMatrices(LPFRAME pFrame, LPD3DXMATRIX pParentMatrix=0)
  {	
    if(pParentMatrix) D3DXMatrixMultiply(&pFrame->matCombined, &pFrame->TransformationMatrix, pParentMatrix);
    else pFrame->matCombined = pFrame->TransformationMatrix;
    if(pFrame->pFrameSibling) UpdateFrameMatrices((LPFRAME)pFrame->pFrameSibling, pParentMatrix);
    if(pFrame->pFrameFirstChild) UpdateFrameMatrices((LPFRAME)pFrame->pFrameFirstChild, &pFrame->matCombined);
  }
  void UpdateFrameHierarchy()
  {
    if(m_pFrameRoot)
    {
      UpdateFrameMatrices((LPFRAME)m_pFrameRoot);
      LPMESHCONTAINER pMesh = m_pFirstMesh;
      if(pMesh)
      {
        if(pMesh->pSkinInfo)
        {
          UINT Bones = pMesh->pSkinInfo->GetNumBones();
          for (UINT i = 0; i < Bones; i++)
            D3DXMatrixMultiply(&m_pBoneMatrices[i], &pMesh->pBoneOffsets[i], pMesh->ppFrameMatrices[i]);
          // Lock the meshes' vertex buffers
          void *SrcPtr, *DestPtr;
          pMesh->MeshData.pMesh->LockVertexBuffer(D3DLOCK_READONLY, (void**)&SrcPtr);
          pMesh->pSkinMesh->LockVertexBuffer(0, (void**)&DestPtr);
          // Update the skinned mesh using provided transformations
          pMesh->pSkinInfo->UpdateSkinnedMesh(m_pBoneMatrices, NULL, SrcPtr, DestPtr);
          // Unlock the meshes vertex buffers
          pMesh->pSkinMesh->UnlockVertexBuffer();
          pMesh->MeshData.pMesh->UnlockVertexBuffer();
        }
      }
    }
  }
  CModel &operator=(const CModel&){}
  CModel(const CModel&){}

public:
  CModel(IDirect3DDevice9* pD3DDevice, const TCHAR* strFileName)
  {
    m_pd3dDevice = pD3DDevice;	
    m_pFrameRoot = NULL;			
    m_pBoneMatrices = NULL;			
    m_vecCenter = D3DXVECTOR3(0.0f,0.0f,0.0f);
    m_fRadius = 0.0f;				
    m_dwCurrentAnimation = NO_ANIMATION;	
    m_dwAnimationSetCount = 0;	
    m_uMaxBones = 0;
    m_pAnimController = NULL;
    m_pFirstMesh = NULL;
    LoadXFile(strFileName);
  }
  virtual ~CModel()
  {
    SAFE_RELEASE(m_pAnimController); //Delete Animation Controller
    if(m_pFrameRoot) // if there is a frame hierarchy
    {
      CAllocateHierarchy Alloc;
      D3DXFrameDestroy(m_pFrameRoot, &Alloc);
      m_pFrameRoot = NULL;
    }
    SAFE_DELETE_ARRAY(m_pBoneMatrices); //Delete the bones matrices
  }

  // Purpose: Return the center of the bounding sphere
  inline const D3DXVECTOR3* GetBoundingSphereCenter() const { return &m_vecCenter; }
  // Purpose: Return the Radius of the bounding sphere
  inline float GetBoundingSphereRadius() const { return m_fRadius; }
  // Purpose: Return the maximum number of available animations
  inline DWORD GetMaxAnimation() const { return m_dwAnimationSetCount; }
  // Purpose: Return the animation being used (current animation)
  inline DWORD GetCurrentAnimation() const { return m_dwCurrentAnimation; }
  // Purpose: Return the valid period (0..period) for the current animation set
  inline DOUBLE GetCurrentAnimationPeriod() const { return m_dAnimPeriod; }
  // Purpose: Set the current animation to the passed in dwAnimationFlag (the animation set to use)
  void SetCurrentAnimation(DWORD dwAnimationFlag=NO_ANIMATION);
  // Purpose: draw the model with the device given
  void Draw();
  // Purpose: Load the model from the .X file given in strFileName
  void LoadXFile(const TCHAR* strFileName);
  // Purpose: Increment the model's animation time by dElapsedTimeUpdate [in secondes]
  void IncTime(double dElapsedTime)
  {
    if(m_pAnimController && m_dwCurrentAnimation != NO_ANIMATION) m_pAnimController->SetTime(m_pAnimController->GetTime()+dElapsedTime);
    UpdateFrameHierarchy();
  }
  // Purpose: Initialize the model's animation time to dTime [in secondes]
  void SetTime(double dTime=0)
  {
    if(m_pAnimController && m_dwCurrentAnimation != NO_ANIMATION) m_pAnimController->SetTime(dTime);
    UpdateFrameHierarchy();
  }
  // Dump hierarchy information
  void DumpFrameInfo(FILE* pf, LPD3DXFRAME pFrame=0, int level=0)
  {
    if(pFrame==0)
    {
      pFrame=m_pFrameRoot;
      fprintf(pf,"DumpFrameInfo : bounding sphere [%7.2f, %7.2f, %7.2f] ray=%7.2f\n",m_vecCenter.x,m_vecCenter.y,m_vecCenter.z,m_fRadius);
    }
    fprintf(pf," L%dFrame : \"%s\"\n  ",level,pFrame->Name);
    for(int i=0; i<4; i++)
    {
      fprintf(pf,"[");
      for(int j=0; j<4; j++) fprintf(pf,(i==3 ? "%8.2f%c" : "%5.2f%c"),pFrame->TransformationMatrix.m[i][j],(j==3 ?']':','));
    }
    fprintf(pf,"\n");
    fprintf(pf,"  MeshCont=%p",pFrame->pMeshContainer);
    if(pFrame->pMeshContainer) fprintf(pf,", Name=\"%s\", SkinInfo=%p",pFrame->pMeshContainer->Name, pFrame->pMeshContainer->pSkinInfo);
    fprintf(pf,"\n");

    if(pFrame->pFrameSibling) DumpFrameInfo(pf,pFrame->pFrameSibling,level);
    if(pFrame->pFrameFirstChild) DumpFrameInfo(pf,pFrame->pFrameFirstChild,level+1);
  }
};


#endif // _XFILE_MODEL_H