// tsig.c ... functions on Tuple Signatures (tsig's)
// part of SIMC signature files
// Written by John Shepherd, September 2018

#include <unistd.h>
#include <string.h>
#include "defs.h"
#include "tsig.h"
#include "reln.h"
#include "hash.h"
#include "bits.h"

// make a tuple signature
Bits codeword(char *attr_value,Count m,Count k,int length){
    Count nbits=0;
    Bits cword=newBits(m);
    srandom(hash_any(attr_value, length));
    while (nbits<k){
        int i =random()%m;
        if (! bitIsSet(cword, i)){
            setBit(cword, i);
            nbits++;
        }
        
    }
    return cword;
}
Bits makeTupleSig(Reln r, Tuple t)
{
	assert(r != NULL && t != NULL);
    char **v1 = tupleVals(r, t);
    int i;
    Count m=r->params.tm;
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

// find "matching" pages using tuple signatures

void findPagesUsingTupSigs(Query q)
{
	assert(q != NULL);
	//TODO
    Reln r=q->rel;
    Tuple seltup=q->qstring;
    Bits this=makeTupleSig(r, seltup);
    File fr=r->tsigf;
    int i,j,tuppage;
    Page pnow;
    unsetAllBits(q->pages);
    int numm=0;
    for (i=0;i<r->params.tsigNpages;i++){
        pnow=getPage(fr, i);
        q->nsigpages=q->nsigpages+1;
        
        for (j=0;j<pageNitems(pnow);j++){
            q->nsigs++;
            tuppage=(int)((q->nsigs-1)/r->params.tupPP);
            int size=r->params.tsigSize;
            Byte *addr=addrInPage(pnow, j, size);
            Bits now=newBits(r->params.tm);
            Byte *add=giveback(now);
            
            
            memcpy(add, addr, size);
            if (isSubset(this, now)){
                setBit(q->pages, tuppage);

                numm++;
            }
            
        }
    }
    
    

	// The printf below is primarily for debugging
	// Remove it before submitting this function
	printf("Matched Pages:"); showBits(q->pages); putchar('\n');
}
