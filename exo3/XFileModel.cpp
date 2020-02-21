#include "XFileModel.h"

void CModel::LoadXFile(const TCHAR* strFileName)
{
  CAllocateHierarchy Alloc; //Allocation class
  //Load the mesh
  if(FAILED(D3DXLoadMeshHierarchyFromX(strFileName,		// File load
    D3DXMESH_MANAGED,	// Load Options
    m_pd3dDevice,		// D3D Device
    &Alloc,				// Hierarchy allocation class
    NULL,				// NO Effects
    &m_pFrameRoot,		// Frame hierarchy
    &m_pAnimController)))// Animation Controller
  {
    MessageBox(NULL, strFileName, "Model Load Error", MB_OK);
  }
  if(m_pAnimController)
    m_dwAnimationSetCount = m_pAnimController->GetMaxNumAnimationSets();

  if(m_pFrameRoot)
  {
    SetupBoneMatrices((LPFRAME)m_pFrameRoot, NULL); //Set the bones up
    //Setup the bone matrices array 
    m_pBoneMatrices  = new D3DXMATRIX[m_uMaxBones];
    ZeroMemory(m_pBoneMatrices, sizeof(D3DXMATRIX)*m_uMaxBones);
    //Calculate the Bounding Sphere
    D3DXFrameCalculateBoundingSphere(m_pFrameRoot, &m_vecCenter, &m_fRadius);
  }
  //DumpFrameInfo(stdout);
}

void CModel::SetupBoneMatrices(LPFRAME pFrame, LPD3DXMATRIX pParentMatrix)
{
  LPMESHCONTAINER pMesh = (LPMESHCONTAINER)pFrame->pMeshContainer;
  //Set up the bones on the mesh
  if(pMesh)
  {
    if(!m_pFirstMesh) m_pFirstMesh = pMesh;
    if(pMesh->pSkinInfo) // if there is a skinmesh, then setup the bone matrices
    {
      //Create a copy of the mesh
      pMesh->MeshData.pMesh->CloneMeshFVF(D3DXMESH_MANAGED, pMesh->MeshData.pMesh->GetFVF(), m_pd3dDevice, &pMesh->pSkinMesh);
      if(m_uMaxBones < pMesh->pSkinInfo->GetNumBones())
      {
        //Get the number of bones
        m_uMaxBones = pMesh->pSkinInfo->GetNumBones();
      }
      LPFRAME pTempFrame = NULL;
      for(UINT i = 0; i < pMesh->pSkinInfo->GetNumBones(); i++) //For each bone
      {   
        // Find the frame
        pTempFrame = (LPFRAME)D3DXFrameFind(m_pFrameRoot, pMesh->pSkinInfo->GetBoneName(i));
        //set the bone part
        pMesh->ppFrameMatrices[i] = &pTempFrame->matCombined;
      }
    }
  }
  //Check your Sister
  if(pFrame->pFrameSibling)
    SetupBoneMatrices((LPFRAME)pFrame->pFrameSibling, pParentMatrix);
  //Check your Son
  if(pFrame->pFrameFirstChild)
    SetupBoneMatrices((LPFRAME)pFrame->pFrameFirstChild, &pFrame->matCombined);
}

void CModel::SetCurrentAnimation(DWORD dwAnimationFlag)
{
  if(dwAnimationFlag == m_dwCurrentAnimation) return; // returns if the animation is one that we are already using
  m_dwCurrentAnimation = dwAnimationFlag;
  // if requested index is valid (lower than the number of animations)
  if(dwAnimationFlag < m_dwAnimationSetCount) 
  { 
    LPD3DXANIMATIONSET AnimSet = NULL;
    m_pAnimController->GetAnimationSet(m_dwCurrentAnimation, &AnimSet);
    m_pAnimController->SetTrackAnimationSet(0, AnimSet);
    m_dAnimPeriod = AnimSet->GetPeriod();
    SAFE_RELEASE(AnimSet);
    SetTime();
  }
  else if(dwAnimationFlag == NO_ANIMATION)
  { 
    m_pAnimController->SetTrackAnimationSet(0, 0);
    m_dAnimPeriod = 0;
    SetTime();
  }
}

void CModel::Draw()
{
  for(MESHCONTAINER *pMesh = m_pFirstMesh; pMesh; pMesh = (LPMESHCONTAINER)pMesh->pNextMeshContainer)
  {
    //Select the mesh to draw
    LPD3DXMESH pDrawMesh = (pMesh->pSkinInfo) ? pMesh->pSkinMesh: pMesh->MeshData.pMesh;
    //Draw each mesh subset with correct materials and texture
    for (DWORD i = 0; i < pMesh->NumMaterials; i++)
    {
      m_pd3dDevice->SetMaterial(pMesh->pMaterials9+i);
      m_pd3dDevice->SetTexture(0, pMesh->ppTextures[i]);
      pDrawMesh->DrawSubset(i);
    }
  }
}

void CModel::DrawFrame(LPFRAME pFrame)
{
  LPMESHCONTAINER pMesh = (LPMESHCONTAINER)pFrame->pMeshContainer;
  while(pMesh) //While there is a mesh try to draw it
  {
    //Select the mesh to draw
    LPD3DXMESH pDrawMesh = (pMesh->pSkinInfo) ? pMesh->pSkinMesh: pMesh->MeshData.pMesh;
    //Draw each mesh subset with correct materials and texture
    for (DWORD i = 0; i < pMesh->NumMaterials; ++i)
    {
      m_pd3dDevice->SetMaterial(&pMesh->pMaterials9[i]);
      m_pd3dDevice->SetTexture(0, pMesh->ppTextures[i]);
      pDrawMesh->DrawSubset(i);
    }
    pMesh = (LPMESHCONTAINER)pMesh->pNextMeshContainer; //Go to the next one
  }
  //Check your Sister
  if(pFrame->pFrameSibling)
    DrawFrame((LPFRAME)pFrame->pFrameSibling);
  //Check your Son
  if(pFrame->pFrameFirstChild)
    DrawFrame((LPFRAME)pFrame->pFrameFirstChild);
}
