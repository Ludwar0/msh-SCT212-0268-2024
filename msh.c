#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_ARGS 256
static volatile sig_atomic_t foreground_pid = 0;
static int last_status = 0;

static void shell_sigint(int sig)
{
    (void)sig;
    /* The shell survives; the foreground child receives SIGINT separately. */
    if (foreground_pid > 0) kill((pid_t)foreground_pid, SIGINT);
    write(STDOUT_FILENO, "\n", 1);
}

static void reap_background(void)
{
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
        fprintf(stderr, "[msh] background process %ld finished\n", (long)pid);
}

static void print_help(void)
{
    puts("Built-ins: cd [dir], help, exit\nExternal commands support trailing &, and output redirection with > file.");
}

static int run_builtin(char **argv)
{
    if (!argv[0]) return 1;
    if (strcmp(argv[0], "help") == 0) { print_help(); last_status = 0; return 1; }
    if (strcmp(argv[0], "exit") == 0) exit(0);
    if (strcmp(argv[0], "cd") == 0) {
        const char *dest = argv[1] ? argv[1] : getenv("HOME");
        if (!dest) { fprintf(stderr, "msh: HOME is not set\n"); last_status = 1; }
        else if (chdir(dest) < 0) { perror("msh: cd"); last_status = 1; }
        else last_status = 0;
        return 1;
    }
    return 0;
}

static void execute(char **argv)
{
    int background = 0, redirect = -1;
    size_t n = 0;
    while (argv[n]) n++;
    if (n && strcmp(argv[n - 1], "&") == 0) { background = 1; argv[--n] = NULL; }
    for (size_t i = 0; i < n; i++) {
        if (strcmp(argv[i], ">") == 0) {
            if (i + 1 >= n || i == 0 || i + 2 != n) {
                fprintf(stderr, "msh: usage: command > file\n"); last_status = 2; return;
            }
            redirect = (int)i; argv[i] = NULL; break;
        }
    }
    if (!argv[0]) return;
    pid_t pid = fork();
    if (pid < 0) { perror("msh: fork"); last_status = 1; return; }
    if (pid == 0) {
        struct sigaction sa = {0}; sa.sa_handler = SIG_DFL; sigemptyset(&sa.sa_mask);
        sigaction(SIGINT, &sa, NULL);
        if (redirect >= 0) {
            int fd = open(argv[redirect + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) { perror("msh: open"); _exit(126); }
            if (dup2(fd, STDOUT_FILENO) < 0) { perror("msh: dup2"); close(fd); _exit(126); }
            close(fd);
        }
        execvp(argv[0], argv);
        fprintf(stderr, "msh: %s: %s\n", argv[0], strerror(errno));
        _exit(errno == ENOENT ? 127 : 126);
    }
    if (background) {
        printf("[msh] started background pid %ld\n", (long)pid);
        last_status = 0;
    } else {
        int status;
        foreground_pid = (sig_atomic_t)pid;
        pid_t waited;
        do { waited = waitpid(pid, &status, 0); } while (waited < 0 && errno == EINTR);
        foreground_pid = 0;
        if (waited < 0) { perror("msh: waitpid"); last_status = 1; }
        else if (WIFEXITED(status)) last_status = WEXITSTATUS(status);
        else if (WIFSIGNALED(status)) last_status = 128 + WTERMSIG(status);
    }
}

int main(void)
{
    struct sigaction sa = {0}; sa.sa_handler = shell_sigint; sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    char *line = NULL; size_t capacity = 0;
    for (;;) {
        reap_background();
        printf("msh[%d]> ", last_status); fflush(stdout);
        errno = 0;
        ssize_t length = getline(&line, &capacity, stdin);
        if (length < 0) { if (feof(stdin)) break; if (errno == EINTR) { clearerr(stdin); continue; } perror("msh: getline"); break; }
        char *argv[MAX_ARGS]; size_t argc = 0;
        char *save = NULL;
        for (char *tok = strtok_r(line, " \t\r\n", &save); tok; tok = strtok_r(NULL, " \t\r\n", &save)) {
            if (argc + 1 >= MAX_ARGS) { fprintf(stderr, "msh: too many arguments\n"); argc = 0; break; }
            argv[argc++] = tok;
        }
        argv[argc] = NULL;
        if (argc && !run_builtin(argv)) execute(argv);
    }
    free(line);
    reap_background();
    puts("\n[msh] exiting");
    return 0;
}
