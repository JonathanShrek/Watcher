#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <fcntl.h>
#include <time.h>
#include "watcher.h"

int main(int argc, char* argv[]) {
	FILE *log;
	int fd;
	int wd;
	char logTime[LOGTIME_BUF];

	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	snprintf(logTime, LOGTIME_BUF, "%d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

	log = fopen("/tmp/watcher.log", "a+"); // a+ (create + append) option

	if (log == NULL) {
		puts("Failed to create the log file\n");
	}
	
	if (argc != 3) {
		fprintf(log, "%s - Failed to provide the correct amount of arguments\n", logTime);
		return EXIT_FAILURE;
	}
	
	//initialize a new inotify instance
	fd = inotify_init();

	if (fd < 0) {
		perror("inotify_init");
	}

	//add a watch to an initialized inotify instance
	wd = inotify_add_watch(fd, argv[1], IN_MODIFY | IN_CREATE | IN_DELETE);

	if (wd < 0) {
		fprintf(log, "%s - Couldn't add watch to %s\n", logTime, argv[1]);
	} else {
		fprintf(log, "%s - Watching: %s\n\n", logTime, argv[1]);
	}
	
	fclose(log); // Closes the log stream

	// Do it forever
	while(true) {
		get_event(fd, argv[1], argv[2], log);
		sleep(1);	//added sleep(1) for throttle
	}

	// Clean up
	inotify_rm_watch(fd, wd);
	close(fd);

	return EXIT_SUCCESS;
}

void get_event (int fd, const char *watchedDir, const char *destinationDir, FILE *log) {
	char buffer[BUF_LEN];
	int length, i = 0;
	char logTime[LOGTIME_BUF];

	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	snprintf(logTime, sizeof(logTime), "%d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	  
	if (log == NULL) {
		puts("Failed to attach to the existing log file.");
	}
		 
	length = read(fd, buffer, BUF_LEN);  
	if (length < 0) {
		perror("read");
	}  
			  
	while (i < length) {
		struct inotify_event *event = (struct inotify_event *) &buffer[i];

		if (event->len) {
			if (event->mask & IN_CREATE) {
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

void copy_dir(const char *watchedDir, const char *newFileName, const char *destinationDir, FILE *log) {
	char buf[COMMAND_BUF];
	int systemResult;
	char logTime[LOGTIME_BUF];

	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	snprintf(logTime, sizeof(logTime), "%d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

	fprintf(log, "%s - Copying %s to the data folder.\n", logTime, newFileName);

	// Creates the system command and stores it in the buffer
	snprintf(buf, COMMAND_BUF, "cp -r %s/%s %s", watchedDir, newFileName, destinationDir);

	systemResult = system(buf);
	
	if (systemResult != -1) {
		fprintf(log, "%s - System command executed successfully.\n\n", logTime);
	} else {
		fprintf(log, "%s - System command failed.\n\n", logTime);
	}
}
