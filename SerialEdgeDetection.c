#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

// IT388 | 10/19/2019 | Jason B.

//Primitives ----
int threshold, max = 1000, width, height;
char pbm_file[80];

//Pointers ----
FILE * fileptr;
char * tokens;
int ** pixel_array;
int ** bw_Array;

//Timekeeping ----
clock_t startFull, start, endFull, end;
double elapsedFull = 0, elapsed;

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
    int j = 0;

    //Handling Header on Image
    for(i = 0; fgets(buffer, max, fileptr) != NULL; i++){
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

    //INIT 2D ARRAY
    pixel_array = (int **)malloc(height * sizeof(int *));
    for(j = 0; j < height; j++){
      pixel_array[j] = (int *) malloc(width * sizeof(int));
    }

    int row = 0;
    int col = 0;
    int count = 0;

    //Handling Pixel Data on Image
    for(row = 0, count = 0; row < height; row++) {
      for(col = 0; col < width; col++, count++){
        pixel = fgetc(fileptr);
        pixel_array[row][col] = pixel;
      }
    }

    fclose(fileptr);
    system("rm gray.pgm");
}

void free_arrays(){
  int i = 0;
  for(i = 0; i < height; i++){
    free(pixel_array[i]);
    free(bw_Array[i]);
  }
  free(pixel_array);
  free(bw_Array);
}


void detectEdges(){
start = clock();
  int i = 0, j = 0, k = 0;
  bw_Array = (int **)malloc(height * sizeof(int *));

  for( i =0; i < height; i++){
    bw_Array[i] = (int *) malloc(width * sizeof(int));
  }

  for( k = 0; k < height; k++){
    for( j = 0; j < width; j++){
      bw_Array[k][j] = 1;
    }
  }

  int col = 0;
  int row = 0;

  for( col = 0; col < width; col++){
    for( row = 0; row < height-1; row++){
      if(abs(pixel_array[row][col] - pixel_array[row+1][col]) > threshold){
        bw_Array[row][col] = 0;
      }
    }
  }

  int rowA = 0;
  int colA = 0;

  for( rowA = 0; rowA < height; rowA++){
    for( colA = 0; colA < width-1; colA++){
      if(abs(pixel_array[rowA][colA] - pixel_array[rowA][colA+1]) > threshold){
        bw_Array[rowA][colA] = 0;
      }
    }
  }
  end = clock();
  elapsed = ((double)(end-start)) / CLOCKS_PER_SEC;

}

void printToFile(char * fileName){

  FILE * writeFile = fopen(fileName, "w");
  fprintf(writeFile, "P1\n");
  fprintf(writeFile, "%d %d\n", width, height);

  //fwrite(filename, sizeof(filename), 1, writeFile); //#filename would go here
  //fwrite(newLine, sizeof(newLine), 1, writeFile);

  int row  = 0;
  int col = 0;

  for( row = 0; row < height; row++){ //write numbers
    for( col = 0; col < width; col++){
      fprintf(writeFile, "%d ", bw_Array[row][col]);
    }
    fprintf(writeFile, "\n"); //newLine Time
  }
}

int main(int argc, char * argv[]){
  //Usage convertPNGtoPGM <input.png> <output.pgm> threshold
  if(argc != 4){
    printf("Usage: edge_detect <input.png> <output.pbm> [0-255]\n");
    return 1;
  }

  //Update with user-defined threshold
  threshold = atoi(argv[3]);
  if(threshold > 255 || threshold < 0){
    printf("Usage: edge_detect <input.png> <output.pbm> [0-255]\n");
    return 2;
  }

  //Convert PNG image to PGM using
  strcpy(pbm_file, argv[2]);
  convertPNGtoPGM(argv[1]);

  startFull = clock();
  

  parseImg(argv[1]);

  detectEdges();

  printToFile(argv[2]);
  
  endFull = clock();
  elapsedFull = ((double)(endFull-startFull)) / CLOCKS_PER_SEC;

  printf("Runtime for parallelized section was: %f seconds for 1 thread.\n", elapsed);
  printf("Runtime for the whole program(excluding conversion) was: %f for 1 thread.\n\n", elapsedFull);
  free_arrays(); //VERY LAST
  return 0;
}
