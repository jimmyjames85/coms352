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

void print_buffer(buffer_t *buffer)
{
    int i = 0;
    for (i = 0; i < buffer->buffer_max_size; i++)
    {
        if (EOF == buffer->data[i])
            return;
        printf("%c", buffer->data[i]);
    }
}

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
    //llfreefree(buffer->ll_data);
    free(buffer);
}

//#################################################################################################


buffer_t *buffer_in;
//////////////////////////////buffer_in semaphores///////////////////////////////////////////
sem_t *for_empty_buffer_in_counter;
sem_t *for_full_buffer_in_counter;
sem_t *for_empty_buffer_in_encryptor;
sem_t *for_full_buffer_in_encryptor;
//////////////////////////////buffer_in semaphores///////////////////////////////////////////

buffer_t *buffer_out;
//////////////////////////////buffer_out semaphores///////////////////////////////////////////
sem_t *for_empty_buffer_out_counter;
sem_t *for_full_buffer_out_counter;
sem_t *for_empty_buffer_out_writer;
sem_t *for_full_buffer_out_writer;
//////////////////////////////buffer_out semaphores///////////////////////////////////////////

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
        sem_wait(for_empty_buffer_in_counter);
        sem_wait(for_empty_buffer_in_encryptor);
        /////////////////CRITICAL CODE/////////////////////////////////////
        ch = fgetc(fp);
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
        /////////////////CRITICAL CODE/////////////////////////////////////
        ch = buffer_in->data[pos_i];
        pos_i = (pos_i + 1) % buffer_in->buffer_max_size;

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


        if (pos_o >= buffer_out->buffer_max_size)
        {
            //TODO wait on buffer_in
            printf("buffer out beyond max");
            pos_o = 0;
            exit(1);
        }
        buffer_out->data[pos_o++] = ch;


        /////////////////CRITICAL CODE/////////////////////////////////////
        sem_post(for_empty_buffer_in_encryptor);
    }

    return NULL;//TODO
}


void *count_buffer_in(void *sem_t_arr_1st_full_2nd_is_empty)
{
    int counts[26] = {0};
    int i;

    char ch = '\0';
    int pos = 0;
    while (EOF != ch)
    {
        sem_wait(for_full_buffer_in_counter);
        /////////////////CRITICAL CODE/////////////////////////////////////

        ch = toupper(buffer_in->data[pos]);
        pos = (pos + 1) % buffer_in->buffer_max_size;
        i = ch - 'A';
        if (i >= 0 && i < 26)
            counts[i]++;

        /////////////////CRITICAL CODE/////////////////////////////////////
        sem_post(for_empty_buffer_in_counter);
    }

    for (i = 0; i < 26; i++)
    {
        ch = 'A' + i;
        if (counts[i])
            printf("%c: %d\r\n", ch, counts[i]);
    }

    return NULL;//TODO
}
//#################################################################################################

void printArgs(int argc, char *argv[])
{
    int i = 0;
    for (i = 0; i < argc; i++)
        printf("%s\r\n", argv[i]);
    printf("-----------------------------\r\n");
}

int main(int argc, char *argv[])
{

    char *input_file = "infile2";
    char *out_file = "myOutfile1";
    int max_buffer_size = 5;
    printf("Buffer Size: %d\r\n", max_buffer_size);

    buffer_in = newBuffer(max_buffer_size);
    buffer_out = newBuffer(max_buffer_size);

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


    pthread_t reader_thread, input_count_thread, encryption_thread, output_count_thread, writer_thread;


    if (pthread_create(&reader_thread, NULL, read_into_buffer, input_file))
    {
        printf("ERROR creating reader thread\r\n");
        exit(1);
    }

    if (pthread_create(&input_count_thread, NULL, count_buffer_in, buffer_in))
    {
        printf("ERROR creating input counter thread\r\n");
        exit(1);
    }

    if (pthread_create(&encryption_thread, NULL, encrypt_buffer, NULL))
    {
        printf("ERROR creating encryption counter thread\r\n");
        exit(1);
    }









    if (pthread_join(reader_thread, NULL)) // wait for the thread 1 to finish
    {
        printf("ERROR joining reader thread \n");
    }

    if (pthread_join(input_count_thread, NULL)) // wait for the thread 2 to finish
    {
        printf("ERROR joining input counter thread\n");
    }

    if (pthread_join(encryption_thread, NULL)) // wait for the thread 3 to finish
    {
        printf("ERROR joining input counter thread\n");
    }

    ////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////
    print_buffer(buffer_out);
    ////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    deleteBuffer(buffer_in);
    deleteBuffer(buffer_out);
    free(for_empty_buffer_in_counter);
    free(for_full_buffer_in_counter);
    free(for_empty_buffer_in_encryptor);
    free(for_full_buffer_in_encryptor);
    pthread_exit(NULL);
}