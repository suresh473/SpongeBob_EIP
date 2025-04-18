/*
 Thread pooling: A thread pool is a group of pre-instantiated, idle threads which stand ready to be given work.
 These are preferred over instantiating new threads for each task when there is a large number of short tasks
 to be done rather than a small number of long ones. This prevents having to incur the overhead of creating a
 thread a large number of times.
*/

/*
 Thread pool implementation
*/

#define MAX_NUM_THREADS     4
#define MAX_QUEUE_SIZE      10

typedef struct {
    void (*func)(void *);
    void* arg;
}TPTask;

typedef struct {
    pthread_mutex_t tp_mutex;
    pthread_cond_t tp_cond;
    pthread_t* tp_threads;
    TPTask tp_task[MAX_QUEUE_SIZE];
    int head;
    int tail;
    int que_size;
    bool stop_process;
}ThreadPool_t;

void *worker_thread(void* arg) {

    ThreadPool_t* tp = (ThreadPool_t *)arg;
    
    while(true) {
        pthread_mutex_lock(&tp->tp_mutex);
        while(tp->que_size == 0 && !tp->stop_process) {
            pthread_cond_wait(&tp->tp_cond, &tp->tp_mutex);
        }

        if(tp->que_size == 0 && tp->stop_process == true) {
            pthread_mutex_unlock(&tp->tp_mutex);
            break;
        }
        TPTask task;
        task.func = tp->tp_task[tp->head].func;
        task.arg = tp->tp_task[tp->head].arg;
        tp->head = (tp->head + 1) % MAX_QUEUE_SIZE;
        --tp->que_size;
        pthread_mutex_unlock(&tp->tp_mutex);

        task.func(task.arg);
    }
    return NULL;
}

int add_task(ThreadPool_t* tp, void (*func)(void *), void* arg) {

    pthread_mutex_lock(&tp->tp_mutex);
    if(tp->que_size == MAX_QUEUE_SIZE) {
        pthread_mutex_unlock(&tp->tp_mutex);
        return -1;
    }

    tp->tp_task[tp->tail].func = func;
    tp->tp_task[tp->tail].arg = arg;
    tp->tail = (tp->tail + 1) % MAX_QUEUE_SIZE;
    ++tp->que_size;
    pthread_cond_signal(&tp->tp_cond);
    pthread_mutex_unlock(&tp->tp_mutex);
    return 0;

}

void worker_func(void* num) {
    printf("In worker func, arg %d \n", *(int *)num);
    usleep(10000);
    return;
}

void init_tp(ThreadPool_t* tp) {
    pthread_mutex_init(&tp->tp_mutex, NULL);
    pthread_cond_init(&tp->tp_cond, NULL);

    tp->tp_threads = (pthread_t *)malloc(sizeof(pthread_t) * MAX_NUM_THREADS);
    for(int i = 0; i < MAX_NUM_THREADS; ++i) {
        pthread_create(&tp->tp_threads[i], NULL, worker_thread, tp);
    }
    tp->head = tp->tail = tp->que_size = 0;
    tp->stop_process = false;
    printf("Init tp successful \n");
    return;
}

void destroy_tp(ThreadPool_t* tp) {
    pthread_mutex_lock(&tp->tp_mutex);
    tp->stop_process = true;
    pthread_cond_broadcast(&tp->tp_cond);
    pthread_mutex_unlock(&tp->tp_mutex);
    
    for(int i = 0; i < MAX_NUM_THREADS; ++i) {
        pthread_join(tp->tp_threads[i], NULL);
    }
    free(tp->tp_threads);
    pthread_mutex_destroy(&tp->tp_mutex);
    pthread_cond_destroy(&tp->tp_cond);
    return;
}

int main() {
    ThreadPool_t* thread_pool = (ThreadPool_t *)malloc(sizeof(ThreadPool_t));
    init_tp(thread_pool);
    
    for(int i = 0; i < 20; ++i) {
        int *arg = malloc(sizeof(int));
        *arg = i;
        int ret = add_task(thread_pool, worker_func, arg);
        if(ret < 0) {
            printf("Queue is full \n");
            free(arg);
        }
        usleep(5000);
    }
    printf("Enqueued all the tasks \n");

    destroy_tp(thread_pool);
    free(thread_pool);
    return 0;
}
