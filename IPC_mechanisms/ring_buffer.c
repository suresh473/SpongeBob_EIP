/*
 Ring buffer implementation
*/

#define MAX_QUEUE_SIZE      10

typedef struct {
    int buffer[MAX_QUEUE_SIZE];
    int head;
    int tail;
    int size;
    pthread_mutex_t rb_mutex;
}RingBuffer;

pthread_t prod_thread_t;
pthread_t cons_thread_t;

int add_queue(RingBuffer* rb, int val) {

    pthread_mutex_lock(&rb->rb_mutex);
    if(rb->size == MAX_QUEUE_SIZE) {
        printf("Queue is full \n");
        pthread_mutex_unlock(&rb->rb_mutex);
        return -1;
    }
    rb->buffer[rb->head] = val;
    rb->head = (rb->head + 1) % MAX_QUEUE_SIZE;
    rb->size++;
    pthread_mutex_unlock(&rb->rb_mutex);
    return 0;
}

int get_queue(RingBuffer* rb) {

    pthread_mutex_lock(&rb->rb_mutex);
    if(rb->size == 0) {
        printf("Queue is empty \n");
        pthread_mutex_unlock(&rb->rb_mutex);
        return -1;
    }
    int val = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) % MAX_QUEUE_SIZE;
    rb->size--;
    pthread_mutex_unlock(&rb->rb_mutex);
    return val;
}

void *prod_thread(void *arg) {
    RingBuffer* rb = (RingBuffer *)arg;
    
    for(int i = 0; i < 20; ++i) {
        int ret = add_queue(rb, i);
        usleep(10000);
        if(ret < 0) {
            --i;
            continue;
        }
        printf("Added to queue %d \n", i);
    }
    return NULL;
}

void *cons_thread(void *arg) {
    RingBuffer* rb = (RingBuffer *)arg;
    
    for(int i = 0; i < 20; ++i) {
        int ret = get_queue(rb);
        usleep(30000);
        if(ret < 0) {
            --i;
            continue;
        }
        printf("read from queue %d \n", ret);
    }
    return NULL;
}

void init_rb(RingBuffer* rb) {
    rb->head = rb->tail = rb->size = 0;
    pthread_mutex_init(&rb->rb_mutex, NULL);
    return;
}

void destroy_rb(RingBuffer* rb) {
    pthread_mutex_destroy(&rb->rb_mutex);
    return;
}

int main() {
    RingBuffer* rb = (RingBuffer *)malloc(sizeof(RingBuffer));
    
    init_rb(rb);
    pthread_create(&prod_thread_t, NULL, prod_thread, rb);
    pthread_create(&cons_thread_t, NULL, cons_thread, rb);

    pthread_join(prod_thread_t, NULL);
    pthread_join(cons_thread_t, NULL);
    destroy_rb(rb);
    return 0;
}
