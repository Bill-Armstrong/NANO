
// DTREE binary I/O routines
/* MIT License

Copyright (c) 2020 William Ward Armstrong

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. */

// dtr_bio.c

#ifdef DTREEDLL
#define DTRIMP __declspec(dllexport)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>

#include <dtree.h>
#include "dtr_priv.h"

// Intel X64 is little endian
#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN
#endif

/* make sure we know the "endian"-ness of the target platform */
/* ... needed for binary file ops */
#if (!defined(BIG_ENDIAN) && !defined(LITTLE_ENDIAN))
#error Either BIG_ENDIAN or LITTLE_ENDIAN must be defined
#elif (defined(BIG_ENDIAN) && defined(LTTLE_ENDIAN))
#error Only one of BIG_ENDIAN or LITTLE_ENDIAN may be defined
#endif

#define BIG_ENDIAN_FLAG 0
#define LITTLE_ENDIAN_FLAG 1

#if defined(_MSC_VER)
/* disable constant conditional expression warning */
#pragma warning(disable : 4127)  
#endif

/* binary file identification string */
static char _szDTRIdent[] = "DTR\r";

/* binary I/O helpers */
int WriteBinMinMaxNode(FILE* pFile, MINMAXNODE* pMMN);
int ReadBinMinMaxNode(FILE* pFile, MINMAXNODE** ppMMN);

/* binary file import */

#define READ_OBJ(o) do { \
                      if (fread(&(o), sizeof((o)), 1, pFile) != 1) { \
                        DestroyDtree(*ppDtree); \
                        *ppDtree = NULL; \
                        return DTR_FILEREADERR; \
                      } \
                    } while (0)

#define READ_ARRAY(a, s, n) do { \
                              if (fread((a), (s), (n), pFile) != (size_t)(n)) { \
                                DestroyDtree(*ppDtree); \
                                *ppDtree = NULL; \
                                return DTR_FILEREADERR; \
                              } \
                            } while (0)

