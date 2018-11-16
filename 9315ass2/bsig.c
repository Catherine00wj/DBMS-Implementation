// bsig.c ... functions on Tuple Signatures (bsig's)
// part of SIMC signature files
// Written by John Shepherd, September 2018

#include "defs.h"
#include "reln.h"
#include "query.h"
#include "bsig.h"
#include "psig.h"
void findPagesUsingBitSlices(Query q)
{
	assert(q != NULL);
    Reln r=q->rel;
    Tuple seltup=q->qstring;
    RelnParams *rp = &(r->params);
    Bits pagesig=makePageSig(r, seltup);
    int off,i;
    Byte *addr;
    Page p;
    setAllBits(q->pages);
    Bits zero=newBits(nPages(r));
    Count pid;
    Count size=rp->bsigSize;
    Byte *add=giveback(zero);
    Count num=givenumber(q->pages);
    int last=-1;
    for (i=0;i<rp->pm;i++){
        if (bitIsSet(pagesig, i)){
            q->nsigs++;
            pid=(int)(i/rp->bsigPP);
            off=(int)(i % rp->bsigPP);
            if (pid!=last){
                q->nsigpages++;
                last=pid;
            }
            p=getPage(r->bsigf, pid);
            addr=addrInPage(p, off, size);
            memcpy(add, addr, num);
            andBits(q->pages, zero);

        }
    }
    printf("Matched Pages:"); showBits(q->pages); putchar('\n');
}

