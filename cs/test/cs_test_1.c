#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <dcc.h>

#define TEST_NAME	    "cs_test_1"
#define MAX_COMMAND_LEN 100

static void usage(void);
extern char *optarg;

int
main(int argc, char **argv)
{
	int option=0, address=0, direction=0, step=0, fd, command_len=0;
	char *dev=NULL, command[MAX_COMMAND_LEN];

	if(argc == 1)
		usage();

	while((option = getopt(argc, argv, "D:d:s:a:")) != -1)
	{
		switch(option)
		{
			case 'd':
				direction = atoi(optarg);
				break;
				
			case 's':
				step = atoi(optarg);
				break;

			case 'a':
				address = atoi(optarg);
				break;
			
			case 'D':
				dev = strdup(optarg);
				break;
			
			default:
				usage();
		}
	}

	/* Print diagnostic info. */
	printf("Constructing DCC command:\n");
	printf("\tAddress\t\t=> %d\n", address);
	printf("\tDirection\t=> %s\n", 
		direction == DCC_DIRECTION_FORWARD ? "forward" : "reverse");
	printf("\tSpeed step\t=> %d\n\n", step);

    if(step > 0)
    {
        if(direction)
        {
            /* forward command */
            command_len = snprintf(command, MAX_COMMAND_LEN,
                "forward addr %d speed %d;\n", address, step); 
        }
        else
        {
            /* reverse command */
            command_len = snprintf(command, MAX_COMMAND_LEN,
                "reverse addr %d speed %d;\n", address, step); 
        }
    }
    else
    {
        /* stop command */
        command_len = snprintf(command, MAX_COMMAND_LEN,
            "stop addr %d;\n", address); 
    }

	/* Send the command to the command station. */
	fd = open(dev, O_RDWR);
	if(fd < 0)
	{	
		printf("Could not open device: %s\n", dev);
		exit(EXIT_FAILURE);
	}

	write(fd, (const void*) command, (command_len - 1));
	close(fd);

	printf("%d bytes written to %s ...\n\n", (command_len - 1), dev);

	return 0;
}

static void
usage()
{
	fprintf(stderr, 
		"%s\n\t-a ADDRESS\n\t-d DIRECTION\n\t-s SPEED\n\t-D DEVICE\n", 
		TEST_NAME);

	exit(EXIT_FAILURE);
}
