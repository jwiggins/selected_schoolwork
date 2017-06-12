/*
	data_dist.cpp
	$Id: data_dist.cpp,v 1.5 2003/05/10 19:41:16 prok Exp $

	$Log: data_dist.cpp,v $
	Revision 1.5  2003/05/10 19:41:16  prok
	This is the final commit. The project is done. This is the tree that will be
	submitted to Prof. Browne.
	
	Revision 1.4  2003/05/09 17:19:55  prok
	Fixed the min/max density calculation in data_dist. The SRS00002 dataset should render correctly now.
	
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <mpi.h>

#include "timer.h"
#include "myrandom.h"

union f_swap {
	float value;
	char bytes[4];
};

union ui_swap {
	unsigned int value;
	char bytes[4];
};

void usage(char *progname)
{
	printf("usage:%s -r <cache directory> -f <filename> -c <# of cache nodes>\n",
									progname);
}

// assumes little endian is the native order!!!
void swap_big_to_little_uint(unsigned int &val, char *buffer);
void swap_little_to_big_uint(unsigned int &val, char *buffer);
void swap_big_to_little_float(float &val, char *buffer);
void swap_little_to_big_float(float &val, char *buffer);
void swap_buffer(char *buffer, int size, int type);

void distribute_cubes(FILE *fp, char *cachedir, char *filename, int cachenodes,
								unsigned int type, unsigned int dim[3], unsigned int cubedim,
								int xb, int yb, int zb, float *min, float *max);
void write_info_file(char *cachedir, char *file, unsigned int dim[3],
								int xb, int yb, int zb, int type, float min[3], float max[3],
							 	float span[3], unsigned int cubedim, float minD, float maxD);
void parse_rawiv_header(char *buffer, float *min, float *max, unsigned int *numverts,
						unsigned int *numcells, unsigned int *dim, float *origin, float *span);
void create_rawiv_header(char *buffer, float *min, float *max, unsigned int numverts,
						unsigned int numcells, unsigned int *dim, float *origin, float *span);
int read_cube_row(char ***buffers, FILE *fp, unsigned int type, unsigned int *datadim,
				 unsigned int cubedim, unsigned int ox, unsigned int oy, unsigned int oz,
				 float *min, float *max);
void adjust_min_and_max(char *buffer, int elems, int type,
												float *min, float *max);

// globals!!!
Timer *diskTime, *netTime;

int main(int argc, char **argv)
{
	static struct option long_options[] = {
				{"help", no_argument, 0, 'h'},
				{"file", required_argument, 0, 'f'},
				{"rootdir", required_argument, 0, 'r'},
				{"cachenum", required_argument, 0, 'c'},
        { 0, 0, 0, 0}
	};
  char *short_options = "f:r:c:h";
	
	char header[68], **cubes, *datadir = NULL, *filename = NULL, fname[255], c;
	float min[3], max[3], origin[3], span[3];
	unsigned int numverts, numcells, numcubes, type=1, dim[3], cubedim=10, oy,oz;
	int rank, size, cachenodes=0, xb,yb,zb, indx;
	long datasize;
	float minD=1000000.0, maxD=-1000000.0;
	FILE *fp;

	// init MPI
	MPI_Init(&argc, &argv);

	// query rank and size
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	// parse the command line args
	while ((c = getopt_long(argc, argv, short_options, long_options, &indx)) != - 1)
	{
		switch (c)
		{
        case 'f':
            filename = optarg;
            break;
        case 'r':
            datadir = optarg;
            break;
        case 'c':
            cachenodes = atoi(optarg);
            break;
				case 'h':
            if (rank == 0)
							usage(argv[0]);
            exit(0);
						break;
        case '?':
            break;
		}
	}
	
	// bail out if our options aren't initialized
	if (filename == NULL || datadir == NULL || cachenodes == 0)
	{
		if (rank == 0)
			usage(argv[0]);
		exit(-1);
	}

	// open the dataset
	if (rank == 0)
	{
		fp = fopen(filename, "r");
		if (fp == NULL)
		{
			printf("[%d] could not open file %s\n", rank, filename);
			exit(-1);
		}
	}

	// init timers
	diskTime = new Timer("disk read time");
	netTime = new Timer("network transmission time");

	if (rank == 0)
	{
		// seek to the end of the file
		fseek(fp, 0L, SEEK_END);
		// get the size of the data (filesize less 68 byte header)
		datasize = ftell(fp) - 68;
		// seek back to the beginning
		fseek(fp, 0L, SEEK_SET);
	
		// read the rawiv header
		fread(header, sizeof(header), 1, fp);
	}

	// broadcast the datasize
	MPI_Bcast(&datasize, 1, MPI_INT, 0, MPI_COMM_WORLD);
	// broadcast the header
	MPI_Bcast(header, 68, MPI_CHAR, 0, MPI_COMM_WORLD);

	// parse the header
	parse_rawiv_header(header, min,max,&numverts,&numcells,dim,origin,span);
	// determine the datatype of the voxels
	type = (datasize / numverts);

	if (rank == 0)
	{
		printf("datasize = %d bytes (%fMB)\n", datasize, datasize / (1024.0*1024.0));
		printf("data type = %s\n", (type==1)?"char":((type==2)?"short":"float"));
		printf("min (%f,%f,%f)\n", min[0], min[1], min[2]);
		printf("max (%f,%f,%f)\n", max[0], max[1], max[2]);
		printf("numverts = %d\n", numverts);
		printf("numcells = %d\n", numcells);
		printf("dim (%d,%d,%d)\n", dim[0], dim[1], dim[2]);
		printf("origin (%f,%f,%f)\n", origin[0], origin[1], origin[2]);
		printf("span (%f,%f,%f)\n", span[0], span[1], span[2]);
	}

	// compute the dimensions of the dataset in blocks
	xb = dim[0] / cubedim;
	xb += (dim[0] % cubedim) ? 1 : 0; // extra cube that hangs over edge
	yb = dim[1] / cubedim;
	yb += (dim[1] % cubedim) ? 1 : 0; // extra cube that hangs over edge
	zb = dim[2] / cubedim;
	zb += (dim[2] % cubedim) ? 1 : 0; // extra cube that hangs over edge

	// init the min and max density vars
	switch(type)
	{
		case 1:
			minD = 255.0;
			maxD = 0.0;
			break;
		case 2:
			minD = 65535.0;
			maxD = 0.0;
			break;
		case 4:
		default:
			break;
	}
	
	// distribute the blocks among the cache processors
	distribute_cubes(fp, datadir, filename, cachenodes,
								 type, dim, cubedim, xb,yb,zb, &minD, &maxD);

	// write the info file on all processors
	write_info_file(datadir, filename, dim,xb,yb,zb,type,
									min,max,span, cubedim,minD,maxD);

	// print statistics
	if (rank == 0)
	{
		diskTime->Print();
		netTime->Print();
	}
	
	// cleanup
	if (rank == 0)
		fclose(fp);
	delete diskTime;
	delete netTime;

	// cleanup MPI
	MPI_Finalize();

	return 0;
}

void swap_big_to_little_uint(unsigned int &val, char *buffer)
{
	ui_swap uis;

	// swap the order (little -> big)
	uis.bytes[0] = buffer[3];
	uis.bytes[1] = buffer[2];
	uis.bytes[2] = buffer[1];
	uis.bytes[3] = buffer[0];
	// assign the value
	val = uis.value;
}

void swap_little_to_big_uint(unsigned int &val, char *buffer)
{
	ui_swap uis;

	// assign the value
	uis.value = val;
	// swap the order (little -> big)
	buffer[3] = uis.bytes[0];
	buffer[2] = uis.bytes[1];
	buffer[1] = uis.bytes[2];
	buffer[0] = uis.bytes[3];
}

void swap_big_to_little_float(float &val, char *buffer)
{
	f_swap fs;

	// swap the order (little -> big)
	fs.bytes[0] = buffer[3];
	fs.bytes[1] = buffer[2];
	fs.bytes[2] = buffer[1];
	fs.bytes[3] = buffer[0];
	// assign the value
	val = fs.value;
}

void swap_little_to_big_float(float &val, char *buffer)
{
	f_swap fs;

	// assign the value
	fs.value = val;
	// swap the order (little -> big)
	buffer[3] = fs.bytes[0];
	buffer[2] = fs.bytes[1];
	buffer[1] = fs.bytes[2];
	buffer[0] = fs.bytes[3];
}

void swap_buffer(char *buffer, int size, int type)
{
	// swapping isn't necessary on single byte data
	if (type == 1)
		return;
	
	char sbuf[4];

	for (int i=0; i < size/type; i++)
	{
		memcpy(sbuf, buffer+(i*type), type);
		
		switch (type)
		{
			case 2:
			{
				buffer[i*type] = sbuf[1];
				buffer[i*type+1] = sbuf[0];
				break;
			}
			case 4:
			{
				buffer[i*type] = sbuf[3];
				buffer[i*type+1] = sbuf[2];
				buffer[i*type+2] = sbuf[1];
				buffer[i*type+3] = sbuf[0];
				break;
			}
			default:
				break;
		}
	}
}

void distribute_cubes(FILE *fp, char *cachedir, char *filename, int cachenodes,
							unsigned int type, unsigned int dim[3], unsigned int cubedim,
							int xb, int yb, int zb, float *min, float *max)
{
	FILE *out = NULL, *inds;
	char fname[255], *basename, *cube, **cubes;
	unsigned char blk_rec;
	int oy, oz, rank, size, seed;
	MPI_Status status;

	
	// get some MPI data
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	// strip leading characters off of the filename to get the basename
	basename = strrchr(filename, '/');
	if (basename == NULL)
		basename = filename;
	else
		basename++;
	
	// only open an output blockfile if we are one of the cache nodes
	if (rank < cachenodes)
	{
		// generate the output filename
		sprintf(fname, "%s/%s_%03d.blks\0", cachedir, basename, rank);
		// open output file
		out = fopen(fname, "w");
	}
	
	// generate the block ids filename
	sprintf(fname, "%s/%s.ids\0", cachedir, basename, rank);
	// open output file
	inds = fopen(fname, "w");

	// seed the random number generator
	if (rank == 0)
		seed = time(NULL);
	// broadcast the seed
	MPI_Bcast(&seed, 1, MPI_INT, 0, MPI_COMM_WORLD);
	// give it to the rng
	mysrandom(seed);
	
	// allocate a cube buffer on cache nodes
	if (rank != 0 && rank < cachenodes)
	{
		// allocate space for a cube
		cube = (char *)malloc(cubedim*cubedim*cubedim*type);
	}

	for (int i=0; i < zb; i++)
	{
		for (int j=0; j < yb; j++)
		{
			// set an origin
			oy = j*cubedim;
			oz = i*cubedim;

			//printf("reading row at (0, %d, %d)\n", oy, oz);

			// extract a row of subvolumes
			if (rank == 0)
			{
				// read the row
				read_cube_row(&cubes, fp, type, dim, cubedim, 0, oy, oz, min,max);
			}
			
			// distribute the cubes
			for (int n=0; n < xb; n++)
			{
				// build a block record
				//record.id = (i*xb*yb)+(j*xb)+n;
				blk_rec = (unsigned char)myrandom(0, cachenodes-1);
				// write it to the id file
				fwrite(&blk_rec, sizeof(unsigned char), 1, inds);
				
				//printf("blockid %d -> processor %d\n", blockid, blockid%size);

				// send from root
				if (rank == 0)
				{
					// don't try to send to yourself
					if (rank != (int)blk_rec)
					{
						netTime->Start();

						MPI_Send(cubes[n], cubedim*cubedim*cubedim*type, 
  	        	   MPI_CHAR, (int)blk_rec, 0, MPI_COMM_WORLD);

						netTime->Stop();
					}
					else // this cube belongs to us
					{
						// write the cube to disk
						//printf("writing data\n");
						fwrite(cubes[n], cubedim*cubedim*cubedim*type, 1, out);
					}
				}
				else if (rank == (int)blk_rec)
				{
					MPI_Recv(cube, cubedim*cubedim*cubedim*type, 
          	   MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);
					// write the cube to disk
					//printf("writing data\n");
					fwrite(cube, cubedim*cubedim*cubedim*type, 1, out);
				}
			}

			if (rank == 0)
			{
				// free memory used by cube row
				for (int n=0; n < xb; n++)
					free(cubes[n]);
				free(cubes);

				// print a status dot
				printf("."); fflush(stdout);
			}
		}
	}

	// close the files
	if (rank < cachenodes)
		fclose(out);
	fclose(inds);
	// free the cube buffer
	if (rank != 0 && rank < cachenodes)
		free(cube);

	// put an endline after all those dots 
	if (rank == 0)
		printf("\n"); fflush(stdout);

	// make all the non-cache nodes wait
	MPI_Barrier(MPI_COMM_WORLD);
	// distribute min and max values to everybody
	MPI_Bcast(min, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
	MPI_Bcast(max, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
}

void write_info_file(char *cachedir, char *file, unsigned int dim[3],
							 	int xb, int yb, int zb, int type, float min[3],
								float max[3], float span[3], unsigned int cubedim,
								float minD, float maxD)
{
	FILE *fp;
	char fname[255], *basename;

	// strip leading characters off of the filename to get the basename
	basename = strrchr(file, '/');
	if (basename == NULL)
		basename = file;
	else
		basename++;
	
	// generate the filename
	sprintf(fname, "%s/%s.info\0", cachedir, basename);
	// open info file
	fp = fopen(fname, "w");
	
	// write the info file
	// data dimensions
	fprintf(fp, "%d %d %d\n", dim[0], dim[1], dim[2]);
	// block dimensions
	fprintf(fp, "%d %d %d\n", xb, yb, zb);
	// data type
	fprintf(fp, "%d\n", type);
	// minimum coordinate (origin)
	fprintf(fp, "%f %f %f\n", min[0], min[1], min[2]);
	// maximum coordinate
	fprintf(fp, "%f %f %f\n", max[0], max[1], max[2]);
	// cell span
	fprintf(fp, "%f %f %f\n", span[0], span[1], span[2]);
	// subvolume cube dimension
	fprintf(fp, "%d\n", cubedim);
	// minimum "density"
	fprintf(fp, "%f\n", minD);
	// maximum "density"
	fprintf(fp, "%f\n", maxD);
	
	// close the info file
	fclose(fp);
}

void parse_rawiv_header(char *buffer,float *min, float *max, unsigned int *numverts,
								unsigned int *numcells, unsigned int *dim, float *origin, float *span)
{
	// read minX
	swap_big_to_little_float(min[0], buffer);
	buffer += 4;
	// read minY
	swap_big_to_little_float(min[1], buffer);
	buffer += 4;
	// read minZ
	swap_big_to_little_float(min[2], buffer);
	buffer += 4;

	// read maxX
	swap_big_to_little_float(max[0], buffer);
	buffer += 4;
	// read maxY
	swap_big_to_little_float(max[1], buffer);
	buffer += 4;
	// read maxZ
	swap_big_to_little_float(max[2], buffer);
	buffer += 4;

	// read numverts 
	swap_big_to_little_uint(*numverts, buffer);
	buffer += 4;

	// read numcells 
	swap_big_to_little_uint(*numcells, buffer);
	buffer += 4;

	// read dimX
	swap_big_to_little_uint(dim[0], buffer);
	buffer += 4;
	// read dimY
	swap_big_to_little_uint(dim[1], buffer);
	buffer += 4;
	// read dimZ
	swap_big_to_little_uint(dim[2], buffer);
	buffer += 4;

	// read originX
	swap_big_to_little_float(origin[0], buffer);
	buffer += 4;
	// read originY
	swap_big_to_little_float(origin[1], buffer);
	buffer += 4;
	// read originZ
	swap_big_to_little_float(origin[2], buffer);
	buffer += 4;

	// read spanX
	swap_big_to_little_float(span[0], buffer);
	buffer += 4;
	// read spanY
	swap_big_to_little_float(span[1], buffer);
	buffer += 4;
	// read spanZ 
	swap_big_to_little_float(span[2], buffer);
}

void create_rawiv_header(char *buffer, float *min, float *max, unsigned int numverts,
								unsigned int numcells, unsigned int *dim, float *origin, float *span)
{
	f_swap fs;
	ui_swap uis;

	//printf("creating rawiv header\n");
	
	// minX
	swap_little_to_big_float(min[0], buffer);
	buffer += 4;
	// minY
	swap_little_to_big_float(min[1], buffer);
	buffer += 4;
	// minZ
	swap_little_to_big_float(min[2], buffer);
	buffer += 4;

	// maxX
	swap_little_to_big_float(max[0], buffer);
	buffer += 4;
	// maxY
	swap_little_to_big_float(max[1], buffer);
	buffer += 4;
	// maxZ
	swap_little_to_big_float(max[2], buffer);
	buffer += 4;

	// numverts 
	swap_little_to_big_uint(numverts, buffer);
	buffer += 4;

	// numcells 
	swap_little_to_big_uint(numverts, buffer);
	buffer += 4;

	// dimX
	swap_little_to_big_uint(dim[0], buffer);
	buffer += 4;
	// dimY
	swap_little_to_big_uint(dim[1], buffer);
	buffer += 4;
	// dimZ
	swap_little_to_big_uint(dim[2], buffer);
	buffer += 4;

	// originX
	swap_little_to_big_float(origin[0], buffer);
	buffer += 4;
	// originY
	swap_little_to_big_float(origin[1], buffer);
	buffer += 4;
	// originZ
	swap_little_to_big_float(origin[2], buffer);
	buffer += 4;

	// spanX
	swap_little_to_big_float(span[0], buffer);
	buffer += 4;
	// spanY
	swap_little_to_big_float(span[1], buffer);
	buffer += 4;
	// spanZ 
	swap_little_to_big_float(span[2], buffer);
}

int read_cube_row(char ***buffers, FILE *fp, unsigned int type, unsigned int *datadim,
							 	unsigned int cubedim, unsigned int ox, unsigned int oy, unsigned int oz,
								float *min, float *max)
{
	char *buffer, **bptrs, *rowbuf;
	unsigned int offset, numbufs;
	int xXtra, yXtra, zXtra;	
	
	// how many buffers are needed?
	numbufs = datadim[0] / cubedim;
	numbufs += (datadim[0] % cubedim) ? 1 : 0; // extra cube that hangs over edge
	
	// allocate lists of pointers to buffers
	*buffers = (char **)malloc(numbufs*sizeof(char *));
	bptrs = (char **)malloc(numbufs*sizeof(char*));
	
	// allocate the buffers
	for (int i=0; i < numbufs; i++)
	{
		bptrs[i] = (*buffers)[i] = (char *)malloc(cubedim*cubedim*cubedim*type);
		memset(bptrs[i], 0, cubedim*cubedim*cubedim*type);
	}

	// allocate memory for a row's worth of data
	rowbuf = (char *)malloc(datadim[0]*type);

	// check for dataset boundaries within row 
	xXtra = datadim[0] % cubedim; // x dimension is a special case
	yXtra = (oy+cubedim) - datadim[1];
	yXtra = (yXtra < 0) ? 0 : yXtra;
	zXtra = (oz+cubedim) - datadim[2];
	zXtra = (zXtra < 0) ? 0 : zXtra;
	
	//printf("extras: (%d,%d,%d)\n", xXtra, yXtra, zXtra);
	
	// read the data
	for (int z=0; z < (cubedim-zXtra); z++)
	{
		for (int y=0; y < (cubedim-yXtra); y++)
		{
			// seek to the row start
			offset = 68 // header
					+ ((oz+z)*datadim[0]*datadim[1]*type) // slices in z direction
					+ ((oy+y)*datadim[0]*type) // rows in y direction
					+ (ox*type); // columns in x direction
			fseek(fp, offset, SEEK_SET);

			//printf("read row\n");
			
			// read a row
			diskTime->Start();
			fread(rowbuf,datadim[0]*type, 1, fp);
			diskTime->Stop();

			// swap the data
			swap_buffer(rowbuf, datadim[0]*type, type);

			// adjust min and max values
			adjust_min_and_max(rowbuf, datadim[0], type, min, max);
			
			// write it into the cubes
			for(int i=0; i<numbufs; i++)
			{
				if (i < numbufs-1)
				{
					// copy part of the row to the cube's buffer
					memcpy(bptrs[i], rowbuf+(i*cubedim*type), cubedim*type);
					// increment the cube's buffer pointer
					bptrs[i] += cubedim*type;
				}
				else
				{
					// is the last cube completely within the dataset?
					if (xXtra != 0)
					{
						// copy part of the row to the cube's buffer
						memcpy(bptrs[i], rowbuf+(i*cubedim*type), (cubedim-xXtra)*type);
						// zero the part outside the dataset
						memset(bptrs[i]+((cubedim-xXtra)*type), 0, xXtra*type);
						// increment the cube's buffer pointer
						bptrs[i] += cubedim*type;
					}
					else // normal copy operation
					{
						// copy part of the row to the cube's buffer
						memcpy(bptrs[i], rowbuf+(i*cubedim*type), cubedim*type);
						// increment the cube's buffer pointer
						bptrs[i] += cubedim*type;
					}
				}
			}
		}
		// pad with zero'd rows when needed
		for (int y=(cubedim-yXtra); y < cubedim; y++)
		{
			for (int i=0; i<numbufs; i++)
			{
				// zero the row
				memset(bptrs[i],0,cubedim*type);
				// increment the buffer's pointer
				bptrs[i] += cubedim*type;
			}
		}	
	}
	// pad with zero'd slices when needed
	for (int z=(cubedim-zXtra); z < cubedim; z++)
	{
		for (int i=0; i<numbufs; i++)
		{
			// zero the slice
			memset(bptrs[i],0,cubedim*cubedim*type);
			// increment the buffer's pointer
			bptrs[i] += cubedim*cubedim*type;
		}
	}

	for (int i=0; i < numbufs; i++)
	{
		if ((bptrs[i]-(*buffers)[i]) != cubedim*cubedim*cubedim*type) 
		printf("bptr - buffer = %d. (should be %d)\n",bptrs[i]-(*buffers)[i],
									cubedim*cubedim*cubedim*type);
	}

	// clean up
	free(bptrs);
	free(rowbuf);
	
	return numbufs;
}

// check min and max values
void adjust_min_and_max(char *buffer, int elems, int type,
												float *min, float *max)
{
	unsigned char *cptr = (unsigned char *)buffer;
	unsigned short *sptr = (unsigned short *)buffer;
	float *fptr = (float *)buffer;

	switch(type)
	{
		case 1:
		{
			for (int x=0; x < elems; x++)
			{
				if (*min > (float)cptr[x])
					*min = (float)cptr[x];
				if (*max < (float)cptr[x])
					*max = (float)cptr[x];
			}
			break;
		}
		case 2:
		{
			for (int x=0; x < elems; x++)
			{
				if (*min > (float)sptr[x])
					*min = (float)sptr[x];
				if (*max < (float)sptr[x])
					*max = (float)sptr[x];
			}
			break;
		}
		case 4:
		{
			for (int x=0; x < elems; x++)
			{
				if (*min > fptr[x])
					*min = fptr[x];
				if (*max < fptr[x])
					*max = fptr[x];
			}
			break;
		}
		default:
			break;
	}
}

