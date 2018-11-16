// psig.c ... functions on page signatures (psig's)
// part of SIMC signature files
// Written by John Shepherd, September 2018

#include "defs.h"
#include "reln.h"
#include "query.h"
#include "psig.h"
#include "tsig.h"
Bits makePageSig(Reln r, Tuple t)
{
    assert(r != NULL && t != NULL);
    char **v1 = tupleVals(r, t);
    int i;
    Count m=r->params.pm;
    Count k=r->params.tk;
    Bits all=newBits(m);
    Bits this=newBits(m);
    int length;
    unsetAllBits(all);
    for (i=0;i<r->params.nattrs;i++){
        if (i==0)
            length=7;
        if (i==1)
            length=20;
        if (i>1)
            length=6;
        if (v1[i][0]!='?'){
            this=codeword(v1[i], m, k,length);
            orBits(all,this);
        }
    }
    
    return all;
}

void findPagesUsingPageSigs(Query q)
{
    Reln r=q->rel;
    Tuple seltup=q->qstring;
    Bits this=makePageSig(r, seltup);
    File fr=r->psigf;
    int i,j;
    Page pnow;
    unsetAllBits(q->pages);
    int numm=0;
    for (i=0;i<r->params.psigNpages;i++){
        pnow=getPage(fr, i);
        q->nsigpages=q->nsigpages+1;
        
        for (j=0;j<pageNitems(pnow);j++){
            q->nsigs++;
            int size=r->params.psigSize;
            Byte *addr=addrInPage(pnow, j, size);
            Bits now=newBits(r->params.pm);
            Byte *add=giveback(now);
            memcpy(add, addr, size);
            
            
            
            if (isSubset(this, now)){
                setBit(q->pages, q->nsigs-1);
                
                numm++;
            }
            
        }
    }
    
    
    
    // The printf below is primarily for debugging
    // Remove it before submitting this function
    printf("Matched Pages:"); showBits(q->pages); putchar('\n');
}

