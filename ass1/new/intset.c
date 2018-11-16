#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include "postgres.h"

#include "access/hash.h"
#include "catalog/pg_collation.h"
#include "utils/builtins.h"
#include "utils/formatting.h"


#include "fmgr.h"
#include "libpq/pqformat.h"    
void make(char *s,int *a){
  int i,number=0,flag=1,n=0;
  for (i=1;i<strlen(s);i++){
    if (s[i]=='-'){
      flag=-1;
    }
    else if (s[i]==',' ||s[i]=='}'){
      a[n]=flag*number;
      number=0;
      flag=1;
      n=n+1;
    }
    else {
      number=number*10+s[i]-48;
    }
  }
}
int getlong(char *s){
  int i,n=0;
  for (i=0;i<strlen(s);i++)
    if (s[i]==',')
      n=n+1;
    return n;
}
void printnow(int *data,char *result,int n,int most){
  result[0] = '{';
  int i,x, nn;
  int num = 0;
  nn = 1;
  i = 0;
  char mid[most];
  while (i < n) {
    if (i == 0 || data[i] != data[i - 1]) {
      
      num = data[i];
      sprintf(mid, "%d", num);
      for (x = 0; x < strlen(mid); x++) {
        result[nn] = mid[x];
        nn = nn + 1;
      }
      result[nn] = ',';
      nn = nn + 1;;
    }
    
    i = i + 1;
    
  }
  result[nn - 1] = '}';
  result[nn] = '\0';
  if (n==0){
    result[0]='{';
    result[1]='}';
    result[2]='\0';
  }
}
PG_MODULE_MAGIC;
typedef struct intset
{
	int number;
	struct intset *next;
}intset;

Datum        intset_in(PG_FUNCTION_ARGS);
Datum        intset_out(PG_FUNCTION_ARGS);
/*****************************************************************************
* Input/Output functions
*****************************************************************************/
PG_FUNCTION_INFO_V1(intset_in);

Datum
intset_in(PG_FUNCTION_ARGS)
{
	char    *str = PG_GETARG_CSTRING(0);

	int length, n, i, f, j, isblock, isdigit;


	length = strlen(str);
	char *new;
	isdigit = 0;//0 for symbol ,1 for digit
	new = (char*)malloc(sizeof(char)*length);
	f = 1;
	n = 1;
	j = 0;
	isblock = 0; // 0 for no blocks ,1 for blocks between

	for (i = 0; i <length; i++) {
		if ((str[i] >= '0' && str[i] <= '9') || str[i] == '-') {

			if (isblock == 1 && isdigit == 1) {
				f = 0;
				break;
			}
			isdigit = 1;
			isblock = 0;
		}
		else
			if (str[i] == ' ')
				isblock = 1;
			else
				isdigit = 0;
		if (str[i] != ' ') {
			new[j] = str[i];

			j = j + 1;

		}

	}

	new[j] = '\0';
	if (new[0] != '{' || new[j - 1] != '}')
		f = 0;

	int number;
	int flag;
	int k;
	int *data;
	isdigit = 0;//0 for symbol ,1 for digit
	k = 0;
	for (i = 0; i<strlen(new); i++) {
		if (new[i] == ',')
			k = k + 1;
	}
	data = (int*)malloc(sizeof(int)*k);
	n = 0;
	flag = 1;
	number = 0;
	char result[length], mid[length];
	new[strlen(new) - 1] = ',';
	if (strlen(new) != 2) {

		for (i = 1; i < strlen(new); i++) {

			if (new[i] == '-') {
				if (new[i - 1] != ',' && new[i - 1] != '{')
				{
					f = 0;
					break;
				}
				flag = -1;
			}
			else if (new[i] >= '0' && new[i] <= '9') {

				number = number * 10 + new[i] - 48;

			}
			else if (new[i] == ',') {
				if (new[i - 1]<'0' || new[i - 1]>'9') {
					f = 0;
					break;
				}
				data[n] = number * flag;

				number = 0;
				n = n + 1;
				flag = 1;

			}
			else
			{
				f = 0;
				break;
			}
		}

		for (i = 0; i < n; i++) {

			for (j = i + 1; j < n; j++) {
				if (data[i] > data[j]) {
					number = data[i];
					data[i] = data[j];
					data[j] = number;
				}
			}
		}





		result[0] = '{';
		int x, nn;
		int num = 0;
		nn = 1;
		i = 0;
		while (i < n) {
			if (i == 0 || data[i] != data[i - 1]) {

				num = data[i];
				sprintf(mid, "%d", num);
				for (x = 0; x < strlen(mid); x++) {
					result[nn] = mid[x];
					nn = nn + 1;
				}
				result[nn] = ',';
				nn = nn + 1;;
			}

			i = i + 1;

		}
		result[nn - 1] = '}';
		result[nn] = '\0';

	}
	else
	{
		result[0] = '{';
		result[1] = '}';
		result[2] = '\0';
	}
	if (f == 0) {
		ereport(ERROR,
			(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
				errmsg("invalid input syntax for intset: \"%{digit,digit,....}\"",
					str)));
	}
	PG_RETURN_TEXT_P(cstring_to_text(result));
}

