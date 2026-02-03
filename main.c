#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

char tmpdir[64] = {0};
volatile sig_atomic_t stop_requested = 0;

int run(char** argv){
    pid_t pid = fork();

    if (pid == 0){
        execv(argv[0], argv);
        perror("execv failed");
        _exit(1);
    }

    if (pid < 0){
        return -1;
    }

	int status;
    while (waitpid(pid, &status, 0) < 0) {
        if (errno == EINTR) { 
            if (stop_requested){
                continue;
            }
            
            continue;
        }
        
        return -1;
    }

    if (WIFEXITED(status)){
        return WEXITSTATUS(status);
    }

    if (WIFSIGNALED(status)){
        return -1;
    }

    return -1;
}

void Cleanup(void){
    if (tmpdir[0] == '\0' || strcmp(tmpdir, "/") == 0){
        return;
    }

    stop_requested = 0;

    chdir("/");
    char* rm_args[] = {"/usr/bin/rm", "-rf", tmpdir, NULL};
    run(rm_args);

    tmpdir[0] = '\0';
}

void SignalHandler(int sig){
    (void)sig;
    stop_requested = 1;
}

int main(int argc, char** argv){
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SignalHandler;

    if (sigaction(SIGINT, &sa, NULL) == -1){
        perror("sigaction");
        return 1;
    }

    if (argc != 2){
        printf("Usage: %s <name>\n", argv[0]);
        return 1;
    }

    if (geteuid() == 0){
        printf("Do not run this as root!\n");
        return 1;
    }

    for (char *p = argv[1]; *p; p++) {
        if (!isalnum((unsigned char)*p) && *p != '.' && *p != '_' && *p != '+' && *p != '-'){
            printf("Invalid name!\n");
            return 1;
        }
    }

    strcpy(tmpdir, "/tmp/aurXXXXXX");
    if (!mkdtemp(tmpdir)){
        perror("mkdtemp");
        return 1;
    }

    atexit(Cleanup);

    if (stop_requested) return 0;

    if (chdir(tmpdir) != 0){
        perror("chdir");
        return 1;
    }

    if (stop_requested) return 0;

    char url[1024];
    int n = snprintf(url, sizeof(url), "https://aur.archlinux.org/%s.git", argv[1]);
    if (n < 0 || n >= (int)sizeof(url)){
        fprintf(stderr, "URL too long.\n");
        return 1;
    }

    char* git_args[] = {"/usr/bin/git", "clone", url, argv[1], NULL};
    if (run(git_args) != 0 || stop_requested){
        if (stop_requested){
            fprintf(stderr, "interrupted by user\n");
        }
        
        return 1;
    }

    if (stop_requested) return 0;

    if (chdir(argv[1]) != 0){
        perror("chdir");
        return 1;
    }

    if (stop_requested) return 0;

    char* makepkg_args[] = {"/usr/bin/makepkg", "-si", NULL};
    if (run(makepkg_args) != 0 || stop_requested){
        if (stop_requested){
            fprintf(stderr, "interrupted by user\n");
        }
        
        return 1;
    }

    return 0;
}
