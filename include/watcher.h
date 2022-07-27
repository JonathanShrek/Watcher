#define MAX_EVENTS 1024									//Max. number of events to process at one go
#define LEN_NAME 1024									//Assuming length of the filename won't exceed 16 bytes
#define EVENT_SIZE (sizeof(struct inotify_event)) 		//size of one event
#define BUF_LEN (MAX_EVENTS * (EVENT_SIZE + LEN_NAME))	//buffer to store the data of events
#define COMMAND_BUF 1024 								//buffer to store the system command data
#define LOGTIME_BUF 30 									//buffer to store the log datetime

// Prototypes
void get_event(int fd, const char *watchedDir, const char *destinationDir, FILE *log);
void copy_dir(const char *dir, const char *name, const char *destinationDir, FILE *log);
