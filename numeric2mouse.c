// based on https://www.kernel.org/doc/html/v4.12/input/uinput.html
#define DEBUG 1
#define VERBOSE 1

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <libgen.h>
#include <signal.h>
#include <time.h>
#include <linux/uinput.h>
#include <yaml.h>
#include "keymappings.h"

#define die(str, args...) do { \
        perror(str); \
        exit(EXIT_FAILURE); \
    } while(0)

#define MAX_KEYS 10
#define MAX_MAPPINGS 256

typedef enum {
    ACTION_MOVE_MOUSE,
    ACTION_KEY_COMBO,
    ACTION_EXECUTE,
    ACTION_PASSTHROUGH
} action_type_t;

typedef struct {
    action_type_t type;
    union {
        struct {
            int x;
            int y;
        } mouse;
        struct {
            int keys[MAX_KEYS];
            int count;
        } combo;
        struct {
            char command[512];
            int rate_limit_seconds;
            time_t last_exec_time;
        } exec;
    } data;
} action_t;

typedef struct {
    int code;
    action_t action;
} key_mapping_t;

key_mapping_t mappings[MAX_MAPPINGS];
int mapping_count = 0;

volatile int interrupted = 0;

void handle_int(int num) {
    printf("INTERRUPT %d", num); fflush(stdout);
    interrupted = 1;
}

void load_config(const char* config_path) {
    FILE* file = fopen(config_path, "r");
    if (!file) {
        fprintf(stderr, "Warning: Could not open config file %s, using defaults\n", config_path);
        return;
    }

    yaml_parser_t parser;
    yaml_event_t event;

    if (!yaml_parser_initialize(&parser)) {
        fprintf(stderr, "Failed to initialize YAML parser\n");
        fclose(file);
        return;
    }

    yaml_parser_set_input_file(&parser, file);

    char current_key[256] = "";
    char current_field[256] = "";
    int in_mappings = 0;
    int in_mapping_entry = 0;
    int in_action = 0;
    int in_keys_sequence = 0;
    key_mapping_t temp_mapping;
    memset(&temp_mapping, 0, sizeof(temp_mapping));
    temp_mapping.action.data.exec.rate_limit_seconds = 0;  // 0 = no rate limit

    do {
        if (!yaml_parser_parse(&parser, &event)) {
            fprintf(stderr, "YAML parse error\n");
            break;
        }

        switch (event.type) {
            case YAML_MAPPING_START_EVENT:
                if (in_mappings && !in_mapping_entry) {
                    in_mapping_entry = 1;
                }
                break;

            case YAML_SCALAR_EVENT: {
                const char* value = (const char*)event.data.scalar.value;
                
                if (!in_mappings && strcmp(value, "mappings") == 0) {
                    in_mappings = 1;
                } else if (in_mapping_entry && !in_action && strlen(current_field) == 0) {
                    strncpy(current_field, value, sizeof(current_field) - 1);
                    
                    if (strcmp(current_field, "key") == 0) {
                        // Next scalar will be the key name
                    } else if (strcmp(current_field, "action") == 0) {
                        in_action = 1;
                    }
                } else if (in_mapping_entry && strcmp(current_field, "key") == 0) {
                    temp_mapping.code = parse_key_code(value);
                    strcpy(current_field, "");
                } else if (in_action && strlen(current_key) == 0) {
                    strncpy(current_key, value, sizeof(current_key) - 1);
                    
                    if (strcmp(current_key, "type") == 0) {
                        // Next value is action type
                    } else if (strcmp(current_key, "x") == 0) {
                        // Next value is x coordinate
                    } else if (strcmp(current_key, "y") == 0) {
                        // Next value is y coordinate
                    } else if (strcmp(current_key, "keys") == 0) {
                        in_keys_sequence = 1;
                    } else if (strcmp(current_key, "command") == 0) {
                        // Next value is command to execute
                    } else if (strcmp(current_key, "rateLimitInSeconds") == 0) {
                        // Next value is rate limit in seconds
                    }
                } else if (in_action && strlen(current_key) > 0) {
                    if (strcmp(current_key, "type") == 0) {
                        if (strcmp(value, "move_mouse") == 0) {
                            temp_mapping.action.type = ACTION_MOVE_MOUSE;
                        } else if (strcmp(value, "key_combination") == 0) {
                            temp_mapping.action.type = ACTION_KEY_COMBO;
                        } else if (strcmp(value, "execute") == 0) {
                            temp_mapping.action.type = ACTION_EXECUTE;
                        }
                    } else if (strcmp(current_key, "x") == 0) {
                        temp_mapping.action.data.mouse.x = atoi(value);
                    } else if (strcmp(current_key, "y") == 0) {
                        temp_mapping.action.data.mouse.y = atoi(value);
                    } else if (strcmp(current_key, "command") == 0) {
                        strncpy(temp_mapping.action.data.exec.command, value, 
                                sizeof(temp_mapping.action.data.exec.command) - 1);
                    } else if (strcmp(current_key, "rateLimitInSeconds") == 0) {
                        temp_mapping.action.data.exec.rate_limit_seconds = atoi(value);
                    } else if (in_keys_sequence) {
                        int key_code = parse_key_code(value);
                        if (key_code >= 0 && temp_mapping.action.data.combo.count < MAX_KEYS) {
                            temp_mapping.action.data.combo.keys[temp_mapping.action.data.combo.count++] = key_code;
                        }
                    }
                    strcpy(current_key, "");
                }
                break;
            }

            case YAML_SEQUENCE_END_EVENT:
                if (in_keys_sequence) {
                    in_keys_sequence = 0;
                    strcpy(current_key, "");
                }
                break;

            case YAML_MAPPING_END_EVENT:
                if (in_action) {
                    in_action = 0;
                } else if (in_mapping_entry) {
                    // Save the completed mapping
                    if (temp_mapping.code >= 0 && mapping_count < MAX_MAPPINGS) {
                        mappings[mapping_count++] = temp_mapping;
                    }
                    memset(&temp_mapping, 0, sizeof(temp_mapping));
                    in_mapping_entry = 0;
                    strcpy(current_field, "");
                }
                break;

            default:
                break;
        }

        if (event.type != YAML_STREAM_END_EVENT)
            yaml_event_delete(&event);

    } while (event.type != YAML_STREAM_END_EVENT);

    yaml_event_delete(&event);
    yaml_parser_delete(&parser);
    fclose(file);

    printf("Loaded %d key mappings from %s\n", mapping_count, config_path);
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
   for(unsigned long i = 0; i < KEY_REFRESH_RATE_TOGGLE; i++)
       if(ioctl(fdo, UI_SET_KEYBIT, i) < 0) die("error: ioctl announce keys");

   memset(&usetup, 0, sizeof(usetup));
   usetup.id.bustype = BUS_USB;
   usetup.id.vendor = 0x1234;
   usetup.id.product = 0x5678;
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
    int i;
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
}

