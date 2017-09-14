/*
 * This file is part of Lighttwist.
 * 
 * @Copyright 2004-2008 Université de Montréal, Laboratoire Vision3D
 *   Sébastien Roy (roys@iro.umontreal.ca)
 *   Vincent Chapdelaine-Couture (chapdelv@iro.umontreal.ca)
 *   Louis Bouchard (lwi.bouchard@gmail.com)
 *   Jean-Philippe Tardif
 *   Patrick Holloway
 *   Nicolas Martin
 *   Vlad Lazar
 *   Jamil Draréni 
 *   Marc-Antoine Drouin
 * @Copyright 2005-2007 Société des arts technologiques
 *
 * Lighttwist is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Lighttwist is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Lighttwist.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "uvmap.h"

//#define RADIAL_CORRECTION

#ifdef RADIAL_CORRECTION

/**
 * On suppose que le 1er cercle est celui du haut, donc r=1 a la fin
 * et le second cercle est celui du bas r=0 a la fin.
 * Et dans l'image, le rayon le plus petit est celui du haut...
 */
double radial_correction(double r)
{
	// directement de mathematica!!!

	//return ( 0.00206815+ 18.468*r - 5.29374*r*r - 15.503*atan(r) );
	//return( -1.97552*r + 3.78855*atan(r) );
	//return( 2.00376*r - 1.00376*r*r);
	//return(-0.000275349 + 1.20382*r - 1.45919*r*r + 1.25449*r*r*r);
	//return(0.00191468+ 1.21883*r - 1.51127*r*r + 1.28994*r*r*r);
	//return(-0.000505295 + 1.11748*r - 1.79*r*r + 1.67164*r*r*r);
	return(0.000596145+ 2.06609*r - 2.35854*r*r + 1.28994*r*r*r);
	//return(r);
}

#endif


//uv coordinates
uv_coord_struct uv_coord[NB_UV_COORD_MAX];
double uv_inner_scale, uv_outer_scale;
double uv_vcenter;
imgu *uv_map=NULL;
int uv_type;
matrix* uv_homography=NULL;
matrix* uv_homography_inv=NULL;
triangle uv_triangles[NB_TRIANGLES];

void uvGetCoords(uv_coord_struct *coord)
{
  int i;
  //(*coord)=(uv_coord_struct *)(malloc(sizeof(uv_coord_struct)*NB_UV_COORD_MAX));

  for(i=0;i<NB_UV_COORD_MAX;i++) coord[i]=uv_coord[i];
}

void uvSetCoords(uv_coord_struct *coord)
{
  int i;

  for(i=0;i<NB_UV_COORD_MAX;i++) uv_coord[i]=coord[i];
}

void uvGetScales(double *inner, double *outer)
{
  (*inner)=uv_inner_scale;
  (*outer)=uv_outer_scale;
}

void uvSetScales(double inner, double outer)
{
  uv_inner_scale=inner;
  uv_outer_scale=outer;
}

void uvGetVCenter(double *center)
{
  (*center)=uv_vcenter;
}

void uvSetVCenter(double center)
{
  uv_vcenter=center;
}

void uvGetType(int *type)
{
  (*type)=uv_type;
}

void uvSetType(int type)
{
  uv_type=type;
}

void uvGetImg(imgu **uvmap)
{
  (*uvmap)=uv_map;
}

/*! returns the center xc,yc and the ray of the circle passing by these three points*/
void uvEllipseCenter( float x1,float y1, float x2,float y2, float x3,float y3,
		float *xc,float *yc,float *r)
{
float denom;


	denom=2*(x3*(y1 - y2) + x1*(y2 - y3) + x2*(y3 - y1));

	*xc=-(x2*x2*y1) + x3*x3*y1 + x1*x1*y2 - x3*x3*y2 + y1*y1*y2 - y1*y2*y2 - x1*x1*y3 + 
		 x2*x2*y3 - y1*y1*y3 + y2*y2*y3 + y1*y3*y3 - y2*y3*y3;

	*yc=-(x1*x1*x2) + x1*x2*x2 + x1*x1*x3 - x2*x2*x3 - x1*x3*x3 + x2*x3*x3 - x2*y1*y1 + 
		 x3*y1*y1 + x1*y2*y2 - x3*y2*y2 - x1*y3*y3 + x2*y3*y3;

	*r=sqrt((x1*x1 - 2*x1*x2 + x2*x2 + (y1 - y2)*(y1 - y2))*
			(x1*x1 - 2*x1*x3 + x3*x3 + (y1 - y3)*(y1 - y3))*
	       		(x2*x2 - 2*x2*x3 + x3*x3 + (y2 - y3)*(y2 - y3)));

	 *xc /= denom;
	 *yc /= denom;
	 *r /= fabs(denom);

}

