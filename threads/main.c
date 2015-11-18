#include <stdio.h>      /* printf, scanf, NULL, FILE , stdin, stdout, stderr*/
#include <stdlib.h>     /* malloc, free, rand */
#include <pthread.h>
#include <ctype.h> /* isalpha */
#include <semaphore.h>

typedef struct buffer_struct
{
    char *data;
    unsigned int buffer_max_size;

} buffer_t;

buffer_t *newBuffer(unsigned buffer_max_size)
{
    buffer_t *buffer = (buffer_t *) malloc(sizeof(buffer_t));
    buffer->buffer_max_size = buffer_max_size;
    buffer->data = malloc(sizeof(char) * buffer_max_size);
    return buffer;
}

void deleteBuffer(buffer_t *buffer)
{
    free(buffer->data);
    free(buffer);
}

//#################################################################################################

buffer_t *buffer_in;
//////////////////////////////buffer_in semaphores///////////////////////////////////////////
sem_t *sem_bin_counter_empty;
sem_t *sem_bin_counter_full;
sem_t *sem_bin_encryptor_empty;
sem_t *sem_bin_in_encryptor_full;


buffer_t *buffer_out;
//////////////////////////////buffer_out semaphores///////////////////////////////////////////
sem_t *sem_bout_counter_empty;
sem_t *sem_bout_counter_full;
sem_t *sem_bout_writer_empty;
sem_t *sem_bout_writer_full;

sem_t *print_order_mutex;
//#################################################################################################

void *read_into_buffer(void *cstr_input_file)
{
    char *input_file = (char *) cstr_input_file;

    FILE *fp = fopen(input_file, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Can't open file %s!\n", input_file);
        exit(1);
    }
    int ch = '\0';

    int pos = 0;
    while (EOF != ch)
    {
        ch = fgetc(fp);

        /**
         * read_into_buffer is the producer for the buffer_in
         * which has two consumers...
         *
         * the buffer_in_counter
         * the buffer_in_encryptor
         */

        sem_wait(sem_bin_counter_empty);
        sem_wait(sem_bin_encryptor_empty);
        /////////////////CRITICAL CODE/////////////////////////////////////
        buffer_in->data[pos] = ch;
        pos = (pos + 1) % buffer_in->buffer_max_size;
        /////////////////CRITICAL CODE/////////////////////////////////////
        sem_post(sem_bin_counter_full);//once for the counter
        sem_post(sem_bin_in_encryptor_full);//another for the encrypt
    }
    fclose(fp);
    pthread_exit(NULL);
}


void *encrypt_buffer(void *unused_arg)
{
    int s = 1;
    int pos_o = 0;
    int pos_i = 0;
    char ch = '\0';
    while (EOF != ch)
    {
        sem_wait(sem_bin_in_encryptor_full);
        /////////////////CRITICAL CODE TO READ char/////////////////////////////////////
        ch = buffer_in->data[pos_i];
        pos_i = (pos_i + 1) % buffer_in->buffer_max_size;
        /////////////////CRITICAL CODE TO READ char/////////////////////////////////////
        sem_post(sem_bin_encryptor_empty);


        //Encrypt
        if (isalpha(ch))
        {
            if (s == 1)
            {
                if (!isalpha(++ch))
                    ch -= 26;

                s = -1;
            }
            else if (s == -1)
            {
                if (!isalpha(--ch))
                    ch += 26;

                s = 0;
            }
            else if (s == 0)
            {
                s = 1;
            }
        }


        /**
         * The encryptor also acts as the producer for the buffer_out
         * which has two consumers...
         *
         * the buffer_out_counter
         * the buffer_out_writer
         */
        sem_wait(sem_bout_counter_empty);
        sem_wait(sem_bout_writer_empty);
        /////////////////CRITICAL CODE TO transfer to buffer_out /////////////////////////////////////
        buffer_out->data[pos_o] = ch;
        pos_o = (pos_o + 1) % buffer_out->buffer_max_size;
        /////////////////CRITICAL CODE TO transfer to buffer_out /////////////////////////////////////
        sem_post(sem_bout_counter_full);
        sem_post(sem_bout_writer_full);
    }

    pthread_exit(NULL);
}

/**
 * This struct is used to hold pointers to the
 * arguments needed for the method count_buffer_chars.
 * Because the method is used by the pthread_create,
 * it is only allowed one argument...hence this struct.
 *
 */
