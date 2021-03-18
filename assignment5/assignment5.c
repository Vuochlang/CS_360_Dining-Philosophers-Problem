#include <unistd.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

int randomGaussian(int mean, int stddev);

int main(int argc, char *argv[]){
    int chopsticks[5];
    struct sembuf pickUp[1] = {{0, -1, 0}};  // pick up a chopstick
    struct sembuf putDown[1] = {{0, 1, 0}};  // put down a chopstick

    for (int i = 0; i < 5; i++) {
        chopsticks[i] = semget(IPC_PRIVATE, 1, IPC_CREAT | IPC_EXCL |0600);
        if (chopsticks[i] < 0) {
            write(2, strerror(errno), strlen(strerror(errno)));
            return (errno);
        }
        if (semop(chopsticks[i], putDown, 1) < 0) {
            write(2, strerror(errno), strlen(strerror(errno)));
            return (errno);
        }
    }

    int id = 0;

    while (id < 5) {
        if (fork() == 0) {
            int eating = 0;
            int thinking = 0;
            int cycle = 0;
            int temp;

            srand(id); // random

            temp = randomGaussian(11, 7);
            if (temp < 0)
                temp = 0;
            printf("Philosopher %d is thinking for %d seconds (%d)\n", id, temp, thinking);
            thinking += temp;
            sleep(temp);

            while (eating < 100) {
                int a, b;

                a = semop(chopsticks[id % 5], pickUp, 1);  // chopstick to the left
                b = semop(chopsticks[(id + 1) % 5], pickUp, 1);  // chopstick to the right

                if (a == 0 && b == 0) { // picked up two chopsticks
                    temp = randomGaussian(9, 3);
                    if (temp < 0)
                        temp = 0;
                    printf("Philosopher %d is eating for %d seconds (%d)\n", id, temp, eating);
                    eating += temp;
                    sleep(temp);

                    // if error occurs when put down chopstick on th right
                    if (semop(chopsticks[(id + 1) % 5], putDown, 1) < 0)
                        write(2, strerror(errno), strlen(strerror(errno)));

                    // if error occurs when put down chopstick on th left
                    if (semop(chopsticks[id % 5], putDown, 1) < 0)
                        write(2, strerror(errno), strlen(strerror(errno)));

                    // after eating at least 100 seconds, terminate
                    if (eating >= 20) {
                        printf("Philosopher %d ate for %d seconds and thought for %d seconds, over %d cycles.\n", id, eating, thinking, cycle);
                        exit(1);
                    }

                    temp = randomGaussian(11, 7);
                    if (temp < 0)
                        temp = 0;
                    printf("Philosopher %d is thinking for %d seconds (%d)\n", id, temp, thinking);
                    thinking += temp;
                    sleep(temp);
                    cycle ++;
                }

                else if ( a == 0 || b == 0) {  // when picked only one chopstick
                    if (a < 0)  // put down the left chopstick
                        semop(chopsticks[id % 5], putDown, 1);
                    if (b < 0)  // right chopstick
                        semop(chopsticks[(id + 1) % 5], putDown, 1);
                }
            }
        }
        id++;
    }

    // wait for all children to finish
    for (int i = 0; i < 5; i++) {
        wait(0);
    }

    // remove each semaphore, chopsticks
    for (int i = 0; i < 5; i++) {
        if (semctl(chopsticks[i], 1, IPC_RMID) < 0)
            write(2, strerror(errno), strlen(strerror(errno)));;
    }
	return 0;
}

int randomGaussian(int mean, int stddev) {
    double mu = 0.5 + (double) mean;
    double sigma = fabs((double) stddev);
    double f1 = sqrt(-2.0 * log((double) rand() / (double) RAND_MAX));
    double f2 = 2.0 * 3.14159265359 * (double) rand() / (double) RAND_MAX;
    if (rand() & (1 << 5))
        return (int) floor(mu + sigma * cos(f2) * f1);
    else
        return (int) floor(mu + sigma * sin(f2) * f1);
}
