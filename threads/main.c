#include <stdio.h>      /* printf, scanf, NULL, FILE , stdin, stdout, stderr*/
#include <stdlib.h>     /* malloc, free, rand */
#include <pthread.h>
#include <ctype.h> /* isalpha */
#include <semaphore.h>



char *newChar(char c)
{
    char *ret = malloc(sizeof(char));
    *ret = c;
    return ret;
}

typedef struct buffer_struct
{
    char * data;
    unsigned int buffer_max_size;

} buffer_t;

void print_buffer(buffer_t *buffer)
{
    int i = 0;
    for (i = 0; i < buffer->buffer_max_size; i++)
    {
        if(EOF==buffer->data[i])
            return;
        printf("%c", buffer->data[i]);
    }
}

buffer_t *newBuffer(unsigned buffer_max_size)
{
    buffer_t *buffer = (buffer_t *) malloc(sizeof(buffer_t));
    buffer->buffer_max_size = buffer_max_size;
    buffer->data = malloc(sizeof(char)*buffer_max_size);
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
buffer_t *buffer_out;
int data_loaded;


//#################################################################################################
void * read_into_buffer(void * cstr_input_file)
{
    char * input_file = (char*)cstr_input_file;
    //printf("%s\r\n",input_file);
    FILE *fp = fopen(input_file, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Can't open file %s!\n", input_file);
        exit(1);
    }

    int pos =0;
    int ch='\0';

    while (EOF!=ch)
    {
        ch = fgetc(fp);
        printf("%c",ch);
        if (pos >= buffer_in->buffer_max_size)
        {
            pos = 0;
            printf("read_buffer_overflow!");
            fclose(fp);
            exit(1);
            //TODO wait and when all consumers have read then remove the head
            //TODO reset pos
        }
        buffer_in->data[pos++]=ch;

    }
    fclose(fp);

    data_loaded=1;
    return NULL;//TODO
}


void * encrypt_buffer(void * arg)
{
    int s = 1;

    int pos_i=0;
    int pos_o=0;

    char ch = '\0';

    while(EOF!=ch)
    {
        if(pos_i>=buffer_in->buffer_max_size)
        {
            //TODO wait on buffer_in
            printf("buffer beyond max");
            pos_i=0;
            exit(1);
        }

        ch = buffer_in->data[pos_i++];

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

        if(pos_o>=buffer_out->buffer_max_size)
        {
            //TODO wait on buffer_in
            printf("buffer out beyond max");
            pos_o=0;
            exit(1);
        }

        buffer_out->data[pos_o++] = ch;
    }
    return NULL;//TODO
}


void * count_buffer(void * buffer_t_ptr)
{
    while(data_loaded==0)
        ;

    printf("dataLoaded=%d\r\n",data_loaded);

    buffer_t *buffer = (buffer_t * ) buffer_t_ptr;
    int pos=0;
    int counts[26]={0};
    int i;
    char ch = '\0';
    while(EOF!=ch)
    {
        if(pos>=buffer->buffer_max_size)
        {
            //TODO wait on buffer_in
            printf("count buffer beyond max");
            pos=0;
            exit(1);
        }

        ch = toupper(buffer->data[pos++]);
        i = ch - 'A';
        if(i>=0 && i<26)
            counts[i]++;
    }

    for(i=0;i<26;i++)
    {
        ch='A'+i;
        if(counts[i])
            printf("%c: %d\r\n", ch, counts[i] );
    }

    data_loaded=2;

    return NULL;//TODO
}
//#################################################################################################

int main(int argc, char *argv[])
{

    char *input_file = "infile1";
    //char *out_file = "outfile1";
    int max_buffer_size = 300;
    data_loaded=0;
    int i=0;
    for(i=0;i<argc;i++)
        printf("%s\r\n",argv[i]);
    printf("-----------------------------\r\n");


    buffer_t *buffer_in = newBuffer(max_buffer_size);
    buffer_t *buffer_out = newBuffer(max_buffer_size);

    pthread_t reader_thread, input_count_thread;//, encryption_thread;

    if(pthread_create(&reader_thread, NULL, read_into_buffer, input_file))
    {
        printf("ERROR creating reader thread\r\n");
        exit(1);
    };

    if(pthread_create(&input_count_thread, NULL, count_buffer, buffer_in))
    {
        printf("ERROR creating input counter thread\r\n");
        exit(1);
    };



    printf("2goodbye\r\n");
    if(pthread_join(reader_thread,NULL)) /* wait for the thread 1 to finish */
    {
        printf("ERROR joining reader thread \n");
    }

    printf("3goodbye\r\n");
    if(pthread_join(input_count_thread,NULL)) /* wait for the thread 2 to finish */
    {
        printf("ERROR joining input counter thread\n");
    }

    printf("4goodbye\r\n");


/*    read_into_buffer(buffer_in, in_file);
    //print_buffer(buffer_in);
    count_buffer(buffer_in);

    encrypt_buffer(buffer_in, buffer_out);
    print_buffer(buffer_out);
    count_buffer(buffer_out);
*/

    deleteBuffer(buffer_in);
    deleteBuffer(buffer_out);
    pthread_exit(NULL);

}