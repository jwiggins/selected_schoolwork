#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

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
	printf("usage:%s -i <infile> -o <outfile> -d <x dim>x<y dim>x<z dim>",
								 progname);
	printf(" -x <x spacing> -y <y spacing> -z <z spacing>\n");
}

// assumes little endian is the native order!!!
void swap_little_to_big_uint(unsigned int &val, char *buffer);
void swap_little_to_big_float(float &val, char *buffer);
void swap_buffer(char *buffer, int size, int type);
void create_rawiv_header(char *buffer, float *min, float *max, unsigned int numverts,
						unsigned int numcells, unsigned int *dim, float *origin, float *span);

int main(int argc, char **argv)
{
	static struct option long_options[] = {
				{"help", no_argument, 0, 'h'},
				{"infile", required_argument, 0, 'i'},
				{"outfile", required_argument, 0, 'o'},
				{"dim", required_argument, 0, 'd'},
				{"xspan", required_argument, 0, 'x'},
				{"yspan", required_argument, 0, 'y'},
				{"zspan", required_argument, 0, 'z'},
        { 0, 0, 0, 0}
	};
  char *short_options = "i:o:d:x:y:z:h";
	
	char header[68],readbuf[512], *infile = NULL, *outfile = NULL, c;
	float min[3], max[3], origin[3], span[3];
	unsigned int numverts, numcells, type=1, dim[3];
	int indx;
	FILE *in, *out;

	min[0] = min[1] = min[2] = 0.0;
	origin[0] = origin[1] = origin[2] = 0.0;

	span[0] = span[1] = span[2] = 0.0;
	dim[0] = dim[1] = dim[2] = 0;

	// parse the command line args
	while ((c = getopt_long(argc, argv, short_options, long_options, &indx)) != - 1)
	{
		switch (c)
		{
				case 'h':
						usage(argv[0]);
            exit(0);
						break;
				case 'i':
						printf("i = %s\n", optarg);
						infile = optarg;
						break;
				case 'o':
						printf("o = %s\n", optarg);
						outfile = optarg;
						break;
				case 'd':
						printf("d = %s\n", optarg);
						sscanf(optarg, "%dx%dx%d", dim, dim+1, dim+2);
						printf("dim =  (%d, %d, %d)\n", dim[0], dim[1], dim[2]);
						break;
				case 'x':
						printf("x = %s\n", optarg);
						span[0] = atof(optarg);
						break;
				case 'y':
						printf("y = %s\n", optarg);
						span[1] = atof(optarg);
						break;
				case 'z':
						printf("z = %s\n", optarg);
						span[2] = atof(optarg);
						break;
        case '?':
				default:
            break;
		}
	}
	
	// bail out if our options aren't initialized
	if (infile == NULL || outfile == NULL
		|| dim[0] == 0 || dim[1] == 0 || dim[2] == 0
		|| span[0] == 0.0 || span[1] == 0.0 || span[2] == 0.0)
	{
		usage(argv[0]);
		exit(-1);
	}

	// compute the max
	max[0] = dim[0] * span[0];
	max[1] = dim[1] * span[1];
	max[2] = dim[2] * span[2];
	// compute numverts and numcells
	numverts = dim[0]*dim[1]*dim[2];
	numcells = (dim[0]-1)*(dim[1]-1)*(dim[2]-1);

	printf("creating header\n");

	// create the header
	create_rawiv_header(header, min, max, numverts, numcells, dim, origin, span);

	printf("opening files\n");
	
	// open files
	in = fopen(infile, "r");
	out = fopen(outfile, "w");
	if (in == NULL || out == NULL)
	{
		if (!in)
			printf("could not open infile %s\n", infile);
		if (!out)
			printf("could not open outfile %s\n", outfile);
		exit(-1);
	}

	printf("writing header\n");

	// write the header
	fwrite(header, sizeof(header), 1, out);

	printf("writing file\n");

	// write the file
	while (!feof(in))
	{
		int size = fread(readbuf, 1, 512, in);

		fwrite(readbuf, size, 1, out);
	}

	// close files
	fclose(in);
	fclose(out);
	
	return 0;
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

