#include <math.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int randomGaussian(int mean, int stddev);
void startAction(int* id);
void thinkingTime(int i);

int eating[5] = {0};
int thinking[5] = {0};
int cycle[5] = {0};

pthread_t philosopher[5];
pthread_mutex_t chopsticks[5];

int main(int argc, char *argv[]) {
    int i;
    int phil[5] = {0, 1, 2, 3, 4}; 

    // shared chopsticks
    for (i = 0; i < 5; i++) {
        if (pthread_mutex_init(&chopsticks[i], NULL) != 0) {
            write(2, strerror(errno), strlen(strerror(errno)));
            return (errno);
        };
    }

    // create thread for each philosopher
    for (i = 0; i < 5; i++) {
        if (pthread_create(&philosopher[i], NULL, (void *)startAction, &phil[i]) != 0) {
            write(2, strerror(errno), strlen(strerror(errno)));
            return (errno);
        };
    }

    // wait for all 5 philosophers
    for (i = 0; i < 5; i++) {
        if (pthread_join(philosopher[i], NULL) != 0) {
            write(2, strerror(errno), strlen(strerror(errno)));
            return (errno);
        };
    }

    // kill threads
    for (i = 0; i < 5; i++) {
        if (pthread_mutex_destroy(&chopsticks[i]) != 0) {
            write(2, strerror(errno), strlen(strerror(errno)));
        };
    }
    return 0;
}

void thinkingTime(int id) {
    int temp = randomGaussian(11, 7);
    if (temp < 0)
        temp = 0;
    printf("Philosopher %d is thinking for %d seconds (%d)\n", id, temp, thinking[id]);
    thinking[id] += temp;
    sleep(temp);
}

void startAction(int *id) {
    int philId, temp, leftChopstick, rightChopstick;
    int first_thinking = 1;

    philId = *id;

    thinkingTime(philId);

    while (eating[philId] < 100) {

        // try to lock left and right chopstick
        leftChopstick = pthread_mutex_trylock(&chopsticks[philId % 5]);
        rightChopstick = pthread_mutex_trylock(&chopsticks[(philId + 1) % 5]);

        if (leftChopstick == 0 && rightChopstick == 0) { // picked up two chopsticks
            temp = randomGaussian(9, 3);
            if (temp < 0)
                temp = 0;
            printf("Philosopher %d is eating for %d seconds (%d)\n", philId, temp, eating[philId]);
            eating[philId] += temp;
            sleep(temp);
            cycle[philId] ++;

            // if error occurs when put down chopstick on th right
            if (pthread_mutex_unlock(&chopsticks[philId % 5]) < 0)
                write(2, strerror(errno), strlen(strerror(errno)));

            // if error occurs when put down chopstick on th right
            if (pthread_mutex_unlock(&chopsticks[(philId + 1) % 5]) < 0)
                write(2, strerror(errno), strlen(strerror(errno)));

            // after eating at least 100 seconds, terminate
            if (eating[philId] >= 20) {
                printf("Philosopher %d ate for %d seconds and thought for %d seconds, over %d cycles.\n", philId, eating[philId], thinking[philId], cycle[philId]);
                return;
            }

            thinkingTime(philId);
        }

        // picked only one chopstick
        else if (leftChopstick == 0 || rightChopstick == 0) {
            if (rightChopstick != 0)  // put down the left chopstick
                pthread_mutex_unlock(&chopsticks[philId % 5]);
            if (leftChopstick != 0)  // right chopstick
                pthread_mutex_unlock(&chopsticks[(philId + 1) % 5]);
        }
    }
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
