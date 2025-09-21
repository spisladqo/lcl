#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include "../libbmp/libbmp.h"
#include "common.h"
#include "queue.h"

lcl_queue_t read_queue;
lcl_queue_t fore_queue;
lcl_queue_t write_queue;

pthread_mutex_t read_lock;
pthread_mutex_t fore_lock;
pthread_mutex_t write_lock;

int fore_tasks_ready, write_tasks_ready;
int read_done, fore_done, write_done;
pthread_cond_t fore_cv, write_cv;

static void* reader_job(void* thr_arg) {
    thread_arg_t* arg = thr_arg;
    pthread_mutex_t* read_lock = arg->read_lock;
    pthread_mutex_t* fore_lock = arg->fore_lock;
    read_done = 0;

    while (!read_done) {
        pthread_mutex_lock(read_lock);
        task_t* task = lcl_queue_pop(&read_queue);
        if (!task) {
            read_done = 1;
            pthread_mutex_unlock(read_lock);
            pthread_cond_broadcast(&fore_cv);
            break;
        }
        pthread_mutex_unlock(read_lock);

        bmp_img* src = task->src;
        bmp_img* targ = task->targ;
        char* src_path = task->src_path;
        char* targ_path = task->src_path;

        int ret = bmp_img_read(src, src_path);
        if (ret) {
            printf("could not read img %s to src: error %d\n", src_path, ret);
        }
        ret = bmp_img_read(targ, targ_path);
        if (ret) {
            printf("could not read img %s to targ: error %d\n", targ_path, ret);
        }

        pthread_mutex_lock(fore_lock);
        lcl_queue_push(&fore_queue, task);
        fore_tasks_ready++;
        pthread_mutex_unlock(fore_lock);
        pthread_cond_signal(&fore_cv);
    }
}

static void* foreman_job(void* thr_arg) {
    thread_arg_t* arg = thr_arg;
    pthread_mutex_t* read_lock = arg->read_lock;
    pthread_mutex_t* fore_lock = arg->fore_lock;
    pthread_mutex_t* write_lock = arg->write_lock;
    fore_done = 0;

    while (!fore_done) {
        pthread_mutex_lock(fore_lock);
        while (fore_tasks_ready == 0) {
            pthread_cond_wait(&fore_cv, fore_lock);
        }
        task_t* task = lcl_queue_pop(&fore_queue);
        fore_tasks_ready--;
        pthread_mutex_unlock(fore_lock);
        
        bmp_img* src = task->src;
        bmp_img* targ = task->targ;
        enum lcl_conv_mode mode = task->conv_mode;
        int work_num = arg->workers_num;
        lcl_filter_t* filter = task->filter;

        lcl_app_filter(mode, work_num, filter, src, targ);

        pthread_mutex_lock(write_lock);
        lcl_queue_push(&write_queue, task);
        write_tasks_ready++;
        pthread_mutex_unlock(write_lock);

        pthread_mutex_lock(read_lock);
        pthread_mutex_lock(fore_lock);
        if (fore_tasks_ready <= 0 && read_done) {
            fore_done = 1;
            pthread_cond_broadcast(&write_cv);
        }
        pthread_mutex_unlock(fore_lock);
        pthread_mutex_unlock(read_lock);

        pthread_cond_signal(&write_cv);
    }
}

static void* writer_job(void* thr_arg) {
    thread_arg_t* arg = thr_arg;
    pthread_mutex_t* read_lock = arg->read_lock;
    pthread_mutex_t* fore_lock = arg->fore_lock;
    pthread_mutex_t* write_lock = arg->write_lock;
    write_done = 0;

    while (!write_done) {
        pthread_mutex_lock(write_lock);
        while (write_tasks_ready == 0) {
            pthread_cond_wait(&write_cv, write_lock);
        }
        task_t* task = lcl_queue_pop(&write_queue);
        write_tasks_ready--;
        pthread_mutex_unlock(write_lock);
        
        pthread_mutex_lock(fore_lock);
        pthread_mutex_lock(write_lock);
        if (write_tasks_ready <= 0 && fore_done) {
            write_done = 1;
        }
        pthread_mutex_unlock(write_lock);
        pthread_mutex_unlock(fore_lock);

        bmp_img* targ = task->targ;
        char* targ_path = task->targ_path;
        int ret = bmp_img_write(targ, targ_path);
        if (ret) {
            printf("could not write img %s to src: error %d\n", targ_path, ret);
        }

    }
}

