// Joseph Venetucci 22-JUL-2018 CS510 ALSP
// Signal Handler for SIGXCPU

#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

/* Signal Handler for SIGXCPU
 *   The default action for SIGXCPU is to terminate the process
 *   This handler instead returns control to the main process, which
 *   means that SIGXCPU will continue to get sent.
 *   When the process has hit it's hard limit, SIGKILL is sent instead of
 *   SIGXCPU.
**/
void
sigxcpu(int signo, siginfo_t* info, void* context) {
  struct rlimit limits;

  if (getrlimit(RLIMIT_CPU, &limits) == -1)
    printf("Error getting limit....continuing\n");
  if (limits.rlim_max == -1)
    printf("PID %d has exceeded it's soft limit for CPU Time. Unlimited hard limit, so the process will continue running\n", getpid());
  else {
    printf("PID %d has exceeded it's soft limit for CPU Time and will exit in %ld seconds\n", getpid(), limits.rlim_max - limits.rlim_cur);
  }
}

int
test_sigxcpu(void) {

  int pid, rstatus;

  if ((pid = fork()) < 0) {
    printf("Unable to fork...\n");
    return -1;
  }

  if (pid == 0) {    // Child
    struct rlimit new;
    struct sigaction sa;

    sa.sa_sigaction = &sigxcpu;
    sa.sa_flags     = SA_SIGINFO;

    // Soft CPU time of 5 seconds, max of 10.
    new.rlim_max = 10;
    new.rlim_cur = 5;

    if (sigaction(SIGXCPU, &sa, NULL) == -1) {
	  printf("Unable to set signal handler in child...\n");
      return -1;
	}
    if (setrlimit(RLIMIT_CPU, &new) == -1) {
      printf("BAD SET\n");
	  return -1;
	}

    for (;;) { /* Take up CPU time */ }

  } else {  // Parent
    waitpid(pid, &rstatus, 0);
	
	// The child should of been terminated by SIGKILL
	if (WIFSIGNALED(rstatus) && (WTERMSIG(rstatus) == SIGKILL))
	  return 0;
	else
	  return -1;
  }
}
