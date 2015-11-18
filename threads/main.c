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
sem_t *for_empty_buffer_in_counter;
sem_t *for_full_buffer_in_counter;
sem_t *for_empty_buffer_in_encryptor;
sem_t *for_full_buffer_in_encryptor;


buffer_t *buffer_out;
//////////////////////////////buffer_out semaphores///////////////////////////////////////////
sem_t *for_empty_buffer_out_counter;
sem_t *for_full_buffer_out_counter;
sem_t *for_empty_buffer_out_writer;
sem_t *for_full_buffer_out_writer;

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
    printf("%s\r\n", input_file);
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

        sem_wait(for_empty_buffer_in_counter);
        sem_wait(for_empty_buffer_in_encryptor);
        /////////////////CRITICAL CODE/////////////////////////////////////
        buffer_in->data[pos] = ch;
        pos = (pos + 1) % buffer_in->buffer_max_size;
        /////////////////CRITICAL CODE/////////////////////////////////////
        sem_post(for_full_buffer_in_counter);//once for the counter
        sem_post(for_full_buffer_in_encryptor);//another for the encrypt
    }
    fclose(fp);
    return NULL;//TODO
}


void *encrypt_buffer(void *unused_arg)
{
    int s = 1;
    int pos_o = 0;
    int pos_i = 0;
    char ch = '\0';
    while (EOF != ch)
    {
        sem_wait(for_full_buffer_in_encryptor);
        /////////////////CRITICAL CODE TO READ char/////////////////////////////////////
        ch = buffer_in->data[pos_i];
        pos_i = (pos_i + 1) % buffer_in->buffer_max_size;
        /////////////////CRITICAL CODE TO READ char/////////////////////////////////////
        sem_post(for_empty_buffer_in_encryptor);


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
        sem_wait(for_empty_buffer_out_counter);
        sem_wait(for_empty_buffer_out_writer);
        /////////////////CRITICAL CODE TO transfer to buffer_out /////////////////////////////////////
        buffer_out->data[pos_o] = ch;
        pos_o = (pos_o + 1) % buffer_out->buffer_max_size;
        /////////////////CRITICAL CODE TO transfer to buffer_out /////////////////////////////////////
        sem_post(for_full_buffer_out_counter);
        sem_post(for_full_buffer_out_writer);
    }

    return NULL;//TODO
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

    return NULL;//TODO
}

void *write_buffer_out_to_file(void *file_loc)
{

    char temp[999];
    int t = 0;
    char ch = '\0';
    int pos = 0;
    while (EOF != ch)
    {
        sem_wait(for_full_buffer_out_writer);
        /////////////////CRITICAL CODE/////////////////////////////////////
        ch = buffer_out->data[pos];
        pos = (pos + 1) % buffer_out->buffer_max_size;
        /////////////////CRITICAL CODE/////////////////////////////////////
        sem_post(for_empty_buffer_out_writer);
        temp[t++] = ch;
    }

    temp[t - 1] = '\0';//overwrite the EOF char
    printf("psuedowrite:\r\n+++%s+++\r\n", temp);

    return NULL;//NULL

}

//#################################################################################################

void printArgs(int argc, char *argv[])
{
    int i = 0;
    for (i = 0; i < argc; i++)
        printf("%s\r\n", argv[i]);
    printf("-----------------------------\r\n");
}


void create_semaphores(unsigned int max_buffer_size)
{
    //////////////////////////////buffer_in semaphores///////////////////////////////////////////
    for_empty_buffer_in_counter = malloc(sizeof(sem_t));
    for_full_buffer_in_counter = malloc(sizeof(sem_t));
    sem_init(for_empty_buffer_in_counter, 0, max_buffer_size);
    sem_init(for_full_buffer_in_counter, 0, 0);

    for_empty_buffer_in_encryptor = malloc(sizeof(sem_t));
    for_full_buffer_in_encryptor = malloc(sizeof(sem_t));
    sem_init(for_empty_buffer_in_encryptor, 0, max_buffer_size);
    sem_init(for_full_buffer_in_encryptor, 0, 0);



    //////////////////////////////buffer_out semaphores///////////////////////////////////////////
    for_empty_buffer_out_counter = malloc(sizeof(sem_t));
    for_full_buffer_out_counter = malloc(sizeof(sem_t));
    sem_init(for_empty_buffer_out_counter, 0, max_buffer_size);
    sem_init(for_full_buffer_out_counter, 0, 0);

    for_empty_buffer_out_writer = malloc(sizeof(sem_t));
    for_full_buffer_out_writer = malloc(sizeof(sem_t));
    sem_init(for_empty_buffer_out_writer, 0, max_buffer_size);
    sem_init(for_full_buffer_out_writer, 0, 0);

    ///////////////////////////print order semaphore/////////////////////////////
    /**
     * so that the input/output char counters take turns printing their results
     * input results are first
     */
    print_order_mutex = malloc(sizeof(sem_t));
    sem_init(print_order_mutex, 0, 0);

}


#if 0
/**
 * Struct to hold all the threads
 */
typedef struct thread_struct
{
    pthread_t reader_thread;
    pthread_t input_count_thread;
    pthread_t encryption_thread;
    pthread_t output_count_thread;
    pthread_t writer_thread;
} project_threads_t;
#endif