typedef struct count_buffer_args_struct
{
    sem_t *for_full_buffer;
    sem_t *for_empty_buffer;
    buffer_t *buffer;

} count_buffer_args_t;

void *count_buffer_chars(void *count_buffer_args_t_ptr)
{
    count_buffer_args_t *args = (count_buffer_args_t *) count_buffer_args_t_ptr;

    int counts[26] = {0};
    int i;

    char ch = '\0';
    int pos = 0;
    while (EOF != ch)
    {
        sem_wait(args->for_full_buffer);
        /////////////////CRITICAL CODE/////////////////////////////////////
        ch = toupper(args->buffer->data[pos]);
        pos = (pos + 1) % args->buffer->buffer_max_size;
        /////////////////CRITICAL CODE/////////////////////////////////////
        sem_post(args->for_empty_buffer);

        i = ch - 'A';
        if (i >= 0 && i < 26)
            counts[i]++;
    }


    //print to stdout
    if (args->buffer == buffer_in)
    {
        printf("Input file contains \r\n");
    }
    else if (args->buffer == buffer_out)
    {
        sem_wait(print_order_mutex);
        printf("Output file contains \r\n");
    }

    for (i = 0; i < 26; i++)
    {
        ch = 'A' + i;
        if (counts[i])
            printf("%c: %d\r\n", ch, counts[i]);
    }
    sem_post(print_order_mutex);

    pthread_exit(NULL);
}

void *write_buffer_out_to_file(void *cstr_file_loc)
{

    FILE *fp = fopen(cstr_file_loc, "w");
    if (fp == NULL)
    {
        fprintf(stderr, "Can't open file %s!\n", (char *) cstr_file_loc);
        exit(1);
    }


    int pos = 0;
    char ch = '\0';
    while (EOF != ch)
    {
        sem_wait(sem_bout_writer_full);
        /////////////////CRITICAL CODE/////////////////////////////////////
        ch = buffer_out->data[pos];
        pos = (pos + 1) % buffer_out->buffer_max_size;
        /////////////////CRITICAL CODE/////////////////////////////////////
        sem_post(sem_bout_writer_empty);

        if(EOF!=ch)
            fputc(ch, fp);
    }

    fclose(fp);
    pthread_exit(NULL);
}

//#################################################################################################




/**
 * Struct to hold all the threads
 */
typedef struct thread_struct
{
    pthread_t *reader_thread;
    pthread_t *input_count_thread;
    pthread_t *encryption_thread;
    pthread_t *output_count_thread;
    pthread_t *writer_thread;
} project_threads_t;


void create_and_start_threads(project_threads_t *threads, count_buffer_args_t *count_buffer_in_args, count_buffer_args_t *count_buffer_out_args, char *input_file, char *output_file)
{

    threads->reader_thread = malloc(sizeof(pthread_t));
    threads->input_count_thread = malloc(sizeof(pthread_t));
    threads->encryption_thread = malloc(sizeof(pthread_t));
    threads->output_count_thread = malloc(sizeof(pthread_t));
    threads->writer_thread = malloc(sizeof(pthread_t));


    if (pthread_create(threads->reader_thread, NULL, read_into_buffer, input_file))
    {
        printf("ERROR creating reader thread\r\n");
        exit(1);
    }

    if (pthread_create(threads->input_count_thread, NULL, count_buffer_chars, count_buffer_in_args))
    {
        printf("ERROR creating input counter thread\r\n");
        exit(1);
    }

    if (pthread_create(threads->encryption_thread, NULL, encrypt_buffer, NULL))
    {
        printf("ERROR creating encryption counter thread\r\n");
        exit(1);
    }

    if (pthread_create(threads->output_count_thread, NULL, count_buffer_chars, count_buffer_out_args))
    {
        printf("ERROR creating input counter thread\r\n");
        exit(1);
    }

    if (pthread_create(threads->writer_thread, NULL, write_buffer_out_to_file,output_file))
    {
        printf("ERROR creating file writer thread\r\n");
        exit(1);
    }
}