int can_execute(action_t* action) {
    if (action->data.exec.rate_limit_seconds == 0) {
				if (VERBOSE) printf("No rate limit\n");
        return 1;
    }

    time_t now = time(NULL);
    time_t time_since_last = now - action->data.exec.last_exec_time;
    if (VERBOSE) printf("Time since last command %ld\n", time_since_last);
    return time_since_last >= action->data.exec.rate_limit_seconds;
}

void record_execution(action_t* action) {
    action->data.exec.last_exec_time = time(NULL);
}

void execute_command(action_t* action) {
    if (!can_execute(action)) {
        if (VERBOSE) {
            time_t now = time(NULL);
            int wait_time = action->data.exec.rate_limit_seconds - (now - action->data.exec.last_exec_time);
            printf("Rate limit: command blocked, wait %d more seconds: %s\n", 
                   wait_time, action->data.exec.command);
        }
        return;
    }
    
    if (VERBOSE) {
        printf("Executing: %s\n", action->data.exec.command);
    }
    const char *command = action->data.exec.command;
		char *background = malloc(strlen(command) + 2); // +2 for '&' and '\0'
		sprintf(background, "%s&", command);
		system(background);
		free(background);
}

int handle_systemd(int argc, char* argv[])
{
    if (argc == 2 && (
                !strcmp(argv[1], "stop")
                || !strcmp(argv[1], "restart") 
                || !strcmp(argv[1], "reload") 
                )) {
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
        while ((ep = readdir(dp))) {
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
    int fdo, fdi;
    struct input_event ev;
    char* input_device = NULL;

    struct sigaction int_handler = {.sa_handler=handle_int};
    sigaction(SIGINT, &int_handler, 0);
    sigaction(SIGTERM, &int_handler, 0);

    argc = handle_systemd(argc, argv);

    // Load configuration
    load_config("/etc/numeric2mouse.yaml");

    if (argc == 2) {
        input_device = argv[1];
    } else {
        input_device = get_input_device("/dev/input/by-path/", "platform-ir-receiver");
        if (!input_device) {
            die("error: specify input device, i.e. /dev/input/by-path/platform-ir-receiver@11-event");
        }
    }
    system("/etc/init.d/inputlirc reload");

    fdo = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if(fdo < 0) die("error: open uinput");

    printf("Opening input device %s\n", input_device);
    fdi = open(input_device, O_RDONLY);
    free(input_device);
    if(fdi < 0) die("error: open input_device");
    if(ioctl(fdi, EVIOCGRAB, 1) < 0) die("error: ioctl");

    setup_device(fdo);

    if (!DEBUG) daemon(0,0);

    int speed = 0;
    while(!interrupted)
    {
        if(read(fdi, &ev, sizeof(struct input_event)) < 0)
            die("error: read");

        if (ev.type == EV_KEY) {
            switch (ev.value) {
                case 0: speed = 0; break;
                case 1: speed = 5; break;
                case 2: speed = speed+10; break;
            }
            
            if(VERBOSE) {
                printf("Got keycode 0x%x (%d)\n", ev.code, ev.code);
                fflush(stdout); 
            }

            // Check mappings
            int handled = 0;
            for (int i = 0; i < mapping_count; i++) {
                if (mappings[i].code == ev.code) {
                    action_t* action = &mappings[i].action;
                    switch (action->type) {
                        case ACTION_MOVE_MOUSE: {
                            int x = action->data.mouse.x;
                            int y = action->data.mouse.y;
                            // Apply speed multiplier
                            if (x != 0) x = (x < 0 ? -1 : 1) * speed;
                            if (y != 0) y = (y < 0 ? -1 : 1) * speed;
                            move_mouse(fdo, x, y);
                            handled = 1;
                            break;
                        }
                        case ACTION_KEY_COMBO:
                            key_combination(fdo, action->data.combo.count, action->data.combo.keys);
                            handled = 1;
                            break;
                        case ACTION_EXECUTE:
                            // Only execute on key press (value == 1), not on repeat or release
                            if (ev.value == 1) {
                                execute_command(action);
                            }
                            handled = 1;
                            break;
                        case ACTION_PASSTHROUGH:
                            // Fall through to default
                            break;
                    }
                    break;
                }
            }

            if (!handled) {
                if (write(fdo, &ev, sizeof(ev)) < 0) die("error: write");
            }
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
