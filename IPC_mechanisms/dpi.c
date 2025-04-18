/*
 Dining philosophers problem:
 Five philosophers sit around a table, each in front of a big plate of spaghetti.
 A single fork (the utensil, not the system call) is placed between neighboring
 Philosophers. Each philosopher comes to the table to think, eat, think, eat, think, and eat.
 That's three square meals of spaghetti after three extended think sessions.
 Each philosopher keeps to themselves as they think. Sometime they think
 for a long time, and sometimes they barely think at all.
 After each philosopher has thought for a while, they proceed to eat one of
 their three daily meals. In order to eat, they must grab hold of two forks—
 one on their left, then one on their right. With two forks in hand, they chow
 on spaghetti to nourish their big, philosophizing brain. When they're full,
 they put down the forks in the same order they picked them up and returns
 to think for a while.
*/

/*
 Dining philosophers implementation
*/

#define NUM_PHILOSOPHERS    5

pthread_mutex_t f_mutex[NUM_PHILOSOPHERS];
pthread_t* phil_threads;
sem_t p_sem;
int num_meals = 3;

void phil_eat() {
    /* Eating time */
    usleep(1000);
    return;
}

void phil_think() {
    /* Thinking time */
    usleep(3000);
    return;
}

void *phil_func(void* index) {

    int idx = *(int *)index;
    for(int i = 0; i < num_meals; ++i) {
        phil_think();
        sem_wait(&p_sem);
        pthread_mutex_lock(&f_mutex[idx]);
        pthread_mutex_lock(&f_mutex[(idx + 1) % NUM_PHILOSOPHERS]);
        phil_eat();
        sem_post(&p_sem);
        pthread_mutex_unlock(&f_mutex[(idx + 1) % NUM_PHILOSOPHERS]);
        pthread_mutex_unlock(&f_mutex[idx]);
    }
    printf("Completed 3 course meal for phil %d \n", idx);
    return NULL;
}

int main() {
    int num_philosophers = NUM_PHILOSOPHERS;
    phil_threads = (pthread_t *)malloc(num_philosophers * sizeof(pthread_t));
    
    sem_init(&p_sem, 0, 4);
    for(int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        int *index = (int *)malloc(sizeof(int));
        *index = i;
        pthread_mutex_init(&f_mutex[i], NULL);
        pthread_create(&phil_threads[i], NULL, phil_func, index);
    }

    sleep(2);
    sem_destroy(&p_sem);
    for(int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        pthread_mutex_destroy(&f_mutex[i]);
        pthread_join(phil_threads[i], NULL);
    }

}
