/*
	Name: Nick Hodge(Hodge1nr)		Class: CPS470
	Section: 22377311			Assignment: 05
	Due: 4/6/20				Points: 10
	
	Description: Copies given files and builds destination
		path. File(s) and destination are specified on execution. 
		Command line arguements must be checked for correct input.
		The files at the paths given are copied 
		to the destination folder only if they are
		regular files. The files are copied to the destination using threads. 
		The result and all errors are printed to stderr. 

	Solution: Command line arguements are checked at the 
		start of the program. If they pass the check
		then the files and destination are passed
		to the copyfiles function. Copyfiles() calls 
		docopy() with a thread holding a struct with the 
		information about the file to copy. The copy is 
		performed and copyfiles() prints the result. 
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <sys/wait.h>

typedef struct pstruct {
	char *srcpath;
	char *dstpath;
	char *name;
}pstruct;

int main(int argc, char *argv[]) {
	
	int check;
	int cpcheck;	
	int chkdst(int, char **argv);
	int copyfiles(int, char **argv);	
		
	check =	chkdst(argc, argv);

	setbuf(stdout, NULL);

	if (check == 0) {
		exit(1);
	}
	else {
		if (check == 1) {
			cpcheck = copyfiles(argc, argv);
		}
	
		if (cpcheck == 0) {
			fprintf(stderr, "Error: Copy failed");
		}
	}

	exit(0);
}


/*	Checks command line arguements for the correct number
	of args and the last arg passed (the destination directory)
	by calling isdir().
	Returns 1 on success and 0 otherwise.
	Prints usage error messages to stderr.
*/
int chkdst(int argc, char **argv) {
	
	int test;	
	int isdir(char *path);
	char *last;
		
	last = argv[argc-1];
		
	if (argc < 3) {		
		fprintf(stderr, "Error: Not enough arguements\n");
		return 0;
	}
	else {
		test = isdir(last);
		if (test != 1 || !argv[argc-1]) {
			fprintf(stderr, "Error: Check dest arguement\n");
			return 0;
		}
	}
	return 1;
}


/*	Checks if the file is a directory using stat() call.
	Returns 1 if the file is a directory and 0 otherwise.
*/
int isdir(char *path) {
		
	int status;
	struct stat statbuff;

	status = stat(path, &statbuff);
	if (status == 0) {
		if (S_ISDIR(statbuff.st_mode)) {
			return 1;
		}
		else {	
			return 0;
		}
	}
	else {
		return 0;
	}
}


/*	Checks if the file is a regular file using stat() call.
	Returns 1 if the file is regular and 0 otherwise.
*/
int isregular(char *path) {

	int status;
	struct stat statbuff;

	status = stat(path, &statbuff);
	if (status == 0) {
		if (S_ISREG(statbuff.st_mode)) {	
			return 1;
		}
		else {
			return 0;
		}
	}
	else {
		return 0;
	}
}


/*	Copies the inputted files to destination directory.
	For regular files the destination file path is built 
	and the file is copied to the destination. 
	The struct holding the path information for the file to be copied
	is sent to docopy() using threads.
	Returns 1 on success and 0 on failure 
*/
int copyfiles(int x, char **argv) {

	int count;
	int destchk;
	int dir;
	int i;
	int reg;
	int retval;
	char *name;
	char *srcpath;
	char *destpath;
	char *path;
	FILE *file;
	pthread_t threadlist[x];
	pstruct *paths[x];	

	char *buildpath(char *src, char *dst);
	void *docopy(void *paths);
	int isdir(char *path);
	int isregular(char *path);

	destpath = argv[x - 1];
	count = 0;
	i = 1;	
	printf("\n");

	while (i < (x - 1)) {
		destchk = 0;
		srcpath = argv[i];
		name = strrchr(srcpath, '/');
		if (name == NULL) {
			name = srcpath;
		}
		reg = isregular(srcpath);
		dir = isdir(srcpath);
		if (reg == 0 && dir == 0) {
			printf("%-20s: No such file or directory\n", name);	
		}
		else if (dir == 1) {
			printf("%-20s: Is a directory\n", name);
		} 
		else if (reg == 0) {		
			printf("%-20s: Not a regular file\n", name);	
		}
		else if (reg == 1 && dir == 0) {
			path = buildpath(srcpath, destpath);
			file = fopen(path, "r");
			if (file) {
				printf("%-20s: Already in destination\n", name);
				destchk = 1;
				retval = 1;
			}
			if (destchk == 0) {
				paths[i] = malloc(sizeof(pstruct));
				paths[i]->srcpath = malloc(sizeof(srcpath));
				paths[i]->dstpath = malloc(500);
				paths[i]->name = malloc(sizeof(500));
				strcpy(paths[i]->name, name);
				strcpy(paths[i]->srcpath, srcpath);
				strcpy(paths[i]->dstpath, path);
				
				pthread_create(&threadlist[i], NULL, &docopy, paths[i]);
				count++;
				retval = 1;
			}	
		}
		i++;
	}
	sleep(1);
	printf("\n");
	printf("%d%s", count, " files copied successfully\n");

	return retval;
}


/*	Wrapper function that receives the struct of paths from
	copyfiles() with a thread. The file paths are sent to cp1file()
	so that the file can be copied. 

	Race condition: A race condition is possible because if a thread
	executes at the same time as another one they will enter the critical
	section at the same time.  
	This could be solved by using a form of mutex lock to vary the time
	that each thread enters the critical section. 
*/
void *docopy(void *paths) {

	int cp1file(const char *srcpath, const char *dstpath);	
	pstruct *data;

	data = (pstruct*)paths;
	
	cp1file(data->srcpath, data->dstpath);	
	
	printf("%-20s: Copied by thread %lu\n", data->name, pthread_self());	
	
	free (data);
	return NULL;
}


/*	Copies the given file to destination file descriptor.
	Returns 1 on success and 0 otherwise.
*/
int cp1file(const char *srcpath, const char *dstpath) {

	int fdin, fdout, retval;
	char buf[2048];
	int nread, nwrote;	
	
	fdin = open(srcpath, O_RDONLY);	
	fdout = open(dstpath, O_CREAT | O_WRONLY, 0644);

	while (1) {
		nread = read(fdin, buf, 2048);
		if (nread == -1) {
			retval = 0;
			break;
		} 
		if (nread == 0) {
			break;
		}
		nwrote = write(fdout, buf, (size_t)nread);
		if (nwrote == -1) {
			retval = 0;
			break;
		}
		if (nread > 0 && nwrote > 0) {
			retval = 1;
			break;
		}
	}
	close(fdin);
	close(fdout);	
	
	return retval;
}


/*	Builds the destination path from the source and
	the destination strings. Dynamiclly allocates memory
	for destination path.	
*/
char *buildpath(char *src, char *dst) {

	char *ptr, d[32], *dest;
	
	size_t n = 128;	

	ptr = malloc(n);
	dest = malloc(n);
	if (!ptr || !dest) {
		fprintf(stderr, "malloc: failed..\n");
	}	

	ptr = strrchr(src, '/');
	if (ptr == NULL) {
		ptr = src;
		strcpy(d, dst);
		strcat(d, "/");
		strcat(d, ptr);
		strcpy(dest, d);
		return dest;
	}
	else {
		strcpy(d, dst);
		strcat(d, ptr);
		strcpy(dest, d);
		return dest;
	}		
}
