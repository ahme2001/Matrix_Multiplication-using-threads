#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>

// struct to store number of rows and columns
typedef struct size_arr size_arr;
struct size_arr
{
    int n_row;
    int n_col;
}size_A , size_B;
// global array to make operation on them.
long arr_A[20][20],arr_B[20][20],arr_C[20][20];
int skip_flag = 0;
// prototype for functions
struct size_arr read_file(char file[50], int flag);
void *matrix_mul();
void *matrix_mul_p_row(void *arg);
void *matrix_mul_p_element(void *arg);
void write_file(int flag,char o_file[50]);

int main(int argc,char* argv[]) {
    pthread_t th_per_matrix;
    struct timeval stop, start;
    char *o_file;

    if(argc == 1) {
        size_A = read_file("a", 1);
        size_B = read_file("b", 2);
        skip_flag = 1;
    }
    else
    {
        o_file = argv[3];
        size_A = read_file(argv[1], 1);
        size_B = read_file(argv[2], 2);
    }

    //check number of col == number of row
    if(size_A.n_col != size_B.n_row)
    {
        printf("You must enter available matrix(number of cols of first must equal number of rows of second)\n");
        exit(1);
    }

    //matrix multiplication by thread per matrix
    gettimeofday(&start, NULL);
    if (pthread_create(&th_per_matrix, NULL, &matrix_mul, NULL) != 0) {
        printf("error");
        return 1;
    }
    pthread_join(th_per_matrix, NULL);
    gettimeofday(&stop,NULL);
    printf("Number of threads : 1\n");
    printf("Seconds taken by thread per matrix: %lu\n",stop.tv_sec-start.tv_sec);
    printf("Microseconds taken by thread per matrix: %lu\n",stop.tv_usec-start.tv_usec);
    write_file(3,o_file);


    //matrix multiplication by thread per row
    gettimeofday(&start, NULL);
    pthread_t th_per_row[size_A.n_row];
    for (int i = 0; i < size_A.n_row; i++) {
        if (pthread_create(&th_per_row[i], NULL, &matrix_mul_p_row, (void *) i) != 0) {
            printf("error");
            return 1;
        }
    }
    for (int i = 0; i < size_A.n_row; i++) {
        pthread_join(th_per_row[i], NULL);
    }
    gettimeofday(&stop,NULL);
    printf("Number of threads: %d\n",size_A.n_row);
    printf("Seconds taken by thread per row: %lu\n",stop.tv_sec-start.tv_sec);
    printf("Microseconds taken by thread per row: %lu\n",stop.tv_usec-start.tv_usec);
    write_file(2,o_file);


    //matrix multiplication by thread per element
    gettimeofday(&start, NULL);
    pthread_t th_per_element[size_A.n_row][size_B.n_col];
    for (int i = 0; i < size_A.n_row; i++) {
        for (int j = 0; j < size_B.n_col; j++) {
            struct size_arr *temp = malloc(sizeof(struct size_arr));
            temp->n_row = i;
            temp->n_col = j;
            if (pthread_create(&th_per_element[i][j], NULL, matrix_mul_p_element, temp) != 0) {
                printf("error");
                return 1;
            }

        }
    }
    for (int i = 0; i < size_A.n_row; i++) {
        for (int j = 0; j < size_B.n_col; j++) {
            pthread_join(th_per_element[i][j], NULL);
        }
    }
    gettimeofday(&stop,NULL);
    printf("Number of threads: %d\n",(size_A.n_row*size_B.n_col));
    printf("Seconds taken by thread per element: %lu\n",stop.tv_sec-start.tv_sec);
    printf("Microseconds taken by thread per element: %lu\n",stop.tv_usec-start.tv_usec);
    write_file(1,o_file);

    return 0;
}
struct size_arr read_file(char file[50],int flag)
{
    char r_file[50] = "";
    struct size_arr test;
    FILE *file1;
    strcat(r_file,file);
    strcat(r_file,".txt");
    file1 = fopen(r_file, "r");
    if (file1 == NULL){
        perror("ERROR");
        exit(1);
    }
    char line[100];
    int i=0;
    // get number of row and columns from file
    fgets(line, sizeof(line) , file1);
    char* row = strtok(line," ");
    char * col = strtok(NULL," ");
    row = strtok(row,"row=");
    test.n_row = atoi(row);
    col = strtok(col,"col=");
    test.n_col = atoi(col);
    // store number written in file in global array to do operation on it.
    while (fgets(line, sizeof(line) , file1))
    {
        char * num = strtok(line,"\t");
        int j=0;
        while (num != NULL)
        {
            if(flag == 1)
                arr_A[i][j++] = atoi(num);
            else
                arr_B[i][j++] = atoi(num);
            num = strtok(NULL,"\t");
        }
        i++;
    }
    fclose(file1);
    return test;
}
// multiply two matrices
void *matrix_mul()
{
    long sum =0;
    for (int i = 0; i < size_A.n_row; ++i) {
        for (int j = 0; j < size_B.n_col; ++j) {
            for (int k = 0; k < size_A.n_col; k++) {
                sum += (arr_A[i][k] * arr_B[k][j]);
            }
            arr_C[i][j] = sum;
            sum = 0;
        }
    }
}
// multiply row by another matrix to make one row
void *matrix_mul_p_row(void *arg)
{
    int i;
    i = (int)arg;
    for (int j = 0; j < size_B.n_col; ++j) {
        for (int k = 0; k < size_A.n_col; k++) {
            arr_C[i][j] += (arr_A[i][k] * arr_B[k][j]);
        }
    }
}
// multiply row by column to give one element
void *matrix_mul_p_element(void *arg)
{
    struct size_arr *te = (struct size_arr*) arg;
    for (int k = 0; k < size_A.n_col; k++) {
        arr_C[te->n_row][te->n_col] += (arr_A[te->n_row][k] * arr_B[k][te->n_col]);
    }
    free(te);
}
// write result of multiplication in files
void write_file(int flag, char o_file[50])
{
    FILE *result;
    char w_file[50]="";
    if (flag == 1){
        if(skip_flag == 1)
            result = fopen("c_per_element.txt", "w");
        else
        {
            strcat(w_file,o_file);
            strcat(w_file,"_per_element.txt");
            result = fopen(w_file, "w");
        }
        fprintf(result,"Method: A thread per element\n");
    }else if(flag == 2){
        if(skip_flag == 1)
            result = fopen("c_per_row.txt", "w");
        else
        {
            strcat(w_file,o_file);
            strcat(w_file,"_per_row.txt");
            result = fopen(w_file, "w");
        }
        fprintf(result,"Method: A thread per row\n");
    }else{
        if(skip_flag == 1)
            result = fopen("c_per_matrix.txt", "w");
        else
        {
            strcat(w_file,o_file);
            strcat(w_file,"_per_matrix.txt");
            result = fopen(w_file, "w");
        }
        fprintf(result,"Method: A thread per matrix\n");
    }
    fprintf(result,"row=%d col=%d\n",size_A.n_row,size_B.n_col);
    for (int i = 0; i < size_A.n_row; i++) {
        for (int j = 0; j < size_B.n_col; ++j) {
            fprintf(result, "%ld ", arr_C[i][j]);
        }
        fprintf(result,"\n");
    }
    fclose(result);
    // empty array of result to be available for next operation
    memset(arr_C, 0, sizeof(arr_C[0][0]) * 20 * 20);
}