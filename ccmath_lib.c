/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */


/*             CCMATH mathematics library source code.
 *
 *  Copyright (C)  2000   Daniel A. Atkinson    All rights reserved.
 *  This code may be redistributed under the terms of the GNU library
 *  public license (LGPL). ( See the lgpl.license file for details.)
 * ------------------------------------------------------------------------
 */

#include <stdlib.h>
#include <math.h>

#include "ccmath_lib.h"

void ccmath_cspl(double *x,double *y,double *z,int m,double tn)
{ double h,s,t,*pa,*pb,*a,*b; int j;
  if(tn==0.) tn=2.;
  else{ h=sinh(tn); tn=(tn*cosh(tn)-h)/(h-tn);}
  pa=(double *)calloc(2*m,sizeof(double)); pb=pa+m;
  h=x[1]-x[0]; t=(y[1]-y[0])/h;
  for(j=1,a=pa,b=pb; j<m ;++j){
    *a++ =tn*((*b=x[j+1]-x[j])+h); h= *b++;
    z[j]=(s=(y[j+1]-y[j])/h)-t; t=s;
   }
  for(j=2,a=pa,b=pb; j<m ;++j){ h= *b/ *a;
    *++a-=h* *b++; z[j]-=h*z[j-1]; }
  z[0]=z[m]=0.;
  for(j=m-1; j>0 ;--j){ z[j]-= *b-- *z[j+1]; z[j]/= *a--;}
  free(pa);
}

double ccmath_csfit(double w,double *x,double *y,double *z,int m)
{ double s,t; int j,k;
  if(w<x[0] || w>x[m]) return 0.;
  for(j=1; w>x[j] ;++j)
      ;
  k=j-1;
  s=(t=w-x[k])*(x[j]-w); t/=(x[j]-x[k]);
  return (t*y[j]+(1.-t)*y[k]-s*(z[j]*(1.+t)+z[k]*(2.-t)));
}

double ccmath_tnsfit(double w,double *x,double *y,double *z,int m,double tn)
{ double s,t,u,a=sinh(tn); int j,k;
  if(w<x[0] || w>x[m]) return 0.;
  for(j=1; w>x[j] ;++j)
      ;
  k=j-1;
  t=(w-x[k])/(s=x[j]-x[k]); s*=s/(a-tn); u=1.-t;
  return t*y[j]+u*y[k]+s*(z[j]*(sinh(tn*t)-t*a)+z[k]*(sinh(tn*u)-u*a));
}
