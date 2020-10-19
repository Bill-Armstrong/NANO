// ALN Library

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

// file cmyaln.h
// callback destination

#ifndef _CMYALN_H
#define _CMYALN_H

#include <iostream>

extern "C" int _kbhit();
extern "C" int _getch();


class CMyAln;

class CMyAln : public CAln
{
    // notification overrides
public:
    virtual BOOL OnTrainStart(TRAININFO* pTrainInfo, void* pvData)
    {
        //cerr << "Training starts..." << std::endl;
        return TRUE;
    }

    virtual BOOL OnTrainEnd(TRAININFO* pTrainInfo, void* pvData)
    {
        std::cerr << "    RMSE " << pTrainInfo->fltRMSErr << " LFNs " << pTrainInfo->nLFNs;
        return TRUE;
    }

    virtual BOOL OnEpochStart(EPOCHINFO* pEpochInfo, void* pvData)
    {
        return TRUE;
    }

    virtual BOOL OnEpochEnd(EPOCHINFO* pEpochInfo, void* pvData)
    {
        //std::cerr << " Leaf nodes " << pEpochInfo->nLFNs << " Active leaf nodes " << pEpochInfo->nActiveLFNs;
        return TRUE;
    }

    virtual BOOL OnVectorInfo(VECTORINFO* pVectorInfo, void* pvData)
    {
        // getvector may be used for reinforcement learning
        if (_kbhit())
        {
            //cerr << endl << "Checking keyboard input (type 's' to stop training)" << endl;
            return _getch() != 's';
        }
        else
        {
            return TRUE;
        }
    }
};

#endif
