/*
    Copyright 2011 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */

#define SCALE_FILTER_NAME       MAKENAME(_filter_DX_shaderproc)

static void SCALE_FILTER_NAME(const SkBitmapProcState& s, int x, int y,
                              DSTTYPE* SK_RESTRICT colors, int count) {
    SkASSERT((s.fInvType & ~(SkMatrix::kTranslate_Mask |
                             SkMatrix::kScale_Mask)) == 0);
    SkASSERT(s.fInvKy == 0);
    SkASSERT(count > 0 && colors != NULL);
    SkASSERT(s.fDoFilter);
    SkDEBUGCODE(CHECKSTATE(s);)

    const unsigned maxX = s.fBitmap->width() - 1;
    const SkFixed oneX = s.fFilterOneX;
    const SkFixed dx = s.fInvSx;
    SkFixed fx;
    const SRCTYPE* SK_RESTRICT row0;
    const SRCTYPE* SK_RESTRICT row1;
    unsigned subY;

    {
        SkPoint pt;
        s.fInvProc(*s.fInvMatrix, SkIntToScalar(x) + SK_ScalarHalf,
                   SkIntToScalar(y) + SK_ScalarHalf, &pt);
        SkFixed fy = SkScalarToFixed(pt.fY) - (s.fFilterOneY >> 1);
        const unsigned maxY = s.fBitmap->height() - 1;
        // compute our two Y values up front
        subY = TILEY_LOW_BITS(fy, maxY);
        int y0 = TILEY_PROCF(fy, maxY);
        int y1 = TILEY_PROCF((fy + s.fFilterOneY), maxY);

        const char* SK_RESTRICT srcAddr = (const char*)s.fBitmap->getPixels();
        unsigned rb = s.fBitmap->rowBytes();
        row0 = (const SRCTYPE*)(srcAddr + y0 * rb);
        row1 = (const SRCTYPE*)(srcAddr + y1 * rb);
        // now initialize fx
        fx = SkScalarToFixed(pt.fX) - (oneX >> 1);
    }

#ifdef OMAP_ENHANCEMENT
    SRCTYPE* row0_pld;
    SRCTYPE* row1_pld;
    DSTTYPE*  colors_pld;
    unsigned src_size = sizeof(SRCTYPE);
    //Cacheline width is 32 bytes or 8 * sizeof(int) for OMAP4
    int pld_src;
    int flag = 0; //To run pld at least once
    if (src_size == 1) {
        pld_src = 32;
    } else if (src_size == 2) {
        pld_src = 16;
    } else if (src_size == 4) {
       pld_src = 8;
    }
//Preloading destination array is not helping.
//Commenting the code for futur use
#if 0
    unsigned dst_size = sizeof(DSTTYPE);
    unsigned  pld_dst;
    if (dst_size == 1) {
        pld_dst = 32;
    } else if (dst_size == 2) {
        pld_dst = 16;
    } else if (dst_size == 4) {
        pld_dst = 8;
    }
#endif
#ifdef TARGET_OMAP4
     row0_pld = (SRCTYPE*)row0;
     row1_pld = (SRCTYPE*)row1;
     if (count > pld_src) {
         __builtin_prefetch (row0_pld, 0, 3);
         __builtin_prefetch (row0_pld + (1 * pld_src), 0, 3);
         row0_pld = row0_pld + (2 * pld_src);

         __builtin_prefetch (row1_pld, 0, 3);
         __builtin_prefetch (row1_pld + (1 * pld_src), 0, 3);
         row1_pld = row1_pld + (2 * pld_src);
     }
//Preloading destination array is not helping.
//Commenting the code for futur use
#if 0
         colors_pld = (DSTTYPE*) colors;
         __builtin_prefetch (colors_pld + (0 * pld_dst), 1, 2);
         __builtin_prefetch (colors_pld + (1 * pld_dst), 1, 2);
        //__builtin_prefetch (colors_pld + (2 * pld_dst), 1, 2);
        //__builtin_prefetch (colors_pld + (3 * pld_dst), 1, 2);
        colors_pld = colors_pld + (2 * pld_dst);
#endif
#endif
#endif

#ifdef PREAMBLE
    PREAMBLE(s);
#endif

    do {
#ifdef OMAP_ENHANCEMENT
#ifdef TARGET_OMAP4
        if ((count >= pld_src) || (flag ==0)) {
            __builtin_prefetch (row0_pld, 0, 3);
            __builtin_prefetch (row1_pld, 0, 3);
            row0_pld += pld_src;
            row1_pld += pld_src;
            flag = 1;
//Preloading destination array is not helping.
//Commenting the code for futur use
#if 0
        __builtin_prefetch (colors_pld + (0 * pld_dst), 1, 2);
        colors_pld += pld_dst;
#endif
        }
#endif
#endif

        unsigned subX = TILEX_LOW_BITS(fx, maxX);
        unsigned x0 = TILEX_PROCF(fx, maxX);
        unsigned x1 = TILEX_PROCF((fx + oneX), maxX);

        FILTER_PROC(subX, subY,
                    SRC_TO_FILTER(row0[x0]),
                    SRC_TO_FILTER(row0[x1]),
                    SRC_TO_FILTER(row1[x0]),
                    SRC_TO_FILTER(row1[x1]),
                    colors);
        colors += 1;

        fx += dx;
    } while (--count != 0);

#ifdef POSTAMBLE
    POSTAMBLE(s);
#endif
}

///////////////////////////////////////////////////////////////////////////////

#undef TILEX_PROCF
#undef TILEY_PROCF
#undef TILEX_LOW_BITS
#undef TILEY_LOW_BITS
#undef MAKENAME
#undef SRCTYPE
#undef DSTTYPE
#undef CHECKSTATE
#undef SRC_TO_FILTER
#undef FILTER_TO_DST
#undef PREAMBLE
#undef POSTAMBLE

#undef SCALE_FILTER_NAME
