#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "/usr/local/Cellar/libomp/10.0.1/include/omp.h"

//Primitives ----
int threshold, max = 1000, width, height;
int nt;
char pbm_file[80];

//Pointers ----
FILE * fileptr;
char * tokens;
int ** pixel_array;
int ** bw_Array;
double start_parallel = 0.0, start_all = 0.0, end_parallel = 0.0, end_all = 0.0, parallel_time = 0.0, all_time = 0.0;

void convertPNGtoPGM(char * input){
    char cmdA[80];
    strcpy(cmdA, "pngtopnm ");
    strcat(cmdA, input);
    strcat(cmdA, " > tmp.pgm");

    char cmdB[80];
    strcpy(cmdB, "ppmtopgm tmp.pgm > ");
    strcat(cmdB, "gray.pgm");

    system(cmdA);
    system(cmdB);
    system("rm tmp.pgm");
}

void parseImg( ){
    fileptr = fopen("gray.pgm", "rb");
    int pixel;
    char buffer[100];
    if(fileptr == NULL){
      printf("Some Error Occurred, Check Permissions\n");
      exit(3);
    }

    int i = 0;

    //Handling Header on Image
    for( i = 0; fgets(buffer, max, fileptr) != NULL; i++){
      if(buffer[0] == '#') {
        i--;
        continue;    //Ignoring comments
      }

      if(i == 0 && buffer[1] != '5')  exit(1);
      if(i == 0)  continue;
      if(i == 2)  break;

      tokens = strtok(buffer, " ");
      width = atoi(tokens);

      tokens = strtok(NULL, " ");
      height = atoi(tokens);
    }

    int j = 0;

    //INIT 2D ARRAY
    pixel_array = (int **)malloc(height * sizeof(int *));
    for(j =0; j < height; j++){
      pixel_array[j] = (int *) malloc(width * sizeof(int));
    }

    int row = 0;
    int col = 0;
    int count = 0;

    //Handling Pixel Data on Image
    for( row = 0, count = 0; row < height; row++) {
      for( col = 0; col < width; col++, count++){
        pixel = fgetc(fileptr);
        pixel_array[row][col] = pixel;
      }
    }

    fclose(fileptr);
    system("rm gray.pgm");
}

void free_arrays(){
  int i =0;
  for( i = 0; i < height; i++){
    free(pixel_array[i]);
    free(bw_Array[i]);
  }
  free(pixel_array);
  free(bw_Array);
}


void detectEdges(){
  start_parallel = omp_get_wtime();
  bw_Array = (int **)malloc(height * sizeof(int *));
  int a = 0;

  #pragma omp parallel for
  for(a =0; a < height; a++){
    bw_Array[a] = (int *) malloc(width * sizeof(int));
  }

  int i = 0;
  int j = 0;

  #pragma omp parallel for private(i,j)
  for(i = 0; i < height; i++){
    for(j = 0; j < width; j++){
      bw_Array[i][j] = 1;
    }
  }

  int colA = 0, rowA = 0;

  #pragma omp parallel for private(colA,rowA)
  for( colA = 0; colA < width; colA++){
    for( rowA = 0; rowA < height-1; rowA++){
      if(abs(pixel_array[rowA][colA] - pixel_array[rowA+1][colA]) > threshold){
        bw_Array[rowA][colA] = 0;
      }
    }
  }

  int row = 0, col = 0;

  #pragma omp parallel for private(row,col)
  for( row = 0; row < height; row++){
    for( col = 0; col < width-1; col++){
      if(abs(pixel_array[row][col] - pixel_array[row][col+1]) > threshold){
        bw_Array[row][col] = 0;
      }
    }
  }

  end_parallel = omp_get_wtime();
  parallel_time = end_parallel - start_parallel;
  //***
}

void printToFile(char * fileName){

  FILE * writeFile = fopen(fileName, "w");
  fprintf(writeFile, "P1\n");
  fprintf(writeFile, "%d %d\n", width, height);

  //fwrite(filename, sizeof(filename), 1, writeFile); //#filename would go here
  //fwrite(newLine, sizeof(newLine), 1, writeFile);
  int row = 0, col = 0;
  for( row = 0; row < height; row++){ //write numbers
    for( col = 0; col < width; col++){
      fprintf(writeFile, "%d ", bw_Array[row][col]);
    }
    fprintf(writeFile, "\n"); //newLine Time
  }
}

int main(int argc, char * argv[]){
  //Usage convertPNGtoPGM <input.png> <output.pgm> threshold
  if(argc != 5){
    printf("Usage: edge_detect <input.png> <output.pbm> [0-255] [num_threads]\n");
    return 1;
  }

  omp_set_num_threads(atoi(argv[4]));
  nt = atoi(argv[4]);

  //Update with user-defined threshold
  threshold = atoi(argv[3]);
  if(threshold > 255 || threshold < 0){
    printf("Usage: edge_detect <input.png> <output.pbm> [0-255]\n");
    return 2;
  }

  //Convert PNG image to PGM using
  strcpy(pbm_file, argv[2]);
  convertPNGtoPGM(argv[1]);

	start_all = omp_get_wtime();
  parseImg(argv[1]);

  detectEdges();

  printToFile(argv[2]);
  end_all = omp_get_wtime();
  all_time = end_all - start_all;
  
  printf("Runtime for parallelized section was: %f seconds for %d threads.\n", parallel_time, nt);
  printf("Runtime for the whole program(excluding conversion) was: %f for %d threads.\n\n", all_time, nt);
  
  free_arrays(); //VERY LAST
  return 0;
}
