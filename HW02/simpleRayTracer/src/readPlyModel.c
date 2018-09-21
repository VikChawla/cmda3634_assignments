#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "simpleRayTracer.h"

void readPlyModel(const char *fileName, int *Ntriangles, triangle_t **triangles){

  FILE *fp = fopen(fileName, "r");

  char buf[BUFSIZ];

  do{
    fgets(buf, BUFSIZ, fp);
  }while(!strstr(buf, "element vertex"));
  
  int Nvertices;
  sscanf(buf, "element vertex %d", &Nvertices);


  do{
    fgets(buf, BUFSIZ, fp);
  }while(!strstr(buf, "element face"));

  sscanf(buf, "element face %d", Ntriangles);

  do{
    fgets(buf, BUFSIZ, fp);
  }while(!strstr(buf, "end_header"));

  dfloat *x = (dfloat*) calloc(Nvertices, sizeof(dfloat));
  dfloat *y = (dfloat*) calloc(Nvertices, sizeof(dfloat));
  dfloat *z = (dfloat*) calloc(Nvertices, sizeof(dfloat));

  dfloat xmin = 1e9, xmax = -1e9, ymin = 1e9, ymax = -1e9, zmin = 1e9, zmax = -1e9;
  
  printf("buf=%s\n", buf);
  for(int v=0;v<Nvertices;++v){
    fscanf(fp, dfloatString dfloatString dfloatString, x+v, y+v, z+v);
    fgets(buf, BUFSIZ, fp);
    xmin = min(xmin, x[v]);
    xmax = max(xmax, x[v]);
    ymin = min(ymin, y[v]);
    ymax = max(ymax, y[v]);
    zmin = min(zmin, z[v]);
    zmax = max(zmax, z[v]);
  }
  printf("buf=%s\n", buf);

  printf("min/max = %g,%g - %g,%g - %g,%g\n",
	 xmin,xmax, ymin,ymax, zmin,zmax);

  dfloat maxL = max(max(xmax-xmin, ymax-ymin), zmax-zmin);
  
  for(int v=0;v<Nvertices;++v){
    dfloat xv = (x[v]-xmin)/maxL;
    dfloat zv = (y[v]-ymin)/maxL;
    dfloat yv = (z[v]-zmin)/maxL;
    x[v] = BOXSIZE/4    + 0.5*BOXSIZE*(1-xv);
    z[v] = 3.*BOXSIZE/4.- 0.5*BOXSIZE*yv;
    y[v] = BOXSIZE      - 0.5*BOXSIZE*zv;
  }
  
  *triangles = (triangle_t*) calloc(*Ntriangles, sizeof(triangle_t));
  for(int e=0;e<*Ntriangles;++e){
    int v1, v2, v3;
    fscanf(fp, "%*d %d %d %d", &v1, &v2, &v3);
    //    printf("V=%d,%d,%d\n", v1, v2, v3);
    triangles[0][e].vertices[0] = vectorCreate(x[v1], y[v1], z[v1]);
    triangles[0][e].vertices[1] = vectorCreate(x[v2], y[v2], z[v2]);
    triangles[0][e].vertices[2] = vectorCreate(x[v3], y[v3], z[v3]);

#if 0
    printf("t %d: (%g,%g,%g), (%g,%g,%g), (%g,%g,%g)\n",
	   e,
	   triangles[0][e].vertices[0].x, triangles[0][e].vertices[0].y, triangles[0][e].vertices[0].z,
	   triangles[0][e].vertices[1].x, triangles[0][e].vertices[1].y, triangles[0][e].vertices[1].z,
	   triangles[0][e].vertices[2].x, triangles[0][e].vertices[2].y, triangles[0][e].vertices[2].z);
#endif
  }

  printf("Read %d triangles and %d vertices from %s\n", *Ntriangles, Nvertices, fileName); 
  
  free(x); free(y); free(z);
}