/** retourne un point sur l'ellipse dans la direction de x,y **/
/** la ligne relie x1,y1 et 0,0... donc il faut faire attention **/
static float kdist(float x1,float y1,float cx,float cy,float ax,float ay)
{
double denom,sq,prefix,ax2,ay2,x12,y12,cx2,cy2,k1,k2;
        ax2=ax*ax;
        ay2=ay*ay;
        x12=x1*x1;
        y12=y1*y1;
        cx2=cx*cx;
        cy2=cy*cy;
        denom = ay2*x12*y1 + ax2*y12*y1;
        sq = ax2*ay2* y12*(ay2*x12 - cy2*x12
                + 2*cx*cy*x1*y1 + (ax2 - cx2)*y12);
        prefix = ay2*cx*x1*y1 + ax2*cy*y12;
        if( sq<0 ) return(-1.0f);
        if( denom==0.0 ) return(-1.0f);
        k1=(prefix + sqrt(sq))/denom;
        k2=(prefix - sqrt(sq))/denom;
        //printf("c=(%f,%f) a=(%f,%f),xy=(%f,%f) k1=%f k2=%f\n",cx,cy,ax,ay,x1,y1,k1,k2);
        if( k1<0 && k2<0 ) return(-1.0f);
        if( k1>=0 ) return (float)(k1);
        return (float)(k2);
}


/*! returns the distance between the center xc,yc and the point on the line
*that passes by (x0,y0)-(xc,yc) and on the circle (xc1,yc1,r1)
*/
static float distance(float x0, float y0, float xc1, float yc1, float r1, float xc, float yc)
{
float a,b,c,d,e,k1,k2;

	a=(2*x0*xc - 2*xc*xc - 2*x0*xc1 + 2*xc*xc1 + 2*y0*yc - 2*yc*yc -  2*y0*yc1 + 2*yc*yc1);
	b=sqrt((x0 - xc)*(x0-xc) + (y0 - yc)*(y0-yc));
	c=sqrt( a*a - 4*(x0*x0 - 2*x0*xc + xc*xc + y0*y0 - 2*y0*yc + yc*yc)*
	    (-r1*r1 + xc*xc - 2*xc*xc1 + xc1*xc1 + yc*yc - 2*yc*yc1 + yc1*yc1) );
	d=(2*(x0*x0 - 2*x0*xc + xc*xc + y0*y0 - 2*y0*yc + yc*yc));
	e=(-2*x0*xc + 2*xc*xc + 2*x0*xc1 - 2*xc*xc1 - 2*y0*yc + 2*yc*yc + 2*y0*yc1 - 2*yc*yc1);


	k1=b*(e - c)/d;
	k2=b*(e + c)/d;

	if( k1<0 && k2<0 ) { printf("k1 k2 err neg!!!\n");return(0); }
	if( k1<0 ) return(k2);
	return(k1);

}

static double length(double p1[2],double p2[2])
{
    double len=sqrt((p1[0]-p2[0])*(p1[0]-p2[0])+(p1[1]-p2[1])*(p1[1]-p2[1]));

    return len;
}

static void bezier(uv_coord_struct *ctrls,double p,double pt_2d[2])
{
  double tm;

  tm=1-p;
  pt_2d[0]=tm*tm*tm*ctrls[0].x + 3*tm*tm*p*ctrls[1].x + 3*tm*p*p*ctrls[2].x + p*p*p*ctrls[3].x;
  pt_2d[1]=tm*tm*tm*ctrls[0].y + 3*tm*tm*p*ctrls[1].y + 3*tm*p*p*ctrls[2].y + p*p*p*ctrls[3].y;
}