void create_threads(pthread_t *reader_thread, pthread_t *input_count_thread, pthread_t *encryption_thread,
                    pthread_t *output_count_thread, pthread_t *writer_thread, count_buffer_args_t *count_buffer_in_args,
                    count_buffer_args_t *count_buffer_out_args, char *input_file)
{
    if (pthread_create(reader_thread, NULL, read_into_buffer, input_file))
    {
        printf("ERROR creating reader thread\r\n");
        exit(1);
    }

    if (pthread_create(input_count_thread, NULL, count_buffer_chars, count_buffer_in_args))
    {
        printf("ERROR creating input counter thread\r\n");
        exit(1);
    }

    if (pthread_create(encryption_thread, NULL, encrypt_buffer, NULL))
    {
        printf("ERROR creating encryption counter thread\r\n");
        exit(1);
    }

    if (pthread_create(output_count_thread, NULL, count_buffer_chars, count_buffer_out_args))
    {
        printf("ERROR creating input counter thread\r\n");
        exit(1);
    }

    if (pthread_create(writer_thread, NULL, write_buffer_out_to_file, NULL))
    {
        printf("ERROR creating file writer thread\r\n");
        exit(1);
    }

}

void join_threads(pthread_t *reader_thread, pthread_t *input_count_thread, pthread_t *encryption_thread,
                  pthread_t *output_count_thread, pthread_t *writer_thread)
{
    if (pthread_join(*reader_thread, NULL))
    {
        printf("ERROR joining reader thread \n");
    }

    if (pthread_join(*input_count_thread, NULL))
    {
        printf("ERROR joining input counter thread\n");
    }

    if (pthread_join(*encryption_thread, NULL))
    {
        printf("ERROR joining encryption thread\n");
    }

    if (pthread_join(*output_count_thread, NULL))
    {
        printf("ERROR joining output counter thread\n");
    }

    if (pthread_join(*writer_thread, NULL))
    {
        printf("ERROR joining writer thread\n");
    }
}


int main(int argc, char *argv[])
{

    char *input_file = "infile2";
    char *out_file = "myOutfile1";
    int max_buffer_size = 30;
    printf("Buffer Size: %d\r\n", max_buffer_size);

    buffer_in = newBuffer(max_buffer_size);
    buffer_out = newBuffer(max_buffer_size);

    create_semaphores(max_buffer_size);

    count_buffer_args_t count_buffer_in_args;
    count_buffer_in_args.for_empty_buffer = for_empty_buffer_in_counter;
    count_buffer_in_args.for_full_buffer = for_full_buffer_in_counter;
    count_buffer_in_args.buffer = buffer_in;

    count_buffer_args_t count_buffer_out_args;
    count_buffer_out_args.for_empty_buffer = for_empty_buffer_out_counter;
    count_buffer_out_args.for_full_buffer = for_full_buffer_out_counter;
    count_buffer_out_args.buffer = buffer_out;

    pthread_t reader_thread, input_count_thread, encryption_thread, output_count_thread, writer_thread;
    printf("creating threads\r\n");
    create_threads(&reader_thread, &input_count_thread, &encryption_thread, &output_count_thread, &writer_thread,
                   &count_buffer_in_args, &count_buffer_out_args, input_file);

/*

    if (pthread_create(&reader_thread, NULL, read_into_buffer, input_file))
    {
        printf("ERROR creating reader thread\r\n");
        exit(1);
    }

    count_buffer_args_t count_buffer_in_args;
    count_buffer_in_args.for_empty_buffer = for_empty_buffer_in_counter;
    count_buffer_in_args.for_full_buffer = for_full_buffer_in_counter;
    count_buffer_in_args.buffer = buffer_in;

    if (pthread_create(&input_count_thread, NULL, count_buffer_chars, &count_buffer_in_args))
    {
        printf("ERROR creating input counter thread\r\n");
        exit(1);
    }

    if (pthread_create(&encryption_thread, NULL, encrypt_buffer, NULL))
    {
        printf("ERROR creating encryption counter thread\r\n");
        exit(1);
    }


    count_buffer_args_t count_buffer_out_args;
    count_buffer_out_args.for_empty_buffer = for_empty_buffer_out_counter;
    count_buffer_out_args.for_full_buffer = for_full_buffer_out_counter;
    count_buffer_out_args.buffer = buffer_out;

    if (pthread_create(&output_count_thread, NULL, count_buffer_chars, &count_buffer_out_args))
    {
        printf("ERROR creating input counter thread\r\n");
        exit(1);
    }

    if (pthread_create(&writer_thread, NULL, write_buffer_out_to_file, NULL))
    {
        printf("ERROR creating file writer thread\r\n");
        exit(1);
    }
*/
    join_threads(&reader_thread, &input_count_thread, &encryption_thread, &output_count_thread, &writer_thread);

/*

    if (pthread_join(reader_thread, NULL))
    {
        printf("ERROR joining reader thread \n");
    }

    if (pthread_join(input_count_thread, NULL))
    {
        printf("ERROR joining input counter thread\n");
    }

    if (pthread_join(encryption_thread, NULL))
    {
        printf("ERROR joining encryption thread\n");
    }

    if (pthread_join(output_count_thread, NULL))
    {
        printf("ERROR joining output counter thread\n");
    }

    if (pthread_join(writer_thread, NULL))
    {
        printf("ERROR joining writer thread\n");
    }*/

    ////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    deleteBuffer(buffer_in);
    deleteBuffer(buffer_out);
    free(for_empty_buffer_in_counter);
    free(for_full_buffer_in_counter);
    free(for_empty_buffer_in_encryptor);
    free(for_full_buffer_in_encryptor);

    free(for_empty_buffer_out_counter);
    free(for_full_buffer_out_counter);
    free(for_empty_buffer_out_writer);
    free(for_full_buffer_out_writer);
    pthread_exit(NULL);
}