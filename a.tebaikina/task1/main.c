#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/types.h>


typedef struct {
    char opt;
    char *arg;
} OPTIONS;

// rlimit:
// rlim_cur - soft limit
// rlim_max - hard limit
static void print_rlim(const char *tag, int resource) {
    struct rlimit rl;
    if (getrlimit(resource,&rl) != 0) {
        perror("getrlimit");
        return;
    }
    printf("%s soft = %s, hard = %s\n", tag,
            (rl.rlim_cur == RLIM_INFINITY) ? "unlimited" :
                ({ static char b1[64]; snprintf(b1,sizeof(b1),"%llu",
                        (unsigned long long)rl.rlim_cur);b1; }),
            (rl.rlim_max == RLIM_INFINITY) ? "unlimited" :
            ({ static char b2[64]; snprintf(b2,sizeof(b2), "%llu",
                                            (unsigned long long)rl.rlim_max); b2; }));

}
// там еще одну функцию надо разобрать и написать
static int parse_size(const char *s, rlim_t *out) {
    if (!s || !out) {
        return -1;
    }
    // поддержка "unlimited" (без регистра)
    if (strncasecmp(s, "unlimited", 9) == 0 && s[9] == '\0') {
        *out = RLIM_INFINITY;
        return 0;
    }
    errno = 0;
    char *end = NULL;
    unsigned long long v =xnj strtoull(s, &end, 10);

    if (errno != 0 || end == s || *end != '\0') return -1;

    *out = (rlim_t)v;
    return 0;

}


// погнали функции

// -i
// uid_t - тип для user ID
static void if_i(void) {
    uid_t r_uid = getuid(), e_uid = geteuid();
    gid_t r_gid = getgid(), e_gid = getegid();
    printf("UID: real = %u  effective = %u; GID: real = %u effective = %u\n",
            (unsigned)r_uid,(unsigned)e_uid, (unsigned)r_gid, (unsigned)e_gid);
}

// -s
static void if_s(void) {
    if (setpgid(0,0) != 0) {
        perror("setpgid");
    } else {
        printf("Process %d is now group leader (PGID = %d)\n",
            getpid(),getpgrp());
    }
}

// -P
static void if_p(void) {
    printf("PID = %d PPID = %d PGID = %d\n", getpid(),getppid(),getpgrp());
}

// -u
static void if_u(void) {
    print_rlim ("RLIMIT_FSIZE", RLIMIT_FSIZE); //??
}

// -U
static void if_U(const char *arg) {

}

// -c
static void if_c(void) {
    print_rlim("RLIMIT_CORE", RLIMIT_CORE);
}

// -C
static void if_C(const char *arg) {
    struct rlimit rl;
    if (getrlimit(RLIMIT_CORE, &rl) != 0) {
        perror("getrlimit"; return;)
    }
    rlim_t value;
    if (parse_size(arg,&value) != 0) {

    }
}

// -d
static void if_d(void) {
    char *cwd = getcwd(NULL, 0);
    if (!cwd) {
        perror("getcwd");
        return;
    }
    printf("%s\n", cwd);
    free(cwd);
}

// -v
static void if_v(void){
    for (char **e = environ; e && *e; ++e) {
        puts(*e);
    }
}

// -V


//буква с двоеточием - опция с аргументом
int main(int argc, char *argv[]) {
    const char *programm = argv[0];
    const char *option_str = "ispuU:cC:dvV:";

    OPTIONS *ops = calloc((size_t)argc, sizeof(OPTIONS));
    if (!ops) {
        perror ("calloc");
        return 1;
    }
    int n_ops = 0;

    opterr = 1;

    int letter;
    while ((letter = getopt(argc, argv, optstring)) != -1) {
        switch (letter) {
            // опции без аргументов
            case 'i': case 's': case 'p':
            case 'u': case 'c': case 'd': case 'v':
                ops[n_ops++] = (OPTIONS){.opt = (char)letter, .arg = NULL };
                break;
            // опции с аргументами
            case 'U': case 'C': case 'V':
                ops[n_ops++] = (OPTIONS){ .opt = (char)letter, .arg = optarg };
                break;
            case '?':
            default:
                free(ops);
                return 2;
        }
    }

    for (int i = n_ops - 1; i >= 0; --i) {
        switch (ops[i].opt) {
            case 'i': if_i(); break;
            case 's': if_s(); break;
            case 'p': if_p(); break;
            case 'u': if_u(); break;
            case 'U': if_U(ops[i].arg); break;
            case 'c': if_c(); break;
            case 'C': if_C(ops[i].arg); break;
            case 'd': do_d(); break;
            case 'v': do_v(); break;
            case 'V': do_V_set(ops[i].arg); break;
            default: break;
        }
    }

    free(ops);
    return 0;
}