static int initHomography()
{
  matrix *pts1,*pts2;

  pts1=NULL;
  pts2=NULL;

  matAllocate(&pts1,4,2);
  matAllocate(&pts2,4,2);

  pts1->values[0]=uv_coord[0].x;
  pts1->values[1]=uv_coord[0].y;
  pts1->values[2]=uv_coord[1].x;
  pts1->values[3]=uv_coord[1].y;
  pts1->values[4]=uv_coord[2].x;
  pts1->values[5]=uv_coord[2].y;
  pts1->values[6]=uv_coord[3].x;
  pts1->values[7]=uv_coord[3].y;

  pts2->values[0]=0.0;
  pts2->values[1]=0.0;
  pts2->values[2]=1.0;
  pts2->values[3]=0.0;
  pts2->values[4]=1.0;
  pts2->values[5]=1.0;
  pts2->values[6]=0.0;
  pts2->values[7]=1.0;

  matHomographySolve(pts1,pts2,&uv_homography);
  matInverse(&uv_homography_inv,uv_homography);

  matFree(&pts1);
  matFree(&pts2);

  return 0;
}

static int initBezierTriangles()
{
    int i;
    double li[2]; //consecutive 1D positions on bezier line
    double p[2][2]; //upper line 2D points
    double q[2][2]; //lower line 2D points
    double plen,qlen;
    double pdist,qdist;
    double pdist_prev,qdist_prev;

    plen=0;
    qlen=0;
    //compute pixel lengths of both lines for normalization
    for (i=0;i<NB_TRIANGLES/2;i++)
    {
      //define 2 consecutive points on each bezier line
      //this gives 4 points, thus 2 triangles
      li[0]=i/(double)(NB_TRIANGLES/2);
      li[1]=(i+1)/(double)(NB_TRIANGLES/2);
      bezier(&uv_coord[0],li[0],p[0]);
      bezier(&uv_coord[0],li[1],p[1]);
      bezier(&uv_coord[MAX_NB_UV_CTRLS],li[0],q[0]);
      bezier(&uv_coord[MAX_NB_UV_CTRLS],li[1],q[1]);
      plen+=length(p[0],p[1]);
      qlen+=length(q[0],q[1]);
    }

    pdist=0;
    qdist=0;
    pdist_prev=0;
    qdist_prev=0;
    for (i=0;i<NB_TRIANGLES/2;i++)
    {
      //define 2 consecutive points on each bezier line
      //this gives 4 points, thus 2 triangles
      li[0]=i/(double)(NB_TRIANGLES/2);
      li[1]=(i+1)/(double)(NB_TRIANGLES/2);
      bezier(&uv_coord[0],li[0],p[0]);
      bezier(&uv_coord[0],li[1],p[1]);
      bezier(&uv_coord[MAX_NB_UV_CTRLS],li[0],q[0]);
      bezier(&uv_coord[MAX_NB_UV_CTRLS],li[1],q[1]);
      pdist_prev=pdist;
      qdist_prev=qdist;
      pdist+=length(p[0],p[1]);
      qdist+=length(q[0],q[1]);

      //first triangle
#ifdef BEZIER_NORMALIZE
      uv_triangles[2*i].u[0]=pdist_prev/plen;
      uv_triangles[2*i].u[1]=pdist/plen;
      uv_triangles[2*i].u[2]=qdist_prev/qlen;
#else
      uv_triangles[2*i].u[0]=li[0];
      uv_triangles[2*i].u[1]=li[1];
      uv_triangles[2*i].u[2]=li[0];
#endif
      uv_triangles[2*i].v[0]=0.0;
      uv_triangles[2*i].v[1]=0.0;
      uv_triangles[2*i].v[2]=1.0;
      uv_triangles[2*i].x[0]=p[0][0];
      uv_triangles[2*i].x[1]=p[1][0];
      uv_triangles[2*i].x[2]=q[0][0];
      uv_triangles[2*i].y[0]=p[0][1];
      uv_triangles[2*i].y[1]=p[1][1];
      uv_triangles[2*i].y[2]=q[0][1];

      //second triangle
#ifdef BEZIER_NORMALIZE
      uv_triangles[2*i+1].u[0]=qdist/qlen;
      uv_triangles[2*i+1].u[1]=qdist_prev/qlen;
      uv_triangles[2*i+1].u[2]=pdist/plen;
#else
      uv_triangles[2*i+1].u[0]=li[1];
      uv_triangles[2*i+1].u[1]=li[0];
      uv_triangles[2*i+1].u[2]=li[1];
#endif
      uv_triangles[2*i+1].v[0]=1.0;
      uv_triangles[2*i+1].v[1]=1.0;
      uv_triangles[2*i+1].v[2]=0.0;
      uv_triangles[2*i+1].x[0]=q[1][0];
      uv_triangles[2*i+1].x[1]=q[0][0];
      uv_triangles[2*i+1].x[2]=p[1][0];
      uv_triangles[2*i+1].y[0]=q[1][1];
      uv_triangles[2*i+1].y[1]=q[0][1];
      uv_triangles[2*i+1].y[2]=p[1][1];
    }

    return 0;
}

