// based on https://www.kernel.org/doc/html/v4.12/input/uinput.html
#define DEBUG 0
#define VERBOSE 0

#include <stdio.h>	// snprintf
#include <stdlib.h>	// exit
#include <dirent.h>	// walk /dev/input
#include <string.h>	// memset
#include <unistd.h>	// daemon, read, write
#include <fcntl.h>	// constants in open
#include <libgen.h>	// kill basename
#include <signal.h>	// interupt handling
//#include <errno.h>
//#include <linux/input.h>
#include <linux/uinput.h>	// constants uinput

#define size_of_array(a) ( sizeof(&a) / sizeof(a[0]) )

#define die(str, args...) do { \
        perror(str); \
        exit(EXIT_FAILURE); \
    } while(0)

volatile int interrupted = 0;

void handle_int(int num) {
	printf("INTERRUPT %d", num); fflush(stdout);
  interrupted = 1;
}

void setup_device(int fdo)
{
   struct uinput_setup usetup;
   if (ioctl(fdo, UI_SET_EVBIT, EV_KEY) < 0) die("error: ioctl");
   if (ioctl(fdo, UI_SET_KEYBIT, BTN_LEFT) < 0) die("error: ioctl");

   if (ioctl(fdo, UI_SET_EVBIT, EV_REL) < 0) die("error: ioctl");
   if (ioctl(fdo, UI_SET_RELBIT, REL_X) < 0) die("error: ioctl");
   if (ioctl(fdo, UI_SET_RELBIT, REL_Y) < 0) die("error: ioctl");

	 if (ioctl(fdo, UI_SET_EVBIT, EV_KEY) < 0) die("error: ioctl announce keys");
	 for(unsigned long i = 0; i < KEY_REFRESH_RATE_TOGGLE; i++)	// dont ask, does not work with KEY_MAX somehow and I dont feel like diving down the rabbit hole
		 if(ioctl(fdo, UI_SET_KEYBIT, i) < 0) die("error: ioctl announce keys");

   memset(&usetup, 0, sizeof(usetup));
   usetup.id.bustype = BUS_USB;
   usetup.id.vendor = 0x1234; // vendor
   usetup.id.product = 0x5678; // product
   strcpy(usetup.name, "uinput-proxy");

   if (ioctl(fdo, UI_DEV_SETUP, &usetup) < 0) die("error: ioctl");
   if (ioctl(fdo, UI_DEV_CREATE) < 0) die("error: ioctl");
}

int emit(int fd, int type, int code, int val)
{
   struct input_event ie;

   ie.type = type;
   ie.code = code;
   ie.value = val;
   /* timestamp values below are ignored */
   ie.time.tv_sec = 0;
   ie.time.tv_usec = 0;

   return write(fd, &ie, sizeof(ie)) < 0;
}

void move_mouse(int fdo, int x, int y)
{
	if (x) emit(fdo, EV_REL, REL_X, x);
	if (y) emit(fdo, EV_REL, REL_Y, y);
}

void key_combination(int fdo, int key_count, int key_codes[])
{
	int i=0;
	for(i=0; i<key_count; i++) {
		if (VERBOSE) printf("combination key %d, key %x\n", i, key_codes[i]);
		emit(fdo, EV_KEY, key_codes[i], 1);
		emit(fdo, EV_SYN, SYN_REPORT, 0);
	}
	for(i=0; i<key_count-1; i++) {
		emit(fdo, EV_KEY, key_codes[i], 0);
		emit(fdo, EV_SYN, SYN_REPORT, 0);
	}
	emit(fdo, EV_KEY, key_codes[i], 0);
	// do not send SYN_REPORT since it is already sent after the intercepted key
}

