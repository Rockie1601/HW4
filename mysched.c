/*
Name: Vijay Rakesh Reddy Bandela
BlazerId: vbandela
Project #: Homework3
To compile: <gcc -o search search.c or run make command>
To run: <./mysched <P>>
./mysched <P>

Replace <P> with the maximum number of parallel jobs you want to allow (typically the number of CPU cores).
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <errno.h>

#define VBANDELA_MAX_JOBS 100
#define VBANDELA_ARG_BUFFER_SIZE 10
#define VBANDELA_MAX_ARGS 256

typedef struct {
    int vbandela_jobid;
    char vbandela_command[256];
    pid_t vbandela_pid;
    int vbandela_status;
    time_t vbandela_startTime;
    time_t vbandela_endTime;
    int vbandela_exitStatus;
} vbandela_job;

vbandela_job vbandela_jobQueue[VBANDELA_MAX_JOBS];
int vbandela_jobCount = 0;
int vbandela_runningJobs = 0;
int vbandela_queueStart = 0;

void vbandela_addJob(char* cmd, int p) {
    if (vbandela_jobCount >= VBANDELA_MAX_JOBS) {
        printf("Job queue is full.\n");
        return;
    }
    int index = (vbandela_queueStart + vbandela_jobCount) % VBANDELA_MAX_JOBS;
    vbandela_jobQueue[index].vbandela_jobid = index + 1;
    strncpy(vbandela_jobQueue[index].vbandela_command, cmd, 255);
    vbandela_jobQueue[index].vbandela_command[255] = '\0'; // Ensure null termination
    vbandela_jobQueue[index].vbandela_status = 0; // Waiting
    vbandela_jobQueue[index].vbandela_startTime = time(NULL);
    vbandela_jobQueue[index].vbandela_endTime = 0;
    vbandela_jobQueue[index].vbandela_exitStatus = -1;
    printf("job %d added to the queue\n", vbandela_jobQueue[index].vbandela_jobid);
    vbandela_jobCount++;
}

void vbandela_parseCommand(char* command, char*** args, int* argCount) {
    int bufferSize = VBANDELA_ARG_BUFFER_SIZE;
    *args = malloc(bufferSize * sizeof(char*));
    if (!*args) {
        perror("Failed to allocate memory for args");
        exit(EXIT_FAILURE);
    }

    char* token = strtok(command, " ");
    *argCount = 0;

    while (token != NULL) {
        (*args)[*argCount] = strdup(token);
        (*argCount)++;

        if (*argCount >= bufferSize) {
            bufferSize *= 2;
            *args = realloc(*args, bufferSize * sizeof(char*));
            if (!*args) {
                perror("Failed to reallocate memory for args");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, " ");
    }

    (*args)[*argCount] = NULL;
}

void vbandela_startJob(int jobIndex, int p) {
    if (vbandela_runningJobs >= p) return;

    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        return;
    } else if (pid == 0) {
        char outFilename[260];
        char errFilename[260];
        sprintf(outFilename, "%d.out", vbandela_jobQueue[jobIndex].vbandela_jobid);
        sprintf(errFilename, "%d.err", vbandela_jobQueue[jobIndex].vbandela_jobid);

        if (!freopen(outFilename, "w", stdout)) {
            perror("Failed to redirect stdout");
            exit(EXIT_FAILURE);
        }
        if (!freopen(errFilename, "w", stderr)) {
            perror("Failed to redirect stderr");
            exit(EXIT_FAILURE);
        }

        char** args;
        int argCount;
        vbandela_parseCommand(vbandela_jobQueue[jobIndex].vbandela_command, &args, &argCount);
        fclose(stdout);
        fclose(stderr);
        execvp(args[0], args);
        perror("Exec failed");
        exit(EXIT_FAILURE);
    } else {
        vbandela_jobQueue[jobIndex].vbandela_pid = pid;
        vbandela_jobQueue[jobIndex].vbandela_status = 1; // Running
        vbandela_runningJobs++;
    }
}

void vbandela_updateJobs(int p) {
    int status;
    for (int i = 0; i < vbandela_jobCount; i++) {
        int index = (vbandela_queueStart + i) % VBANDELA_MAX_JOBS;
        if (vbandela_jobQueue[index].vbandela_status == 1) {
            pid_t result = waitpid(vbandela_jobQueue[index].vbandela_pid, &status, WNOHANG);
            if (result > 0) {
                vbandela_jobQueue[index].vbandela_status = 2; // Completed
                vbandela_jobQueue[index].vbandela_endTime = time(NULL);
                vbandela_jobQueue[index].vbandela_exitStatus = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
                vbandela_runningJobs--;
            } else if (result < 0 && errno != ECHILD) {
                perror("waitpid failed");
            }
        }
    }
}

void vbandela_showSubmitHistory() {
    printf("Job History:\n");
    for (int i = 0; i < vbandela_jobCount; i++) {
        printf("Job ID: %d, Command: %s, Start Time: %s",
               vbandela_jobQueue[i].vbandela_jobid, vbandela_jobQueue[i].vbandela_command,
               ctime(&vbandela_jobQueue[i].vbandela_startTime));

        if (vbandela_jobQueue[i].vbandela_status == 2) { // Completed jobs
            printf(", End Time: %s, Status: %s\n",
                   ctime(&vbandela_jobQueue[i].vbandela_endTime),
                   vbandela_jobQueue[i].vbandela_exitStatus == 0 ? "Success" : "Failure");
        } else if (vbandela_jobQueue[i].vbandela_status == 1) { // Running jobs
            printf(", Status: Running\n");
        } else if (vbandela_jobQueue[i].vbandela_status == 0) { // Waiting jobs
            printf(", Status: Waiting\n");
        } else {
            printf(", Status: Unknown\n");
        }
    }
}

void vbandela_showJobs() {
    for (int i = 0; i < vbandela_jobCount; i++) {
        int index = (vbandela_queueStart + i) % VBANDELA_MAX_JOBS;
        if (vbandela_jobQueue[index].vbandela_status < 2) {
            printf("Job ID: %d, Command: %s, Status: %s\n",
                   vbandela_jobQueue[index].vbandela_jobid, vbandela_jobQueue[index].vbandela_command,
                   vbandela_jobQueue[index].vbandela_status == 0 ? "Waiting" : "Running");
        }
    }
}

void vbandela_cleanupJobs() {
    int writeIndex = 0;
    for (int readIndex = 0; readIndex < vbandela_jobCount; readIndex++) {
        if (vbandela_jobQueue[readIndex].vbandela_status != 2) {
            if (writeIndex != readIndex) {
                vbandela_jobQueue[writeIndex] = vbandela_jobQueue[readIndex];
            }
            writeIndex++;
        }
    }
    vbandela_jobCount = writeIndex;
}

void vbandela_signalHandler(int sig) {
    int status, pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < vbandela_jobCount; i++) {
            if (vbandela_jobQueue[i].vbandela_pid == pid) {
                vbandela_jobQueue[i].vbandela_status = 2; // Completed
                vbandela_jobQueue[i].vbandela_endTime = time(NULL);
                vbandela_jobQueue[i].vbandela_exitStatus = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
                vbandela_runningJobs--;
                break;
            }
        }
    }
}

void vbandela_freeArgs(char** args, int argCount) {
    for (int i = 0; i < argCount; i++) {
        free(args[i]);
    }
    free(args);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <P>\n", argv[0]);
        return 1;
    }

    int p = atoi(argv[1]);
    char command[256];

    struct sigaction sa;
    sa.sa_handler = &vbandela_signalHandler;
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);

    while (1) {
        printf("Enter command> ");
        if (!fgets(command, 256, stdin)) {
            perror("Error reading command");
            continue;
        }
        command[strcspn(command, "\n")] = 0; // Remove newline character

        if (strcmp(command, "submithistory") == 0) {
            vbandela_showSubmitHistory();
        } else if (strncmp(command, "submit", 6) == 0) {
            vbandela_addJob(command + 7, p);
        } else if (strcmp(command, "showjobs") == 0) {
            vbandela_showJobs();
        } else if (strcmp(command, "exit") == 0) {
            break;
        }

        vbandela_updateJobs(p);
        vbandela_cleanupJobs();

        for (int i = 0; i < vbandela_jobCount; i++) {
            int index = (vbandela_queueStart + i) % VBANDELA_MAX_JOBS;
            if (vbandela_jobQueue[index].vbandela_status == 0) {
                vbandela_startJob(index, p);
            }
        }
    }

    return 0;
}
