
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

// alnver.h



#ifndef __ALNVER_H__
#define __ALNVER_H__

/* aln library version */

#ifndef ALNLIBVER

#define ALNLIBVER 0x00020001
// Version 0x00010009 improved speed and DTREE building
// Version 0x00020001 includes better DTREE optimization
#endif  /* ALNLIBVER */


/* aln structure version */

#ifndef ALNVER

// Version 0x00030008->9 allowed trees to be of arbitrary structure not just alternating AND/OR
// Version 0x00030009->10 changed SEy to SEE to avoid confusion.

#define ALNVER 0x00030010

#endif  /* ALNVER */


/*
///////////////////////////////////////////////////////////////////////////////
*/

#endif  /* __ALNVER_H__ */