int uvReadFile(char *uv_filename)
{
  FILE *uv_file;
  char uv_type_text[100];
  int i,v;
  int nb_col;

  uv_file = fopen(uv_filename, "rt");

  if (uv_file==NULL)
  {
    fprintf(stderr,"Crop file %s not found!\n",uv_filename);
    return -1;
  }

  //initialize crop control values values
  for(i=0; i<6; i++)
  {
    uv_coord[i].x=0.0;
    uv_coord[i].y=0.0;
  }
  uv_inner_scale=1.0;
  uv_outer_scale=1.0;

  fscanf(uv_file,"%s\n",uv_type_text);
  if(strcmp(uv_type_text,UV_DUAL_CIRCLES_STR) == 0)
  {
    uv_type = UV_DUAL_CIRCLES;
    for(i=0; i<3; i++)
    {
      fscanf(uv_file,"%d,%d\n", &uv_coord[i].x, &uv_coord[i].y);
    }
    for(i=0; i<3; i++)
    {
      fscanf(uv_file,"%d,%d\n", &uv_coord[MAX_NB_UV_CTRLS+i].x, &uv_coord[MAX_NB_UV_CTRLS+i].y);
    }
    fscanf(uv_file,"INNER_SCALE %lg\n",&uv_inner_scale);
    fscanf(uv_file,"OUTER_SCALE %lg\n",&uv_outer_scale);
    fscanf(uv_file,"VERTICAL_CENTER %lg\n",&uv_vcenter);
  }
  else if(strcmp(uv_type_text,UV_RECTANGLE_STR) == 0)
  {
    uv_type = UV_RECTANGLE;
    for(i=0; i<2; i++)
    {
      fscanf(uv_file,"%d,%d\n", &uv_coord[i].x, &uv_coord[i].y); 
    }
  }
  else if(strcmp(uv_type_text,UV_HOMOGRAPHY_STR) == 0)
  {
    uv_type = UV_HOMOGRAPHY;
    for(i=0;i<4;i++)
    {
      fscanf(uv_file,"%d,%d\n", &uv_coord[i].x, &uv_coord[i].y);
    }
    initHomography();
  }
  else if(strcmp(uv_type_text,UV_BEZIER_STR) == 0)
  {
    uv_type = UV_BEZIER;
    for(i=0; i<8; i++)
    {
      fscanf(uv_file,"%d,%d\n", &uv_coord[i].x, &uv_coord[i].y);
    }
    initBezierTriangles();
  }
  else
  {
    uv_type = NONE;
  }

  fclose(uv_file);

  return 0;
}

int uvSaveFile(char *uv_filename)
{
  FILE *uv_file;
  imgu *uvmap_color;
  int i;

  uvmap_color=NULL;

  uv_file = fopen(uv_filename, "wt");
  if (uv_file==NULL)
  {
    fprintf(stderr,"Error writing to crop file %s.\n",uv_filename);
    return -1;
  }
  
  if(uv_type==UV_DUAL_CIRCLES)
  {
    fprintf(uv_file,"%s\n",UV_DUAL_CIRCLES_STR);
    for(i=0;i<3;i++)
    {
      fprintf(uv_file,"%d,%d\n",uv_coord[i].x,uv_coord[i].y);
    }
    for(i=0;i<3;i++)
    {
      fprintf(uv_file,"%d,%d\n",uv_coord[MAX_NB_UV_CTRLS+i].x,uv_coord[MAX_NB_UV_CTRLS+i].y);
    }
    fprintf(uv_file,"INNER_SCALE %f\n",uv_inner_scale);
    fprintf(uv_file,"OUTER_SCALE %f\n",uv_outer_scale);
    fprintf(uv_file,"VERTICAL_CENTER %f\n",uv_vcenter);
  }
  else if(uv_type==UV_RECTANGLE)
  {
    fprintf(uv_file,"%s\n",UV_RECTANGLE_STR);
    for(i=0;i<2;i++)
    {
      fprintf(uv_file,"%d,%d\n",uv_coord[i].x,uv_coord[i].y);
    }
  }
  else if(uv_type==UV_HOMOGRAPHY)
  {
    fprintf(uv_file,"%s\n",UV_HOMOGRAPHY_STR);
    for(i=0;i<4;i++)
    {
      fprintf(uv_file,"%d,%d\n",uv_coord[i].x,uv_coord[i].y);
    }
  }
  else if(uv_type==UV_BEZIER)
  {
    fprintf(uv_file,"%s\n",UV_BEZIER_STR);
    for(i=0;i<8;i++)
    {
      fprintf(uv_file,"%d,%d\n",uv_coord[i].x,uv_coord[i].y);
    }
  }
  else
  {
    return -1;
  }

  fflush(uv_file);
  fclose(uv_file);

  return 0;
}