int handle_systemd(int argc, char* argv[])
{
	if (argc == 2 && (
				!strcmp(argv[1], "stop")
				|| !strcmp(argv[1], "restart") 
				|| !strcmp(argv[1], "reload") 
				)) {
		// or use kill with pid https://linux.die.net/man/3/kill
		char command[255] = "pkill -O1 ";
		strcat(command, basename(argv[0]));
		printf("command: %s\n", command); fflush(stdout);
		system(command);
		if (!strcmp(argv[1], "stop")) {
				exit(0);
		}
	}
	if (argc == 2 && (
				!strcmp(argv[1], "start")
				|| !strcmp(argv[1], "restart") 
				|| !strcmp(argv[1], "reload") 
				)) {
		// fake as if started without parameters
		argc = 1;
		argv[1] = "";
	}
	if (argc == 2 && !strcmp(argv[1], "status")) {
    char result[2];
    char command[1024];
		sprintf(command, "ps -C %s -o cmd|grep -vP '^CMD$| status$'", basename(argv[0]));
		if (VERBOSE) { printf("command: %s\n", command); fflush(stdout); }

		FILE *cmd = popen(command, "r");
		if(fgets(result, sizeof(result), cmd)) {
			printf("Running...\n");
		} else {
			printf("Not running\n");
    }
		pclose(cmd);
		exit(1);
	}
	return argc;
}

char* get_input_device(char* path, char* needle)
{
	struct dirent *ep;
	DIR *dp = opendir(path);
	char* fullpath = malloc(255);
	strcpy(fullpath, path);
	if (dp != NULL) {
		while (ep = readdir(dp)) {
			if (!strncmp(ep->d_name, needle, strlen(needle))) {
				strcat(fullpath, ep->d_name);
				break;
			}
		}
		(void)closedir(dp);
	}
	return fullpath;
}

int main(int argc, char* argv[])
{
    int                    fdo, fdi;
    struct input_event     ev;
		char* input_device = NULL;

		struct sigaction int_handler = {.sa_handler=handle_int};
		sigaction(SIGINT,&int_handler,0);
		sigaction(SIGTERM, &int_handler,0);

		argc = handle_systemd(argc, argv);

		if (argc == 2) {
			input_device = argv[1];
		} else {
			input_device = get_input_device("/dev/input/by-path/", "platform-ir-receiver");
			if (!input_device) {
				die("error: specify input device, i.e. /dev/input/by-path/platform-ir-receiver@11-event");
			}
		}

    fdo = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if(fdo < 0) die("error: open");

		printf("Opening input device %s\n", input_device);
    fdi = open(input_device, O_RDONLY);
		free(input_device);
    if(fdi < 0) die("error: open");
    if(ioctl(fdi, EVIOCGRAB, 1) < 0) die("error: ioctl");

		setup_device(fdo);

		if (!DEBUG) daemon(0,0);

		int speed = 0;
    while(!interrupted)
    {
        if(read(fdi, &ev, sizeof(struct input_event)) < 0)
            die("error: read");

				if (ev.type==EV_KEY) {
					switch (ev.value) {
						case 0: speed = 0; break;
						case 1: speed = 5; break;
						case 2: speed = speed+10; break;	// incremental speed for repeat
					}
					if(VERBOSE) {
						printf("Got keycode 0x%x (%d)\n", ev.code, ev.code);
						fflush(stdout); 
					}
					// input codes can be found at https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h
					switch (ev.code) {
						case KEY_NUMERIC_1: move_mouse(fdo, -speed, -speed); break;
						case KEY_NUMERIC_2: move_mouse(fdo, 0, -speed); break;
						case KEY_NUMERIC_3: move_mouse(fdo, speed, -speed); break;
						case KEY_NUMERIC_4: move_mouse(fdo, -speed, 0); break;
						case KEY_NUMERIC_6: move_mouse(fdo, speed, 0); break;
						case KEY_NUMERIC_7: move_mouse(fdo, -speed, speed); break;
						case KEY_NUMERIC_8: move_mouse(fdo, 0, speed); break;
						case KEY_NUMERIC_9: move_mouse(fdo, speed, speed); break;
						case KEY_CLOSE: int keys[] = {KEY_LEFTALT, KEY_F4}; key_combination(fdo, 2, keys); break;
            default: if (write(fdo, &ev, sizeof(ev)) < 0) die("error: write");
					};
				} else {
   				if (write(fdo, &ev, sizeof(ev)) < 0) die("error: write");
				}
      	usleep(15000);
    }

    if(ioctl(fdo, UI_DEV_DESTROY) < 0) die("error: ioctl");

    close(fdi);
    close(fdo);

    return 0;
}