PG_FUNCTION_INFO_V1(intset_out);

Datum
intset_out(PG_FUNCTION_ARGS)
{
	char *result = text_to_cstring(PG_GETARG_TEXT_PP(0));
	PG_RETURN_CSTRING(result);


}


PG_FUNCTION_INFO_V1(intset_difference);

Datum
  intset_difference(PG_FUNCTION_ARGS)
  {
    
    char *s1=text_to_cstring(PG_GETARG_TEXT_PP(0));
    char *s2=text_to_cstring(PG_GETARG_TEXT_PP(1));
    int n1=getlong(s1)+1;
    int n2=getlong(s2)+1;
    if (strlen(s1)==2) n1=0;
    if (strlen(s2)==2) n2=0;
    int a1[n1];
    int a2[n2];
    int a3[n1+n2];
    int i,j,n3;
    make(s1,a1);
    make(s2,a2);
    char result[strlen(s1)+strlen(s2)];
    i=0;
    j=0;
    n3=0;
    while (i<n1){
      if (j>=n2){
        a3[n3]=a1[i];
        n3=n3+1;
        i=i+1;
      }
      else{
        if (a1[i]<a2[j]){
          a3[n3]=a1[i];
          n3=n3+1;
          i=i+1;       
        }
        else if (a1[i]>a2[j]){
          j=j+1;
        }
        else{
          i=i+1;
          j=j+1;
        }
      }
    }
    printnow(a3,result,n3,strlen(s1)+strlen(s2));
    
    
    
    PG_RETURN_TEXT_P(cstring_to_text(result));
  }


PG_FUNCTION_INFO_V1(intset_intersection);

Datum
  intset_intersection(PG_FUNCTION_ARGS)
  {
    
    char *s1=text_to_cstring(PG_GETARG_TEXT_PP(0));
    char *s2=text_to_cstring(PG_GETARG_TEXT_PP(1));
    int n1=getlong(s1)+1;
    int n2=getlong(s2)+1;
    if (strlen(s1)==2) n1=0;
    if (strlen(s2)==2) n2=0;
    int a1[n1];
    int a2[n2];
    int a3[n1+n2];
    int i,j,n3;
    make(s1,a1);
    make(s2,a2);
    char result[strlen(s1)+strlen(s2)];
    i=0;
    j=0;
    n3=0;
    while (i<n1 && j<n2){
      if (a2[j]==a1[i]){
        a3[n3]=a1[i];
        n3=n3+1;
        i=i+1;
        j=j+1;
      }
      else{
        if (a1[i]<a2[j]){
          i=i+1;       
        }
        else if (a1[i]>a2[j]){
          j=j+1;
        }
      }
    }
    printnow(a3,result,n3,strlen(s1)+strlen(s2));
    
    
    
    PG_RETURN_TEXT_P(cstring_to_text(result));
  }

PG_FUNCTION_INFO_V1(intset_union);