int uvLoadImg(char *uvmap_filename)
{
  imgu *uv_cmap;
  if (uvmap_filename==NULL) return -1;

  uv_cmap=NULL;
  imguLoad(&uv_cmap,uvmap_filename,LOAD_16_BITS);
  //imguConvertToComplexFromColor(&uv_uvmap,uv_cmap);
  imguConvertToComplexUV(&uv_map,uv_cmap);
  
  imguFree(&uv_cmap);

  return 0;
}

static int uvRect(double x,double y,double uv[2])
{
    double u,v;
    double xmin,ymin,xmax,ymax;

    xmin=uv_coord[0].x;
    ymin=uv_coord[0].y;
    xmax=uv_coord[1].x;
    ymax=uv_coord[1].y;

    u=(x-xmin)/(xmax-xmin);
    v=(y-ymin)/(ymax-ymin);
    if( u<0 || u>1.0 ) 
    {
        uv[0]=-1.0;
        uv[1]=-1.0;
    }
    else if( v<0 || v>1.0 ) 
    {
        uv[0]=-1.0;
        uv[1]=-1.0;
    }
    else
    {
        uv[0]=u;
        uv[1]=v;
    }

    return 0;
}

static int uvRectInverse(double u,double v,double xy[2])
{
    double x,y;
    double xmin,ymin,xmax,ymax;

    xmin=uv_coord[0].x;
    ymin=uv_coord[0].y;
    xmax=uv_coord[1].x;
    ymax=uv_coord[1].y;

    x=u*(xmax-xmin)+xmin;
    y=v*(ymax-ymin)+ymin;

    xy[0]=x;
    xy[1]=y;

    return 0;
}

int uvCenter(double xy[2])
{
    float xc1,yc1,r1;
    float xc2,yc2,r2;
    float x1,y1,x2,y2,x3,y3,x4,y4,x5,y5,x6,y6;

    x1=uv_coord[0].x;
    y1=uv_coord[0].y;
    x2=uv_coord[1].x;
    y2=uv_coord[1].y;
    x3=uv_coord[2].x;
    y3=uv_coord[2].y;
    x4=uv_coord[MAX_NB_UV_CTRLS].x;
    y4=uv_coord[MAX_NB_UV_CTRLS].y;
    x5=uv_coord[MAX_NB_UV_CTRLS+1].x;
    y5=uv_coord[MAX_NB_UV_CTRLS+1].y;
    x6=uv_coord[MAX_NB_UV_CTRLS+2].x;
    y6=uv_coord[MAX_NB_UV_CTRLS+2].y;

    //computes the centres and radius of circles
    uvEllipseCenter(x1,y1,x2,y2,x3,y3,&xc1,&yc1,&r1);
    uvEllipseCenter(x4,y4,x5,y5,x6,y6,&xc2,&yc2,&r2);

    xy[0]=(xc1+xc2)/2.0;
    xy[1]=(yc1+yc2)/2.0;

    return 0;
}