void join_and_free_threads(project_threads_t *threads)
{
    if (pthread_join(*(threads->reader_thread), NULL))
    {
        printf("ERROR joining reader thread \n");
    }

    if (pthread_join(*(threads->input_count_thread), NULL))
    {
        printf("ERROR joining input counter thread\n");
    }

    if (pthread_join(*(threads->encryption_thread), NULL))
    {
        printf("ERROR joining encryption thread\n");
    }

    if (pthread_join(*(threads->output_count_thread), NULL))
    {
        printf("ERROR joining output counter thread\n");
    }

    if (pthread_join(*(threads->writer_thread), NULL))
    {
        printf("ERROR joining writer thread\n");
    }

    free(threads->reader_thread);
    free(threads->input_count_thread);
    free(threads->encryption_thread);
    free(threads->output_count_thread);
    free(threads->writer_thread);
}

void create_and_setup_semaphores(unsigned int max_buffer_size)
{
    //////////////////////////////buffer_in semaphores///////////////////////////////////////////
    sem_bin_counter_empty = malloc(sizeof(sem_t));
    sem_bin_counter_full = malloc(sizeof(sem_t));
    sem_init(sem_bin_counter_empty, 0, max_buffer_size);
    sem_init(sem_bin_counter_full, 0, 0);

    sem_bin_encryptor_empty = malloc(sizeof(sem_t));
    sem_bin_in_encryptor_full = malloc(sizeof(sem_t));
    sem_init(sem_bin_encryptor_empty, 0, max_buffer_size);
    sem_init(sem_bin_in_encryptor_full, 0, 0);



    //////////////////////////////buffer_out semaphores///////////////////////////////////////////
    sem_bout_counter_empty = malloc(sizeof(sem_t));
    sem_bout_counter_full = malloc(sizeof(sem_t));
    sem_init(sem_bout_counter_empty, 0, max_buffer_size);
    sem_init(sem_bout_counter_full, 0, 0);

    sem_bout_writer_empty = malloc(sizeof(sem_t));
    sem_bout_writer_full = malloc(sizeof(sem_t));
    sem_init(sem_bout_writer_empty, 0, max_buffer_size);
    sem_init(sem_bout_writer_full, 0, 0);

    ///////////////////////////print order semaphore/////////////////////////////
    /**
     * semaphore so that the input/output char counters take turns printing their results
     * (input char counts should be printed first)
     */
    print_order_mutex = malloc(sizeof(sem_t));
    sem_init(print_order_mutex, 0, 0);

}

void free_semaphores()
{
    free(sem_bin_counter_empty);
    free(sem_bin_counter_full);
    free(sem_bin_encryptor_empty);
    free(sem_bin_in_encryptor_full);

    free(sem_bout_counter_empty);
    free(sem_bout_counter_full);
    free(sem_bout_writer_empty);
    free(sem_bout_writer_full);

    free(print_order_mutex);
}

unsigned prompt_for_buffer_size()
{
    int buffer=0;
    while(buffer<=0)
    {
        printf("Enter buffer size: ");
        scanf("%d", &buffer);
    }
    return buffer;
}

int main(int argc, char *argv[])
{

    char *format = "Please enter the correct command format: encrypt inputfile outputfile\r\n";
    if (argc < 3)
    {
        printf("%s", format);
        return -1;
    }

    char *input_file = argv[1];
    char *out_file = argv[2];


    int max_buffer_size = prompt_for_buffer_size();

    buffer_in = newBuffer(max_buffer_size);
    buffer_out = newBuffer(max_buffer_size);

    create_and_setup_semaphores(max_buffer_size);

    count_buffer_args_t count_buffer_in_args;
    count_buffer_in_args.for_empty_buffer = sem_bin_counter_empty;
    count_buffer_in_args.for_full_buffer = sem_bin_counter_full;
    count_buffer_in_args.buffer = buffer_in;

    count_buffer_args_t count_buffer_out_args;
    count_buffer_out_args.for_empty_buffer = sem_bout_counter_empty;
    count_buffer_out_args.for_full_buffer = sem_bout_counter_full;
    count_buffer_out_args.buffer = buffer_out;


    project_threads_t threads;

    create_and_start_threads(&threads, &count_buffer_in_args, &count_buffer_out_args, input_file, out_file);

    join_and_free_threads(&threads);


    free_semaphores();

    deleteBuffer(buffer_in);
    deleteBuffer(buffer_out);

    pthread_exit(NULL);
}