Datum
  intset_union(PG_FUNCTION_ARGS)
  {
    
    char *s1=text_to_cstring(PG_GETARG_TEXT_PP(0));
    char *s2=text_to_cstring(PG_GETARG_TEXT_PP(1));
    int n1=getlong(s1)+1;
    int n2=getlong(s2)+1;
    if (strlen(s1)==2) n1=0;
    if (strlen(s2)==2) n2=0;
    int a1[n1];
    int a2[n2];
    int a3[n1+n2];
    int i,j,n3;
    make(s1,a1);
    make(s2,a2);
    char result[strlen(s1)+strlen(s2)];
    i=0;
    j=0;
    n3=0;
    while (i<n1 && j<n2){
      if (a2[j]==a1[i]){
        a3[n3]=a1[i];
        n3=n3+1;
        i=i+1;
        j=j+1;
      }
      else{
        if (a1[i]<a2[j]){
          a3[n3]=a1[i];
          i=i+1;  
          n3=n3+1;

        }
        else if (a1[i]>a2[j]){
          a3[n3]=a2[j];
          n3=n3+1;
          
          j=j+1;
        }
      }
    }
    while (i<n1){
      a3[n3]=a1[i];
      i=i+1;
      n3=n3+1;
    }
    while (j<n2){
      a3[n3]=a2[j];
      j=j+1;
      n3=n3+1;
    }
    printnow(a3,result,n3,strlen(s1)+strlen(s2));
    
    
    
    PG_RETURN_TEXT_P(cstring_to_text(result));
  }


PG_FUNCTION_INFO_V1(intset_disjunction);

Datum
  intset_disjunction(PG_FUNCTION_ARGS)
  {
    
    char *s1=text_to_cstring(PG_GETARG_TEXT_PP(0));
    char *s2=text_to_cstring(PG_GETARG_TEXT_PP(1));
    int n1=getlong(s1)+1;
    int n2=getlong(s2)+1;
    if (strlen(s1)==2) n1=0;
    if (strlen(s2)==2) n2=0;
    int a1[n1];
    int a2[n2];
    int a3[n1+n2];
    int i,j,n3;
    make(s1,a1);
    make(s2,a2);
    char result[strlen(s1)+strlen(s2)];
    i=0;
    j=0;
    n3=0;
    while (i<n1 && j<n2){
      if (a2[j]==a1[i]){
        i=i+1;
        j=j+1;
      }
      else{
        if (a1[i]<a2[j]){
          a3[n3]=a1[i];
          i=i+1;  
          n3=n3+1;
          
        }
        else if (a1[i]>a2[j]){
          a3[n3]=a2[j];
          n3=n3+1;
          
          j=j+1;
        }
      }
    }
    while (i<n1){
      a3[n3]=a1[i];
      i=i+1;
      n3=n3+1;
    }
    while (j<n2){
      a3[n3]=a2[j];
      j=j+1;
      n3=n3+1;
    }
    printnow(a3,result,n3,strlen(s1)+strlen(s2));
    
    
    
    PG_RETURN_TEXT_P(cstring_to_text(result));
  }
//----------------length-----------------------------
PG_FUNCTION_INFO_V1(length);
Datum
length(PG_FUNCTION_ARGS){
    char *string1;
    string1=text_to_cstring(PG_GETARG_TEXT_PP(0));;
    int length,i;
    length=1;
    for (i=0;i<strlen(string1);i++){
        if (string1[i]==',')
            length=length+1;
    }
    PG_RETURN_INT32(length);
}