//small circle is (x1,y1,x2,y2,x3,y3)
//large circle is (x4,y4,x5,y5,x6,y6)
static int uvDualCircles(double x,double y,double uv[2])
{
    float xc1,yc1,r1;
    float xc2,yc2,r2;
    float xc0,yc0;
    float k1,k2,t,tref,u,v;
    double r,rspline;
    float u_scaled_in,u_scaled_out,u_scaled;
    float dx_in,dx_out;
    float x1,y1,x2,y2,x3,y3,x4,y4,x5,y5,x6,y6;
    matrix *Z,*h,*points;
    Z=NULL;
    h=NULL;
    points=NULL;

    matAllocate(&points,3,2);
    matSet(points,0,0,0.0);
    matSet(points,0,1,0.0);
    matSet(points,1,0,uv_vcenter);
    matSet(points,1,1,0.5);
    matSet(points,2,0,1.0);
    matSet(points,2,1,1.0);
    matFitCubicSpline(&Z,&h,points);

    x1=uv_coord[0].x;
    y1=uv_coord[0].y;
    x2=uv_coord[1].x;
    y2=uv_coord[1].y;
    x3=uv_coord[2].x;
    y3=uv_coord[2].y;
    x4=uv_coord[MAX_NB_UV_CTRLS].x;
    y4=uv_coord[MAX_NB_UV_CTRLS].y;
    x5=uv_coord[MAX_NB_UV_CTRLS+1].x;
    y5=uv_coord[MAX_NB_UV_CTRLS+1].y;
    x6=uv_coord[MAX_NB_UV_CTRLS+2].x;
    y6=uv_coord[MAX_NB_UV_CTRLS+2].y;

    //LT_PRINTF("dual circle: small circle (%f,%f)[ref]  (%f,%f) (%f,%f)\n",x1,y1,x2,y2,x3,y3);
    //LT_PRINTF("dual circle: large circle (%f,%f) (%f,%f) (%f,%f)\n",x4,y4,x5,y5,x6,y6);

    //computes the centres and radius of circles
    uvEllipseCenter(x1,y1,x2,y2,x3,y3,&xc1,&yc1,&r1);
    uvEllipseCenter(x4,y4,x5,y5,x6,y6,&xc2,&yc2,&r2);

    //LT_PRINTF("cercle 1 : (%f,%f) r=%f\n",xc1,yc1,r1);
    //LT_PRINTF("cercle 2 : (%f,%f) r=%f\n",xc2,yc2,r2);

    xc0=(xc1+xc2)/2.0;
    yc0=(yc1+yc2)/2.0;

    //angle 0 of reference is the first point of the outer circle (x6,y6)
    tref=atan2(y6-yc0,x6-xc0);
    //LT_PRINTF("xyc0=(%f,%f)  tref = %f deg\n",xc0,yc0,tref*180/M_PI);

    u=(double)x;
    v=(double)y;

    //la distance de x,y
    dx_in = (u-xc0) * uv_inner_scale;
    u_scaled_in = xc0 + dx_in;
    dx_out = (u-xc0) * uv_outer_scale;
    u_scaled_out = xc0 + dx_out;

    u_scaled = (u_scaled_in + u_scaled_out) / 2.0;

    // u,v est le point dans l'image
    // xc0,yc0 est le centre moyen
    // xc1,yc1 est le centre du 1er cercle (rayon r1)
    // xc2,yc2 est le centre du 2ieme cercle (rayon r2)
    k1=kdist(u-xc0,v-yc0,xc1-xc0,yc1-yc0,r1/uv_inner_scale,r1);
    k2=kdist(u-xc0,v-yc0,xc2-xc0,yc2-yc0,r2/uv_outer_scale,r2);
    if( k1<0 || k2<0 ) r=-1.0;
    else r=(1.0-k1)/(k2-k1);

    if( r<0 || r>1.0) 
    {
        uv[0]=-1.0;
        uv[1]=-1.0;
    }
    else
    {
        //angle around the circle
        t=atan2(v-yc0,u_scaled-xc0);
        if(t<tref) t+=2*M_PI;
        t=(t-tref)/(2*M_PI);

        t=1.0-t;

        //fprintf(stderr,"%f %f %f %f %f %f\n",u,v,xc0,yc0,u-xc0,v-yc0);
        //fprintf(stderr,"%f\n",t);
#ifdef RADIAL_CORRECTION
  		r=radial_correction(r);

        uv[0]=t;
        uv[1]=r;
#else
        if (matEvalCubicSpline(Z,h,points,r,&rspline)!=0)
        {
            uv[0]=-1.0;
            uv[1]=-1.0;
        }
        else
        {
            //give back the new coordinate
            uv[0]=t;
            uv[1]=rspline;
        }
#endif
    }

    matFree(&Z);
    matFree(&h);
    matFree(&points);

    return 0;
}

static int uvDualCirclesInverse(double u,double v,double xy[2])
{
    /*TODO*/
    xy[0]=0;
    xy[1]=0;

    return 0;
}

