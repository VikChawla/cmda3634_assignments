
#include "simpleRayTracer.h"

// to compile:
// gcc -O3 -o simpleRayTracer *.c -I.  -fopenmp -lm

// to run:
//  ./simpleRayTracer

// to compile animation:
//   ffmpeg -y -i image_%05d.ppm -pix_fmt yuv420p foo.mp4

int main(int argc, char *argv[])
{
   MPI_Init(&argc, &argv);

   
    
  int rank = 0;
  int size = 1;
  
  initTimer();
  
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_rank(MPI_COMM_WORLD, &size);
  
  // initialize triangles and spheres
  scene_t *scene = sceneSetup();

  grid_t     *grid      = scene->grid;
  shape_t    *shapes    = scene->shapes;
  material_t *materials = scene->materials;
  light_t    *lights    = scene->lights;
  
  /* Will contain the raw image */
  unsigned char *img = (unsigned char*) calloc(3*WIDTH*HEIGHT, sizeof(char));

  // 1. location of observer eye (before rotation)
  sensor_t sensor;

  // background color
  sensor.bg.red   = 126./256;
  sensor.bg.green = 192./256;
  sensor.bg.blue  = 238./256;

  dfloat br = 3.75f*BOXSIZE;

  // angle elevation to y-z plane
  dfloat eyeAngle = M_PI/4.f; // 0 is above, pi/2 is from side.  M_PI/3; 0; M_PI/2.;

  // target view
  vector_t targetX = vectorCreate(BOXSIZE/2., BOXSIZE, BOXSIZE/2.); // this I do not understand why target -B/2
  sensor.eyeX = vectorAdd(targetX, vectorCreate(0, -br*cos(eyeAngle), -br*sin(eyeAngle))); 
  dfloat sensorAngle = eyeAngle; +15.*M_PI/180.;
  sensor.Idir   = vectorCreate(1.f, 0.f, 0.f);
  sensor.Jdir   = vectorCreate(0.f, sin(sensorAngle), -cos(sensorAngle));
  vector_t sensorNormal = vectorCrossProduct(sensor.Idir, sensor.Jdir);
  
  // 2.4 length of sensor in axis 1 & 2
  sensor.Ilength = 20.f;
  sensor.Jlength = HEIGHT*20.f/WIDTH;
  sensor.offset  = 0.f;

  // 2.5 normal distance from sensor to focal plane
  dfloat lensOffset = 50;
  sensor.lensC = vectorAdd(sensor.eyeX, vectorScale(lensOffset, vectorCrossProduct(sensor.Idir, sensor.Jdir)));

  // why 0.25 ?
  sensor.focalPlaneOffset = 0.22f*fabs(vectorTripleProduct(sensor.Idir, sensor.Jdir, vectorSub(targetX,sensor.eyeX))); // triple product
  
  //  sensor.focalOffset = 0.8*BOXSIZE - sensor.lensC.z; // needs to be distance to plane from sensor

  printf("lensOffset = %g, sensor.focalPlaneOffset = %g\n", lensOffset, sensor.focalPlaneOffset);
  
  dfloat *randomNumbers = (dfloat*) calloc(2*NRANDOM, sizeof(dfloat));
  for(int i=0;i<NRANDOM;++i){
    dfloat r1 = 2*drand48()-1;
    dfloat r2 = 2*drand48()-1;

    randomNumbers[2*i+0] = r1/sqrt(r1*r1+r2*r2);
    randomNumbers[2*i+1] = r2/sqrt(r1*r1+r2*r2);
  }
  
  // number of angles to render at
  int Ntheta = 240;
  
  // loop over scene angles
  for(int thetaId=0;thetaId<Ntheta;++thetaId){
    
    /* rotation angle in y-z */
    dfloat theta = thetaId*M_PI*2./(dfloat)(Ntheta-1);

    /* sort objects into grid */
    gridPopulate(grid, scene->Nshapes, shapes);

    /* start timer */
    if (rank == 0)
      ticTimer();
    
    /* render scene */
    renderKernel(WIDTH,
		 HEIGHT,
		 scene[0],
		 sensor,
		 cos(theta), 
		 sin(theta),
		 randomNumbers,
		 img);
     
   	unsigned char* out = (unsigned char*) calloc(3*WIDTH*(HEIGHT/size), sizeof(unsigned char));

   
    unsigned char* in = (unsigned char*) calloc(3*WIDTH*(HEIGHT/size), sizeof(unsigned char));

	 

      if(rank != 0)
      
      {
	int messageTag = 999;

	

    
	for(int x= 0; x< 3*WIDTH*(HEIGHT/size); x++)
		  {
		    out[x] = img[rank *(3*WIDTH*(HEIGHT/size))  + x];
		 
		    MPI_Send(out, 3*WIDTH*(HEIGHT/size), MPI_UNSIGNED_CHAR, 0, messageTag, MPI_COMM_WORLD);

              }
}

  if(rank == 0)
	    {
	      MPI_Status status;

	      //  int messageSource;
	      int messageTag = 999;
	      for(int j = 1; j<size; ++j)
		{
		  
		  MPI_Recv(in, 3*WIDTH*(HEIGHT/size), MPI_UNSIGNED_CHAR, j, messageTag, MPI_COMM_WORLD, &status);

		  for(int i = 0; i < 3*WIDTH*(HEIGHT/size); ++i)
		      {
			 
			img[j*(3*WIDTH*(HEIGHT/size)) + i] = in[j];
		
		      }
		}

	   
      }
         
    /* report elapsed time */
    if (rank == 0) 
      tocTimer("recursiveRenderKernel");
    
    dfloat dt = .025, g = 1;
    int NsubSteps= 40;

    if (rank == 0)
      ticTimer();
    
    // collide and move spheres in time and update grid
    for(int subStep=0;subStep<NsubSteps;++subStep){
      
      sphereCollisions(grid, dt, g, scene->Nshapes, shapes);

      sphereUpdates(grid, dt, g, scene->Nshapes, shapes);

      gridPopulate(grid, scene->Nshapes, shapes);
    }

    // report time taken to move and collide spheres
    if (rank == 0)
      tocTimer("move and collide Spheres");

    /* save scene as ppm file */
    char fileName[BUFSIZ];

    // make sure images directory exists
    mkdir("images", S_IRUSR | S_IREAD | S_IWUSR | S_IWRITE | S_IXUSR | S_IEXEC);

    // write image as ppm format file
    
    /*Q6: modifications to the file generation should go here
    *
    *
    */
    
    sprintf(fileName, "images/image_%05d_rk%d.ppm", thetaId,rank);
    saveppm(fileName, img, WIDTH, HEIGHT);

  }
  
  free(img);
   MPI_Finalize();
  return 0;
 
}
