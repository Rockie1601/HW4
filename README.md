README for mysched.c
Overview
mysched.c is a C program designed to simulate a simple job scheduler. It manages the execution of non-interactive jobs (i.e., jobs that run in the background without requiring user interaction) in a controlled environment. The program limits the number of concurrently running jobs based on the available system resources, typically the number of CPU cores.
Features
•	Job Submission: Users can submit jobs to be executed. Each job is associated with a unique ID.
•	Job Status Tracking: The program tracks the status of each job (waiting, running, or completed).
•	Output and Error Handling: Outputs and errors of each job are redirected to separate files named <jobid>.out and <jobid>.err.
•	Job History: The program maintains a history of all submitted jobs, displaying their start and end times, as well as their final status (success or failure).
Functions
•	vbandela_addJob: Adds a new job to the job queue.
•	vbandela_startJob: Starts a job from the queue.
•	vbandela_updateJobs: Updates the status of all jobs in the queue.
•	vbandela_showSubmitHistory: Displays the history of all submitted jobs.
•	vbandela_showJobs: Lists all currently waiting or running jobs.
•	vbandela_cleanupJobs: Cleans up completed jobs from the job queue.
•	vbandela_signalHandler: Handles signals for job completion.
•	vbandela_freeArgs: Frees dynamically allocated argument arrays.
•	vbandela_parseCommand: Parses the submitted command into executable and arguments.
Compilation and Execution
To compile mysched.c, use the following command in a terminal:
•	gcc -o mysched mysched.c
To run the program, execute it with the number of parallel jobs allowed as an argument:
•	./mysched <P>

Replace <P> with the maximum number of parallel jobs you want to allow (typically the number of CPU cores).
Usage
After starting mysched, you can use the following commands:
•	submit <command> <arguments>: Submits a new job.
•	showjobs: Lists all jobs that are currently waiting or running.
•	submithistory: Displays the history of all jobs, including their status and start/end times.
•	exit: Exits the program.
Example Session
$ ./mysched 2
Enter command> submit sleep 10
job 1 added to the queue
Enter command> submit ls -l
job 2 added to the queue
Enter command> showjobs
Job ID: 1, Command: sleep 10, Status: Running
Job ID: 2, Command: ls -l, Status: Running
Enter command> submithistory
Job ID: 1, Command: sleep 10, Start Time: ..., End Time: ..., Status: Success
Job ID: 2, Command: ls -l, Start Time: ..., Status: Running
Enter command> exit

Output Files
For each job, two files are generated in the current directory:
•	<jobid>.out: Contains the standard output of the job.
•	<jobid>.err: Contains the error output of the job.



