/*
	volren_seq.cpp
	$Id: volren_seq.cpp,v 1.3 2003/05/09 16:07:11 prok Exp $

	$Log: volren_seq.cpp,v $
	Revision 1.3  2003/05/09 16:07:11  prok
	bug hunting continues... currently tracking down a difficult bug in data_dist
	that is indirectly causing volren_par to fail on the SRS00002 dataset.
	
	Revision 1.2  2003/05/07 23:43:41  prok
	commented out some debugging specific timing code, added a render timer to the
	sequential code.
	
	Revision 1.1  2003/05/04 23:18:05  prok
	Cleanup day! Several preemptive changes made before testing begins:
	- interfe.cpp got some changes related to outputting view params so that output
	from the renderer more closely matches OpenGL output
	- cachmanager.cpp lrumemcache.* and volren_par.cpp got some code to assist in
	exiting cleanly at the end of the render
	- extra attention was paid to asynchronous I/O voodoo in lrumemcache.*.
	Specifically, I/O now happens in two threads, requests get stuffed into a queue
	before being handled, requests stay in a list until they are received. All told,
	I think this code will actually work with the way it is being called now.(lots
	of calls to FetchBlock() were a vector for ugliness)
	- fixed a tiny bug in pvolume.cpp where a ray's state could change even if it
	didn't actually complete a sample.
	- moved sequential rendering out of interfe.cpp and into volren_seq.cpp.
	
*/

#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <mpi.h>
#include "veclib.h"
#include "ray.h"
#include "color.h"
#include "camera.h"
#include "volume.h"
#include "trace.h"
#include "timer.h"

void usage(char *app)
{
	printf("usage: %s -h | [-f <rawiv filename> -t <vinay file> -v <view params file>]\n",
				app);
}

void load_view_params(char *file, Vector4 & eye, Vector4 & lookat, Vector4 & up)
{
	FILE *fp;
	float vec[4];

	fp = fopen(file, "r");
	if (fp == NULL)
	{
		printf("could not open view params file %s\n", file);
		exit(-1);
	}

	// eye
	for (int i=0; i<4; i++)
		fscanf(fp, "%f", &vec[i]);
	eye.Set(vec[0],vec[1],vec[2],vec[3]);
	// lookat
	for (int i=0; i<4; i++)
		fscanf(fp, "%f", &vec[i]);
	lookat.Set(vec[0],vec[1],vec[2],vec[3]);
	// up
	for (int i=0; i<4; i++)
		fscanf(fp, "%f", &vec[i]);
	up.Set(vec[0],vec[1],vec[2],vec[3]);
	
	fclose(fp);
}

int main(int argc, char **argv)
{
	// cmdline parsing stuff
	struct option long_options[] = {
		{"help", no_argument, 0, 'h'},
		{"file", required_argument, 0, 'f'},
		{"transfer", required_argument, 0, 't'},
		{"view", required_argument, 0, 'v'},
    { 0, 0, 0, 0}
	};
  char *short_options = "f:t:v:h", c;
	int indx=0;
	// cmdline variables
	char *dataset=NULL, *tfunc=NULL, *viewparam=NULL;
	// other variables
	int width = 500, height = 500;
	unsigned char *frameBuf;
	Vector4 eye,lookat,up;
	Volume vol;
	Trace *raycast;
	Camera *cam;
	Timer *renderTime;

	// init MPI (for timing only)
	MPI_Init(&argc, &argv);

	// parse the command line args
	while ((c = getopt_long(argc, argv, short_options, long_options, &indx)) != - 1)
	{
		switch (c)
		{
        case 'f':
            dataset = optarg;
            break;
				case 't':
						tfunc = optarg;
						break;
				case 'v':
						viewparam = optarg;
						break;
				case 'h':
						usage(argv[0]);
            exit(0);
						break;
        case '?':
				default:
            break;
		}
	}
	
	// bail out if our options aren't initialized
	if (dataset == NULL || tfunc == NULL || viewparam == NULL)
	{
		usage(argv[0]);
		exit(-1);
	}

	// init the timer
	renderTime = new Timer("render time");
	
	// start the timer
	renderTime->Start();

	// init the volume
	vol.LoadTransferFunction(tfunc);
	vol.LoadDataset(dataset);

	// load the view parameters
	load_view_params(viewparam, eye, lookat, up);

	printf("eye: "); eye.Print();
	printf("lookat: "); lookat.Print();
	printf("up: "); up.Print();
	// construct the camera
	cam = new Camera(eye,lookat,up, 60.0,width/(double)height,
									(double)width,(double)height);

	// construct the ray caster
	raycast = new Trace(&vol);

	// allocate the frame buffer
	frameBuf = (unsigned char *)malloc(width*height*3);

	// go!
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			color_t c = raycast->RayCast(cam->RayForPixel(x,y));

			frameBuf[(y*width+x)*3+0] = (unsigned char)(c.r*255.0);
			frameBuf[(y*width+x)*3+1] = (unsigned char)(c.g*255.0);
			frameBuf[(y*width+x)*3+2] = (unsigned char)(c.b*255.0);
		}
		printf("."); fflush(stdout);
	}
	printf("\n");

	// stop the timer
	renderTime->Stop();

	// write the output file
	FILE *fp = fopen("render.ppm", "w");
	if (fp != NULL)
	{
		fprintf(fp, "P6\n%d %d\n255\n", width, height);
		fwrite(frameBuf, width*height*3, 1, fp);
		fclose(fp);
	}
	else
		printf("could not open file render.ppm for writing!\n");

	// print the time taken to render
	renderTime->Print();

	// clean up allocated memory
	free(frameBuf);
	delete raycast;
	delete cam;
	delete renderTime;

	MPI_Finalize();
	
	return 0;
}