int lcl_conv_array(char** src_paths, char** targ_paths, enum lcl_conv_mode* modes,
                    lcl_filter_t** filters, int img_num, thread_jobs_t jobs) {
    if (img_num < 0 || img_num > MAX_IMG_NUM) {
        printf("img_num should be in range [0; %d]\n", MAX_IMG_NUM);
        return LCL_INVALID_ARGUMENT;
    }

    if (!src_paths || !targ_paths || !modes || !filters) {
        printf("array parameters should not be NULL\n");
        return LCL_INVALID_ARGUMENT;
    }

    bmp_img src[img_num], targ[img_num];

    // maybe readers should do this
    for (int i = 0; i < img_num; i++) {
        bmp_img_init_df(&src[i], 0, 0);
        bmp_img_init_df(&targ[i], 0, 0);
    }

    int readers_num = jobs.readers_num;
    int foremen_num = jobs.foremen_num;
    int workers_num = jobs.workers_num;
    int writers_num = jobs.writers_num;

    if (readers_num > img_num) {
        printf("readers num cannot exceed image num\n");
        return LCL_INVALID_ARGUMENT;
    }
    if (foremen_num > img_num) {
        printf("foremen num cannot exceed image num\n");
        return LCL_INVALID_ARGUMENT;
    }
    if (writers_num > img_num) {
        printf("writers num cannot exceed image num\n");
        return LCL_INVALID_ARGUMENT;
    }

    pthread_t readers[readers_num];
    pthread_t foremen[foremen_num];
    pthread_t writers[writers_num];
    task_t tasks[img_num];

    thread_arg_t read_args[readers_num];
    thread_arg_t fore_args[foremen_num];
    thread_arg_t write_args[writers_num];

    pthread_mutex_t read_lock;
    pthread_mutex_t fore_lock;
    pthread_mutex_t write_lock;

    for (int i = 0; i < img_num; i++) {
        task_t task = {
            .src = &src[i],
            .targ = &targ[i],
            .src_path = src_paths[i],
            .targ_path = targ_paths[i],
            .filter = filters[i],
            .conv_mode = modes[i],
        };
        tasks[i] = task;
        lcl_queue_push(&read_queue, &tasks[i]);
    }

    pthread_mutex_init(&read_lock, NULL);
    pthread_mutex_init(&fore_lock, NULL);
    pthread_mutex_init(&write_lock, NULL);

    pthread_cond_init(&fore_cv, NULL);
    pthread_cond_init(&write_cv, NULL);

    for (int i = 0; i < readers_num; i++) {
        thread_arg_t arg = {
            .read_lock = &read_lock,
            .fore_lock = &fore_lock,
        };
        read_args[i] = arg;
        pthread_create(&readers[i], NULL, reader_job, &read_args[i]);
    }

    for (int i = 0; i < foremen_num; i++) {
        thread_arg_t arg = {
            .read_lock = &read_lock,
            .fore_lock = &fore_lock,
            .write_lock = &write_lock,
            .workers_num = workers_num,
        };
        fore_args[i] = arg;
        pthread_create(&foremen[i], NULL, foreman_job, &fore_args[i]);
    }

    for (int i = 0; i < writers_num; i++) {
        thread_arg_t arg = {
            .read_lock = &read_lock,
            .fore_lock = &fore_lock,
            .write_lock = &write_lock,
        };
        write_args[i] = arg;
        pthread_create(&writers[i], NULL, writer_job, &write_args[i]);
    }


    for (int i = 0; i < readers_num; i++) {
        void* ret;
        pthread_join(readers[i], &ret);
    }

    for (int i = 0; i < foremen_num; i++) {
        void* ret;
        pthread_join(foremen[i], &ret);
    }

    for (int i = 0; i < writers_num; i++) {
        void* ret;
        pthread_join(writers[i], &ret);
    }

    pthread_mutex_destroy(&read_lock);
    pthread_mutex_destroy(&fore_lock);
    pthread_mutex_destroy(&write_lock);

    pthread_cond_destroy(&fore_cv);
    pthread_cond_destroy(&write_cv);

    return 0;
}
