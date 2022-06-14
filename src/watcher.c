#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <fcntl.h>
#include <time.h>

#define MAX_EVENTS 1024 /*Max. number of events to process at one go*/
#define LEN_NAME 1024 /*Assuming length of the filename won't exceed 16 bytes*/
#define EVENT_SIZE  ( sizeof (struct inotify_event) ) /*size of one event*/
#define BUF_LEN     ( MAX_EVENTS * ( EVENT_SIZE + LEN_NAME )) /*buffer to store the data of events*/
#define COMMAND_BUF 1024 /*buffer to store the system command data*/
#define LOGTIME_BUF 30 /*buffer to store the log datetime*/

// Prototypes
void get_event(int fd, const char * watchedDir, const char * destinationDir, FILE *log);
void copy_dir(const char * dir, const char * name, const char * destinationDir, FILE *log);

int main(int argc, char* argv[]) {
	FILE *log;
	int fd;
	int wd;
	char logTime[LOGTIME_BUF];

	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	snprintf(logTime, sizeof(logTime), "%d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

	log = fopen("/tmp/watcher.log", "a+"); // a+ (create + append) option

	if (log == NULL) {
		puts("Failed to create the log file\n");
	}
	
	if (argc != 3) {
		fprintf(log, "%s - Failed to provide the correct amount of arguments\n", logTime);
		return EXIT_FAILURE;
	}

	fd = inotify_init();

	if (fd < 0) {
		perror("inotify_init");
	}

	wd = inotify_add_watch(fd, argv[1], IN_MODIFY | IN_CREATE | IN_DELETE);

	if (wd == -1) {
		fprintf(log, "%s - Couldn't add watch to %s\n", logTime, argv[1]);
	} else {
		fprintf(log, "%s - Watching: %s\n\n", logTime, argv[1]);
	}
	
	fclose(log); // Closes the log stream

	// Do it forever
	while(true) {
		get_event(fd, argv[1], argv[2], log);
	}

	// Clean up
	inotify_rm_watch(fd, wd);
	close(fd);

	return EXIT_SUCCESS;
}

void get_event (int fd, const char * watchedDir, const char * destinationDir, FILE *log) {
	char buffer[BUF_LEN];
	int length, i = 0;
	char logTime[LOGTIME_BUF];

	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	snprintf(logTime, sizeof(logTime), "%d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	  
	if (log == NULL) {
		puts("Failed to attach to the existing log file.");
	}
		 
	length = read( fd, buffer, BUF_LEN );  
	if ( length < 0 ) {
		perror( "read" );
	}  
			  
	while ( i < length ) {
		struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];

		if ( event->len ) {
			if ( event->mask & IN_CREATE) {
				log = fopen("/tmp/watcher.log", "a+"); // a+ (create + append) option

				if (event->mask & IN_ISDIR) {
					fprintf(log, "%s - The directory %s was created.\n", logTime, event->name);
				}
				else {
					fprintf(log, "%s - The file %s was created.\n", logTime, event->name);
				}

				copy_dir(watchedDir, event->name, destinationDir, log);

				fclose(log); // Closes the log stream
			}
									            
			i += EVENT_SIZE + event->len;
		}
	}
}

void copy_dir(const char * watchedDir, const char * newFileName, const char * destinationDir, FILE *log) {
	char buf[COMMAND_BUF];
	int systemResult;
	char logTime[LOGTIME_BUF];

	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	snprintf(logTime, sizeof(logTime), "%d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

	fprintf(log, "%s - Copying %s to the data folder.\n", logTime, newFileName);

	// Creates the system command and stores it in the buffer
	snprintf(buf, sizeof(buf), "cp -r %s/%s %s", watchedDir, newFileName, destinationDir);

	systemResult = system(buf);
	
	if (systemResult != -1) {
		fprintf(log, "%s - System command executed successfully.\n\n", logTime);
	} else {
		fprintf(log, "%s - System command failed.\n\n", logTime);
	}
}