static int uvBezier(double x,double y,double uv[2])
{
    int i;
    double pixel[2],puv[2];
    
    pixel[0]=x;
    pixel[1]=y;

    uv[0]=-1.0;
    uv[1]=-1.0;

    for (i=0;i<NB_TRIANGLES;i++)
    {
        if (triMapPixel(&uv_triangles[i],pixel,puv)==0)
        {
            uv[0]=puv[0];
            uv[1]=puv[1];
            break;
        }
    }

    return 0;
}

static int uvBezierInverse(double u,double v,double xy[2])
{
    int i;
    double pixel[2],puv[2];
    
    puv[0]=u;
    puv[1]=v;

    xy[0]=-1.0;
    xy[1]=-1.0;

    for (i=0;i<NB_TRIANGLES;i++)
    {
        if (triMapUV(&uv_triangles[i],puv,pixel)==0)
        {
            xy[0]=pixel[0];
            xy[1]=pixel[1];
            break;
        }
    }

    return 0;
}

static int uvHomographyInverse(double u,double v,double xy[2])
{
    vector3 pixel,puv;  
    
    puv[0]=u;
    puv[1]=v;
    puv[2]=1.0;

    mat3MultiplyVector(uv_homography->values,puv,pixel);
    vect3Homogenize2D(pixel);

    xy[0]=pixel[0];
    xy[1]=pixel[1];

    return 0;
}

static int uvHomography(double x,double y,double uv[2])
{
    vector3 pixel,puv;
    
    pixel[0]=x;
    pixel[1]=y;
    pixel[2]=1.0;

    mat3MultiplyVector(uv_homography->values,pixel,puv);
    vect3Homogenize2D(puv);

    uv[0]=puv[0];
    uv[1]=puv[1];

    if( uv[0]<0.0 || uv[0]>1.0 || uv[1]<0.0 || uv[1]>1.0) 
    {
        uv[0]=-1.0;
        uv[1]=-1.0;
    }

    return 0;
}

int uvCrop(double x,double y,double uv[2])
{
  double range;
  //check using crop img first
  range=uvImgBilinear(x,y,uv);
  if (range>=0.0 && range<0.5) return 0; //check for uv-wrap around

  if( uv_type==UV_RECTANGLE ) 
  {
    if (uvRect(x,y,uv)) return -1;
  }
  else if( uv_type==UV_DUAL_CIRCLES ) 
  {
    if (uvDualCircles(x,y,uv)) return -1;
  } 
  else if( uv_type==UV_BEZIER ) 
  {
    if (uvBezier(x,y,uv)) return -1;
  } 
  else if( uv_type==UV_HOMOGRAPHY ) 
  {
    if (uvHomography(x,y,uv)) return -1;
  } 
  else
  {
    return -1; 
  }

  return 0;
}

int uvCropInverse(double u,double v,double xy[2])
{
  if( uv_type==UV_RECTANGLE ) 
  {
    uvRectInverse(u,v,xy);
  }
  else if( uv_type==UV_DUAL_CIRCLES ) 
  {
    uvDualCirclesInverse(u,v,xy);
  } 
  else if( uv_type==UV_BEZIER ) 
  {
    uvBezierInverse(u,v,xy);
  } 
  else if( uv_type==UV_HOMOGRAPHY ) 
  {
    uvHomographyInverse(u,v,xy);
  } 
  else
  {
    return -1; 
  }

  return 0;
}

//returns 1 if pixel is kept
int uvCheck(double x, double y)
{
  double uv[2];

  if (uvCrop(x,y,uv)) return 0;
  if (uv[0]<-0.5) return 0;
  
  return 1;
}

int uvCropImg(imgu *img)
{
  int x,y;
  int xs,ys;

  if (img==NULL || img->data==NULL) return -1;
  
  ys=img->ys;
  xs=img->xs;

  for(y=0;y<ys;y++) 
  {
    for(x=0;x<xs;x++) 
    {
      if (uvCheck((double)(x),(double)(y))==0) img->data[y*xs+x] = 0;
    }
  }

  return 0;
}