//------------------contain_intger---------------------
PG_FUNCTION_INFO_V1(contain_intger);
Datum
contain_intger(PG_FUNCTION_ARGS){
    int t = PG_GETARG_INT32;
    int length,n,number,i,result;
    char *string1;
    string1=text_to_cstring(PG_GETARG_TEXT_PP(0));
    int *intset1;
    intset1=(int *)malloc(sizeof(int)*length);
    n=0;
    number=0;
    int flag=1;
    result = 0;
    if (strlen(string1) == 2){
        result =0;
    }
    if (strlen(string1) >2 ){
    for (i=1;i<strlen(string1);i++){
        if (string1[i]=='-'){
            flag=-1;
        }
        else if (string1[i]==',' || string1[i]=='}'){
            intset1[n]=number*flag;
            n=n+1;
            number=0;
            flag=1;
        }
        else{
            number=number*10+string1[i]-48;
        }
    }
    for (i=0;i<n;i++){
        if (intset1[i]==t){
            result =1;
        }
        
    }
    PG_RETURN_BOOL(result == 1);
}
    
//----------------------------contain_intset--------------
PG_FUNCTION_INFO_V1(contain_intset);
Datum
contain_intset(PG_FUNCTION_ARGS){
    char *string1= text_to_cstring(PG_GETARG_TEXT_PP(0));
    char *string2= text_to_cstring(PG_GETARG_TEXT_PP(1));
    int length1,length2,n1,n2,result1,number,j,k,flag,i;
    for (j=0;j<strlen(string1);j++){
        if (string1[j]==',')
            length1=length1+1;
    }
    for (k=0;k<strlen(string2);k++){
        if (string2[k]==',')
            length2=length2+1;
    }
    int *intset1=(int *)malloc(sizeof(int)*length1);
    int *intset2=(int *)malloc(sizeof(int)*length2);
    result1 = 0;
    n1=0;
    number=0;
    flag=1;
    for (i=1;i<strlen(string1);i++){
        if (string1[i]=='-'){
            flag=-1;
        }
        else if (string1[i]==',' || string1[i]=='}'){
            intset1[n1]=number*flag;
            n1=n1+1;
            number=0;
            flag=1;
        }
        else{
            number=number*10+string1[i]-48;
        }
    }
    n2=0;
    number=0;
    flag=1;
    if (strlen(string1) == 2){
        result1 = 1;
    }
    for (i=1;i<strlen(string2);i++){
        if (string2[i]=='-'){
            flag=-1;
        }
        else if (string2[i]==',' || string2[i]=='}'){
            intset2[n2]=number*flag;
            n2=n2+1;
            number=0;
            flag=1;
        }
        else{
            number=number*10+string2[i]-48;
        }
    }
    if(n1>n2){
        result1 = 0;
    }
    int count =0;
    if (n1 <= n2){
        for (i=0;i<n2;i++){
            for (int j =0;j<n1;j++){
                if (intset1[j] == intset2[i]){
                    count ++;
                }
            }
        }
    }
    if (count == n1){
        result1 = 1;
    }
    PG_RETURN_BOOL(result1 == 1);
    }
    
//-----------------------compare_equal------------------------
PG_FUNCTION_INFO_V1(compare_equal);
    
Datum
compare_equal(PG_FUNCTION_ARGS){
    char *string1= text_to_cstring(PG_GETARG_TEXT_PP(0));
    char *string2= text_to_cstring(PG_GETARG_TEXT_PP(1));
    int length1,length2,n1,n2,number,i,j,k,flag,result,count;
    length1 = 1;
    length2 = 1;
    for (j=0;j<strlen(string1);j++){
        if (string1[j]==',')
            length1=length1+1;
    }
    for (k=0;k<strlen(string2);k++){
        if (string2[k]==',')
            length2=length2+1;
    }
    int *intset1 =(int *)malloc(sizeof(int)*length1);
    int *intset2 =(int *)malloc(sizeof(int)*length2);
    n1=0;
    number=0;
    flag=1;
    result = 0;
    for (i=1;i<strlen(string1); i++){
        if (string1[i]=='-'){
            flag=-1;
        }
        else if (string1[i]==',' || string1[i]=='}'){
            intset1[n1] = number*flag;
            n1=n1+1;
            number=0;
            flag=1;
        }else{
            number = number*10+string1[i]-48;
        }
    }
    n2=0;
    number=0;
    flag=1;
    count =0;
    for (i=1;i<strlen(string2);i++){
        if (string2[i]=='-'){
            flag=-1;
        }
        else if (string2[i]==',' || string2[i]=='}'){
            intset2[n2]=number*flag;
            n2=n2+1;
            number=0;
            flag=1;
        }
        else{
            number=number*10+string2[i]-48;
        }
    }
    if (n2 == n1){
        for (i =0;i<n1;i++){
            for(j = 0;j<n2;j++){
                if (intset1[i] == intset2[j]){
                    count ++;
                }
            }
        }
    }
    if (count == n1){
        result =1;
    }
    PG_RETURN_BOOL(result == 1);
}