int ReadBinDtreeFile(FILE* pFile, DTREE** ppDtree)
{
  int nVer;
  int i, nErr;
  char cEndian;
  char szIdent[5];

  /* read file type bytes */
  if (fread(szIdent, 1, 5, pFile) != 5)
    return DTR_FILEREADERR;
    
  szIdent[4] = '\0';
  if (strcmp(szIdent, _szDTRIdent) != 0)
    return DTR_FILETYPEERR;
  
  /* read endian type - must only be one byte */
  if (fread(&cEndian, 1, 1, pFile) != 1)
    return DTR_FILEREADERR;

  #if defined(BIG_ENDIAN)
  if (cEndian != BIG_ENDIAN_FLAG)
    return DTR_ENDIANERR;
  #elif defined(LITTLE_ENDIAN)
  if (cEndian != LITTLE_ENDIAN_FLAG)
    return DTR_ENDIANERR;
  #endif

  /* read dtree version */
  if (fread(&nVer, sizeof(nVer), 1, pFile) != 1)
    return DTR_FILEREADERR;
  
  if (nVer > GetDtreeVersion())
    return DTR_UNKNOWNVERSION;

  /* read dtree header info */
  *ppDtree = CreateDtree();
  if (ppDtree == NULL)
    return DTR_MALLOCFAILED;
  
  READ_OBJ((*ppDtree)->nDim);
  READ_OBJ((*ppDtree)->nOutputIndex);
  READ_OBJ((*ppDtree)->nLinearForms);
  READ_OBJ((*ppDtree)->nBlocks);
  READ_OBJ((*ppDtree)->nNodes);

  /* read var defs */
  (*ppDtree)->aVarDefs = CreateVarDefArray((*ppDtree)->nDim);
  if ((*ppDtree)->aVarDefs == NULL)
  {
    DestroyDtree(*ppDtree);
    *ppDtree = NULL;
    return DTR_MALLOCFAILED;
  }
  for (i = 0; i < (*ppDtree)->nDim; i++)
  {
    int nLen;
    READ_OBJ(nLen);
    if (nLen > 0)
    {
      (*ppDtree)->aVarDefs[i].pszName = (char*)malloc((nLen + 1) * sizeof(char));
      if ((*ppDtree)->aVarDefs[i].pszName == NULL)
      {
        DestroyDtree(*ppDtree);
        *ppDtree = NULL;
        return DTR_MALLOCFAILED;
      }

      READ_ARRAY((*ppDtree)->aVarDefs[i].pszName, sizeof(char), nLen);
      (*ppDtree)->aVarDefs[i].pszName[nLen] = '\0';
    }
    
    READ_OBJ((*ppDtree)->aVarDefs[i].bound.fltMin);
    READ_OBJ((*ppDtree)->aVarDefs[i].bound.fltMax);
  }

  /* read linear forms */
  (*ppDtree)->aLinearForms = CreateLinearFormArray((*ppDtree)->nLinearForms, 
                                                   (*ppDtree)->nDim);
  if ((*ppDtree)->aLinearForms == NULL)
  {
    DestroyDtree(*ppDtree);
    *ppDtree = NULL;
    return DTR_MALLOCFAILED;
  }
  for (i = 0; i < (*ppDtree)->nLinearForms; i++)
  {
    READ_OBJ((*ppDtree)->aLinearForms[i].fltBias);
    READ_ARRAY((*ppDtree)->aLinearForms[i].afltW, sizeof(float), (*ppDtree)->nDim);
    READ_ARRAY((*ppDtree)->aLinearForms[i].afltC, sizeof(float), (*ppDtree)->nDim);
  }

  /* write blocks */
  (*ppDtree)->aBlocks = CreateBlockArray((*ppDtree)->nBlocks);
  if ((*ppDtree)->aBlocks == NULL)
  {
    DestroyDtree(*ppDtree);
    *ppDtree = NULL;
    return DTR_MALLOCFAILED;
  }
  for (i = 0; i < (*ppDtree)->nBlocks; i++)
  {
    READ_OBJ((*ppDtree)->aBlocks[i].nDtreeIndex);
    nErr = ReadBinMinMaxNode(pFile, &((*ppDtree)->aBlocks[i].pMinMaxTree));
    if (nErr != DTR_NOERROR)
      return nErr;
  }

  /* read dtree nodes */
  (*ppDtree)->aNodes = CreateDtreeNodeArray((*ppDtree)->nNodes);
  if ((*ppDtree)->aNodes == NULL)
  {
    DestroyDtree(*ppDtree);
    *ppDtree = NULL;
    return DTR_MALLOCFAILED;
  }
  for (i = 0; i < (*ppDtree)->nNodes; i++)
  {
    READ_OBJ((*ppDtree)->aNodes[i].nLeaf);
    READ_OBJ((*ppDtree)->aNodes[i].nParentIndex);

    if ((*ppDtree)->aNodes[i].nLeaf != 0)
    {
      READ_OBJ((*ppDtree)->aNodes[i].info.nBlockIndex);
    }
    else
    {
      READ_OBJ((*ppDtree)->aNodes[i].info.node.fltT);
      READ_OBJ((*ppDtree)->aNodes[i].info.node.nVarIndex);
      READ_OBJ((*ppDtree)->aNodes[i].info.node.nLeftIndex);
      READ_OBJ((*ppDtree)->aNodes[i].info.node.nRightIndex);
    }
  }

  return DTR_NOERROR;
}

int ReadBinMinMaxNode(FILE* pFile, MINMAXNODE** ppMMN)
{
  *ppMMN = CreateMinMaxNode();
  if (*ppMMN == NULL)
    return DTR_MALLOCFAILED;

  /* read type */
  if (fread(&(*ppMMN)->nType, sizeof((*ppMMN)->nType), 1, pFile) != 1)
    return DTR_FILEREADERR;

  if ((*ppMMN)->nType == DTREE_LINEAR)
  {
    if (fread(&MMN_LFINDEX(*ppMMN), sizeof(MMN_LFINDEX(*ppMMN)), 1, pFile) != 1)
      return DTR_FILEREADERR;
  }
  else
  {
    /* read list of children */
    MINMAXNODE** ppMMNChild = &MMN_CHILDLIST(*ppMMN);
    while (1)
    {
      char cPresent;  /* element present flag */
      if (fread(&cPresent, sizeof(cPresent), 1, pFile) != 1)
        return DTR_FILEREADERR;
      
      if (cPresent)
      {
        int nErr;
        nErr = ReadBinMinMaxNode(pFile, ppMMNChild);
        if (nErr != DTR_NOERROR)
          return nErr;

        /* advance child pointer */
        ppMMNChild = &((*ppMMNChild)->pNext);
      }
      else break;  /* finish reading child list */
    }
  }

  return DTR_NOERROR;
}


/* binary file export */

#define WRITE_OBJ(o) do { \
                       if (fwrite(&(o), sizeof((o)), 1, pFile) != 1) \
                         return DTR_FILEWRITEERR; \
                     } while (0)

