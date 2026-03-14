/*
    linux_sysprog_expert.c

    Expert Linux System Programming Examples

    Concepts:
    1. eventfd
    2. timerfd
    3. signalfd
    4. inotify
    5. memfd_create
    6. shared memory ring buffer
    7. worker process pool
    8. mini shell
    9. sendfile (zero-copy)
    10. epoll event loop
*/

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include <sys/signalfd.h>
#include <sys/inotify.h>
#include <sys/mman.h>
#include <sys/sendfile.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

/* -------------------------------------------------- */
/* 1. eventfd example                                 */
/* -------------------------------------------------- */

void demo_eventfd()
{
    int efd = eventfd(0, 0);

    uint64_t value = 5;

    write(efd, &value, sizeof(value));

    read(efd, &value, sizeof(value));

    printf("eventfd value read: %lu\n", value);

    close(efd);
}

/* -------------------------------------------------- */
/* 2. timerfd example                                 */
/* -------------------------------------------------- */

void demo_timerfd()
{
    int tfd = timerfd_create(CLOCK_REALTIME, 0);

    struct itimerspec timer;

    timer.it_value.tv_sec = 2;
    timer.it_value.tv_nsec = 0;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_nsec = 0;

    timerfd_settime(tfd, 0, &timer, NULL);

    printf("Waiting for timer...\n");

    uint64_t expirations;

    read(tfd, &expirations, sizeof(expirations));

    printf("Timer expired %lu times\n", expirations);

    close(tfd);
}

/* -------------------------------------------------- */
/* 3. signalfd example                                */
/* -------------------------------------------------- */

void demo_signalfd()
{
    sigset_t mask;

    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);

    sigprocmask(SIG_BLOCK, &mask, NULL);

    int sfd = signalfd(-1, &mask, 0);

    printf("Press Ctrl+C...\n");

    struct signalfd_siginfo fdsi;

    read(sfd, &fdsi, sizeof(fdsi));

    printf("Received signal via signalfd: %d\n", fdsi.ssi_signo);

    close(sfd);
}

/* -------------------------------------------------- */
/* 4. inotify filesystem monitoring                   */
/* -------------------------------------------------- */

void demo_inotify()
{
    int fd = inotify_init();

    int wd = inotify_add_watch(fd, ".", IN_CREATE | IN_DELETE);

    char buffer[1024];

    printf("Monitoring current directory...\n");

    int length = read(fd, buffer, sizeof(buffer));

    if (length > 0)
        printf("Filesystem event detected\n");

    inotify_rm_watch(fd, wd);
    close(fd);
}

/* -------------------------------------------------- */
/* 5. memfd_create example                            */
/* -------------------------------------------------- */

void demo_memfd()
{
    int fd = memfd_create("sharedmem", 0);

    write(fd, "Hello memfd", 11);

    lseek(fd, 0, SEEK_SET);

    char buffer[50];

    read(fd, buffer, 11);

    buffer[11] = 0;

    printf("Read from memfd: %s\n", buffer);

    close(fd);
}

/* -------------------------------------------------- */
/* 6. Shared memory ring buffer                       */
/* -------------------------------------------------- */

#define RING_SIZE 10

typedef struct
{
    int buffer[RING_SIZE];
    int head;
    int tail;
} ring_buffer;

void demo_ring_buffer()
{
    ring_buffer *rb = mmap(NULL,
                           sizeof(ring_buffer),
                           PROT_READ | PROT_WRITE,
                           MAP_SHARED | MAP_ANONYMOUS,
                           -1,
                           0);

    rb->head = 0;
    rb->tail = 0;

    for(int i=0;i<5;i++)
    {
        rb->buffer[rb->head] = i;
        rb->head = (rb->head + 1) % RING_SIZE;
    }

    while(rb->tail != rb->head)
    {
        printf("Read %d\n", rb->buffer[rb->tail]);
        rb->tail = (rb->tail + 1) % RING_SIZE;
    }

    munmap(rb, sizeof(ring_buffer));
}

/* -------------------------------------------------- */
/* 7. Worker process pool                             */
/* -------------------------------------------------- */

void worker()
{
    printf("Worker %d processing job\n", getpid());
    sleep(1);
    exit(0);
}

void demo_worker_pool()
{
    int workers = 3;

    for(int i=0;i<workers;i++)
    {
        if(fork()==0)
            worker();
    }

    for(int i=0;i<workers;i++)
        wait(NULL);

    printf("All workers completed\n");
}

/* -------------------------------------------------- */
/* 8. Mini shell implementation                       */
/* -------------------------------------------------- */

void demo_shell()
{
    char command[100];

    while(1)
    {
        printf("myshell> ");
        fgets(command,sizeof(command),stdin);

        command[strcspn(command,"\n")] = 0;

        if(strcmp(command,"exit")==0)
            break;

        pid_t pid = fork();

        if(pid==0)
        {
            execlp(command,command,NULL);
            perror("exec");
            exit(1);
        }
        else
            wait(NULL);
    }
}

/* -------------------------------------------------- */
/* 9. Zero-copy sendfile                              */
/* -------------------------------------------------- */

void demo_sendfile()
{
    int in = open("demo_file.txt",O_RDONLY);
    int out = open("copy.txt",O_CREAT|O_WRONLY,0644);

    struct stat st;
    fstat(in,&st);

    sendfile(out,in,NULL,st.st_size);

    printf("File copied using sendfile\n");

    close(in);
    close(out);
}

/* -------------------------------------------------- */
/* 10. epoll event loop                               */
/* -------------------------------------------------- */

void demo_epoll_loop()
{
    int epfd = epoll_create1(0);

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = STDIN_FILENO;

    epoll_ctl(epfd,EPOLL_CTL_ADD,STDIN_FILENO,&ev);

    struct epoll_event events[1];

    printf("Type something...\n");

    while(1)
    {
        int n = epoll_wait(epfd,events,1,-1);

        if(n>0)
        {
            char buf[100];
            int bytes = read(STDIN_FILENO,buf,sizeof(buf));
            write(STDOUT_FILENO,buf,bytes);
        }
    }
}

/* -------------------------------------------------- */
/* Menu                                               */
/* -------------------------------------------------- */

void menu()
{
    printf("\nExpert Linux System Programming\n");
    printf("--------------------------------\n");
    printf("1 eventfd\n");
    printf("2 timerfd\n");
    printf("3 signalfd\n");
    printf("4 inotify\n");
    printf("5 memfd_create\n");
    printf("6 shared memory ring buffer\n");
    printf("7 worker process pool\n");
    printf("8 mini shell\n");
    printf("9 sendfile\n");
    printf("10 epoll event loop\n");
    printf("0 exit\n");
}

/* -------------------------------------------------- */

int main()
{
    int choice;

    while(1)
    {
        menu();

        printf("Choice: ");
        scanf("%d",&choice);
        getchar();

        switch(choice)
        {
            case 1: demo_eventfd(); break;
            case 2: demo_timerfd(); break;
            case 3: demo_signalfd(); break;
            case 4: demo_inotify(); break;
            case 5: demo_memfd(); break;
            case 6: demo_ring_buffer(); break;
            case 7: demo_worker_pool(); break;
            case 8: demo_shell(); break;
            case 9: demo_sendfile(); break;
            case 10: demo_epoll_loop(); break;
            case 0: exit(0);
        }
    }

    return 0;
}