int uvComputeImg(char *uvmap_filename,int resx,int resy)
{
  int index,ibilinear[4];
  imgu *uv_cmap,*uv_tmp;
  double uv[2];
  int x,y,xs,ys;
  if (uvmap_filename==NULL) return -1;

  uv_cmap=NULL;
  uv_tmp=NULL;
  imguFree(&uv_map);

  if (imguAllocateComplex(&uv_tmp,resx,resy,1)) {imguFree(&uv_tmp);return -1;};

  ys=resy;
  xs=resx;

  index=0;
  for(y=0;y<ys;y++) 
  {
    for(x=0;x<xs;x++) 
    {
      if (uvCrop((double)(x),(double)(y),uv)==0)
      {
        uv_tmp->complex[index][0]=uv[0];
        uv_tmp->complex[index][1]=uv[1];
      }
      else
      {
        uv_tmp->complex[index][0]=-1.0;
        uv_tmp->complex[index][1]=-1.0;
      }
      index++;
    }
  }

  imguCopy(&uv_map,uv_tmp);
  imguFree(&uv_tmp);

  //check 4 pixels area to know if we can safely interpolate, otherwise mark as unsure (value > 1.0) and
  //real crop value will be computed upon subsequent request
  for(y=0;y<ys-1;y++) 
  {
    for(x=0;x<xs-1;x++) 
    {
      ibilinear[0]=y*xs+x;
      ibilinear[1]=y*xs+x+1;
      ibilinear[2]=y*xs+x;
      ibilinear[3]=(y+1)*xs+x+1;
      if (uv_map->complex[ibilinear[0]][0]<0 &&
          uv_map->complex[ibilinear[1]][0]<0 &&
          uv_map->complex[ibilinear[2]][0]<0 &&
          uv_map->complex[ibilinear[3]][0]<0) continue;
      if (uv_map->complex[ibilinear[0]][0]>=0 && uv_map->complex[ibilinear[0]][0]<=1.0 &&
          uv_map->complex[ibilinear[1]][0]>=0 && uv_map->complex[ibilinear[1]][0]<=1.0 &&
          uv_map->complex[ibilinear[2]][0]>=0 && uv_map->complex[ibilinear[2]][0]<=1.0 &&
          uv_map->complex[ibilinear[3]][0]>=0 && uv_map->complex[ibilinear[3]][0]<=1.0) continue;

      uv_map->complex[ibilinear[0]][0]=2.0;
      uv_map->complex[ibilinear[0]][1]=2.0;
    }
  }

  //imguConvertFromComplexToColor(&uv_cmap,uv_map);
  imguConvertFromComplexUV(&uv_cmap,uv_map,COMPLEX_RESCALE);
  imguSave(uv_cmap,uvmap_filename,FAST_COMPRESSION,SAVE_16_BITS);
  
  imguFree(&uv_cmap);

  return 0;
}

/////////////////////////////////
//these fonctions use the UV img
/////////////////////////////////
int uvImgClosest(double x,double y,double uv[2])
{
  int ix,iy;
  int xs,ys;
  vector4 cvals;

  if (uv_map==NULL) return -1;

  if (imguCheck(uv_map,x,y)!=0) return -1;

  imguInterpolateClosestComplex(uv_map,x,y,&(uv[0]),0);
  imguInterpolateClosestComplex(uv_map,x,y,&(uv[1]),1);
  
  return 0;
}

double uvImgBilinear(double x,double y,double uv[2])
{
  int i;
  int ix,iy;
  int xs,ys;
  vector4 cvals;
  double min,max;

  if (uv_map==NULL) return -1.0;

  if (imguCheck(uv_map,x,y)!=0 || imguCheck(uv_map,x+1.0,y+1.0)!=0) return -1.0;

  //the problem with bilinear interpolation is with the unwrap operation for the u wrap-around (dual circles configuration). See camera.c
  xs=uv_map->xs;
  ys=uv_map->ys;

  ix=(int)floor(x+0.0001);
  iy=(int)floor(y+0.0001);

  cvals[0]=uv_map->complex[iy*xs+ix][0];
  cvals[1]=uv_map->complex[iy*xs+ix+1][0];
  cvals[2]=uv_map->complex[(iy+1)*xs+ix][0];
  cvals[3]=uv_map->complex[(iy+1)*xs+ix+1][0];

  if (cvals[0]<-0.5 ||
      cvals[1]<-0.5 ||
      cvals[2]<-0.5 ||
      cvals[3]<-0.5) 
  {
    uv[0]=-1.0;
    uv[1]=-1.0;
    return -1.0;
  }
  else
  {
    min=max=cvals[0];
    for (i=1;i<4;i++)
    {
      if (min>cvals[i]) min=cvals[i];
      if (max<cvals[i]) max=cvals[i];
    }
    imguInterpolateBilinearComplex(uv_map,x,y,&(uv[0]),0);
    imguInterpolateBilinearComplex(uv_map,x,y,&(uv[1]),1);
    return max-min;
  }
}