#define WRITE_ARRAY(a, s, n) do { \
                              if (fwrite((a), (s), (n), pFile) != (size_t)(n)) \
                                return DTR_FILEWRITEERR; \
                             } while (0)


int WriteBinDtreeFile(FILE* pFile, DTREE* pDtree)
{
  int nVer;
  int i, nErr;
  char cEndian;

  /* write file type bytes, including NULL */
  if (fwrite(_szDTRIdent, 1, 5, pFile) != 5)
    return DTR_FILEWRITEERR;
  
  /* write endian type - must only be one byte */
  #if defined(BIG_ENDIAN)
  cEndian = BIG_ENDIAN_FLAG;
  #elif defined(LITTLE_ENDIAN)
  cEndian = LITTLE_ENDIAN_FLAG;
  #endif
  if (fwrite(&cEndian, 1, 1, pFile) != 1)
    return DTR_FILEWRITEERR;

  /* write dtree version */
  nVer = GetDtreeVersion();
  WRITE_OBJ(nVer);

  /* write dtree header info */
  WRITE_OBJ(pDtree->nDim);
  WRITE_OBJ(pDtree->nOutputIndex);
  WRITE_OBJ(pDtree->nLinearForms);
  WRITE_OBJ(pDtree->nBlocks);
  WRITE_OBJ(pDtree->nNodes);

  /* write var defs */
  for (i = 0; i < pDtree->nDim; i++)
  {
    int nLen = 0;
    if (pDtree->aVarDefs[i].pszName != NULL)
      nLen = (int)strlen(pDtree->aVarDefs[i].pszName);
   
    WRITE_OBJ(nLen);
    if (nLen > 0)
      WRITE_ARRAY(pDtree->aVarDefs[i].pszName, sizeof(char), nLen);
    
    WRITE_OBJ(pDtree->aVarDefs[i].bound.fltMin);
    WRITE_OBJ(pDtree->aVarDefs[i].bound.fltMax);
  }

  /* write linear forms */
  for (i = 0; i < pDtree->nLinearForms; i++)
  {
    WRITE_OBJ(pDtree->aLinearForms[i].fltBias);
    WRITE_ARRAY(pDtree->aLinearForms[i].afltW, sizeof(float), pDtree->nDim);
    WRITE_ARRAY(pDtree->aLinearForms[i].afltC, sizeof(float), pDtree->nDim);
  }

  /* write blocks */
  for (i = 0; i < pDtree->nBlocks; i++)
  {
    WRITE_OBJ(pDtree->aBlocks[i].nDtreeIndex);
    nErr = WriteBinMinMaxNode(pFile, pDtree->aBlocks[i].pMinMaxTree);
    if (nErr != DTR_NOERROR)
      return nErr;
  }

  /* write dtree nodes */
  for (i = 0; i < pDtree->nNodes; i++)
  {
    WRITE_OBJ(pDtree->aNodes[i].nLeaf);
    WRITE_OBJ(pDtree->aNodes[i].nParentIndex);

    if (pDtree->aNodes[i].nLeaf != 0)
    {
      WRITE_OBJ(pDtree->aNodes[i].info.nBlockIndex);
    }
    else
    {
      WRITE_OBJ(pDtree->aNodes[i].info.node.fltT);
      WRITE_OBJ(pDtree->aNodes[i].info.node.nVarIndex);
      WRITE_OBJ(pDtree->aNodes[i].info.node.nLeftIndex);
      WRITE_OBJ(pDtree->aNodes[i].info.node.nRightIndex);
    }
  }

  return DTR_NOERROR;
}

int WriteBinMinMaxNode(FILE* pFile, MINMAXNODE* pMMN)
{
  /* write type */
  WRITE_OBJ(pMMN->nType);

  if (pMMN->nType == DTREE_LINEAR)
  {
    WRITE_OBJ(MMN_LFINDEX(pMMN));
  }
  else
  {
    /* write list of children */
    MINMAXNODE* pMMNChild = MMN_CHILDLIST(pMMN);
    while (1)
    {
      char cPresent = (char)(pMMNChild != NULL);  /* element present flag */
      WRITE_OBJ(cPresent);
      
      if (pMMNChild != NULL)
      {
        int nErr;
        nErr = WriteBinMinMaxNode(pFile, pMMNChild);
        if (nErr != DTR_NOERROR)
          return nErr;

        /* advance child pointer */
        pMMNChild = pMMNChild->pNext;
      }
      else break;  /* finish writing child list */
    }
  }

  return DTR_NOERROR;
}
