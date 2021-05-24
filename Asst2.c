
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include <dirent.h>
#include <pthread.h>

pthread_mutex_t lock;

pthread_t tid[1000];


//linked lists of wordNode of each file                                                                                                                                              
typedef struct fileN{
  int total;
  char text[1000];
  struct fileN* next;
  struct node* wordList;
} fileN;

//linked list for word
typedef struct node{
  int count;
  float ratio;
  char word[1000];
  struct node* next;
}node;//our struct for word nodes


//linked list for storing print order
typedef struct final{
  char first[1000];
  char second[1000];
  float jsd;
  int sum;//first + second total
  struct final* next;
}final;


char tempFile[1000];

struct fileN* filepath;

struct final* finalHead;

//char tempFile[1000];//used to input filepath

//if total files<2, our main emite warning and stop;
int totalFiles = 0;


//MAIN FUNCS
void* dirReader(void* target_path);
//reads the pathway, opens directory, create pthread

void* fileHandler( void* pathway);
//uses mutex, add tokens, count tokens

void computer();
//compute the similarity between each file pair and print


//SUPPORTING FUNCS FOR DATA STRUCT
node* createNode(char* newWord);
fileN* createFile(char* pathway);

void addWord(fileN* new, char* word);//adds word to the wordList
//I tried to do insertion sort, but the LL of LL was not responding correctly
//either it was missing nodes, or seg fault
//I decided to sort the lists alphebetically by making the sortFile() function

void addFile(fileN* currFile);//add file to file list
void printList();//prints all the words in all files
void sortFile();//sorts the complete filepath


//SUPPORTING FUNCS FOR MATH
void sortList();//used to sort the list of comparisons, order: smallest->largest
void wordRatio();//used to make the word counts into decimal

final* createFinal(fileN* first, fileN* second);
float getJSD(fileN* first, fileN* second);
void addFinal(final* fnode);
void sortFinal();
void printFinal();
void colorPrint(float jsd);//prints the jsd color

node* createNode(char* newWord){

  node* newNode = malloc(sizeof(*newNode));//learned that using the variable itself also works
  //don't have to do sizeof(node*), cause the var is already the same struct

  strcpy(newNode->word, newWord);

  //printf("newNode->word: %s\n", newNode->word);
  
  newNode->count = 1;
  newNode->next = NULL;
  
  return newNode;
}

fileN* createFile(char* pathway){
  fileN* new = malloc(sizeof(*new));

  strcpy(new->text, pathway);

  //printf("strcpy(new->text, pathway);= %s\n", new->text);

  new->wordList = NULL;
  new->next = NULL;
  new->total = 0;
  
  return new;
}


void addWord(fileN* new, char* word){//fileN* currFile){

  node* newWord = createNode(word);

  //printf("newWord: %s\n", word);

  if(new->wordList == NULL){

    new->total++;
    
    new->wordList = newWord;    


    return;

  }

  else{


    node* newNode = new->wordList;

    node* prev = NULL;
    
    while(newNode != NULL){
      
      if(strcmp(newNode->word, word)==0){
	//word exits in list                                                                                                                                                         
	newNode->count++;
	new->total++;
	
	return;
      }
      /*ORIGNAL INSERT SORT CODE, unable to fix, I decided to make sortFile()instead

      else if(strcmp(word, newNode->word) < 0){
	//word alphabetically comes before newNode
	//we will insert it

	new->total++;
	
	if(prev == NULL){
	  //insert word into beginning
	  printf("%s->%s\n", newWord->word, newNode->word);

	  newWord->next = newNode;
	  newNode = newWord;
	  return;
	}
	
	else{
	  //insert in the middle
	  
	  //prev->next = newWord;

	  printf("%s->%s->%s\n", prev->word, newWord->word, newNode->word);
	  newWord->next = newNode;
	  prev->next = newWord;
	  return ;
	}
	
      }
      */

      
      prev = newNode;
      newNode = newNode->next;
      
    }//after while complete prev should be at last node
    
    
    prev->next = newWord;
    
    new->total++;

    return ;
    

  }

  
}


void addFile(fileN* new){

  fileN* current = filepath;

  fileN* node = new;

  if(current == NULL){
    node->next = current;
    filepath = node;
    return;
  }
  
  while( current!=NULL){

    if(strcmp(current->text, new->text)==0){

      printf("ERROR, the same file was opened.\n");
      
      return;
    }
    
    if(current->next == NULL) break;
    current = current->next;
  }//current == last node

  current->next = node;
  node->next = NULL;
  
  return;
  
}


void sortFile(){

  fileN* file = filepath;

  while(file){

    node* current  = file->wordList;

    node* next;//uses next to iterate through the entire wordList
    
    if(current==NULL){//if the wordList is blank
      
      file = file->next;
      continue;
    }

    while( current != NULL){//each while loop would move to next node

      next = current->next;//the current would be compared with rest of nodes

      while(next != NULL){

	if(strcmp(next->word, current->word) < 0){

	  char temp[1000];

	  int num;

	  //switch words
	  strcpy(temp, next->word);

	  strcpy(next->word, current->word);

	  strcpy(current->word, temp);

	  //switch word appearance
	  num = next->count;
	  next->count = current->count;
	  current->count = num;

	  //words and their appearance was been switched
	}

	next = next->next;


      }//done comparing current with rest of list

      current = current->next;
    }//wordList completed
    
    file = file->next;
  }//files in filepath completed
  
}


void wordRatio(){

  fileN* file = filepath;

  while(file!=NULL){

    node* current = file->wordList;

    if(current == NULL){
      //blank files have 1.0 ratio
      file = file->next;
      continue;
    }

    while( current!= NULL){

      //the (double) is needed, for current and file are both int's
      //to get decimal we need it
      float num = (current->count) / ((float) file->total);

      /*
      printf(" (current->count: %d, file->total: %d, ratio: %f\n",
	     current->count,file->total, num);
      */

      //we will just use %0.4f during printf
      //bade idea to estimate, before KLD and JSD
      //num = roundf(10000 * num) / 10000;//set and round num to 3 decimal places

      current->ratio = num;

      current = current->next;
    }

    file = file->next;
    
  }

}



float getJSD(fileN* first, fileN* second){

  float jsd = 0.0;
  
  float firstKLD = 0.0;
  float secondKLD = 0.0;

  node* one = first->wordList;
  node* two = second->wordList;

  int complete = 0; //if 1, finish two, if 2, finish 1

  float div;
  float temp;

  if(one == NULL && two == NULL){
    //they are identical
    jsd = 0.0;
    return jsd;
  }
  else if(one == NULL){
    //float total;
    while(two!= NULL){
      
      div= two->ratio+0;
      div/=2;

      temp = two->ratio*(log10(two->ratio/div));
      jsd+= temp;
      
      two = two->next;
    }
    jsd/= 2;
    return jsd;
  }
  else if(two==NULL){

    while(one!= NULL){

      div= one->ratio+0;
      div/=2;

      temp = one->ratio*(log10(one->ratio/div));
      jsd+= temp;

      one = one->next;
    }
    jsd/= 2;
    return jsd;


  }

  //move to next file, to compare to current file

  //printf("\n %s, %s\n", first->text, second->text);
  
  //printf("while(one != NULL || two != NULL)\n");
  while(one != NULL || two != NULL){

    //printf("if(strcmp(one->word, two->word)==0)\n");
    if(strcmp(one->word, two->word)==0  && complete == 0){

      //printf("%s, %s\n", one->word, two->word);
      div = one->ratio + two->ratio;
      div /= 2;

      temp = one->ratio * (log10(one->ratio/ div));
      firstKLD += temp;

      temp = two->ratio* (log10(two->ratio/ div));
      secondKLD += temp;

      one = one->next;
      two = two->next;

    }

    //printf("else if(strcmp(one->word, two->word)<0)\n");
    else if(strcmp(one->word, two->word)<0 && complete == 0){//one is before two

      //printf("%s, %s\n", one->word, two->word);
      
      div = one->ratio + 0;
      div /= 2;

      temp = one->ratio * (log10(one->ratio/ div));
      firstKLD += temp;

      one= one->next;
    }

    //printf("else if(strcmp(one->word, two->word)>0)\n");
    else if(strcmp(one->word, two->word)>0  && complete == 0){//two is before one

      //printf("%s, %s\n", one->word, two->word);
      
      div = two->ratio + 0;
      div/= 2;

      temp = two->ratio * (log10(two->ratio/div));
      secondKLD += temp;

      two = two->next;

    }

    if(one== NULL && two==NULL) break;
    else if(one == NULL) complete = 1;//one is comleted, finish two
    else if(two == NULL) complete = 2;//two is complete, finish one

    //printf("if(complete==1){\n");
    if(complete==1){

      while(two != NULL){

	div =two->ratio + 0;
	div/= 2;

	temp = two->ratio* (log10(two->ratio/div));
	secondKLD+= temp;

	two = two->next;
      }
      
    }
    //printf("if(complete==2){\n");
    else if(complete==2){
      //printf("if(complete==2){\n");

      while(one != NULL){
	div= one->ratio+ 0;
	div/= 2;

	temp= one->ratio* (log10(one->ratio / div));
	firstKLD += temp;
      
	one = one->next;
      }
    }

    
  }//firstKLD and second KLD should be complete
  
  jsd = firstKLD + secondKLD;
  jsd/= 2;
  
  return jsd;
}


final* createFinal(fileN* first, fileN* second){

  //printf("final* fnode = malloc(sizeof(fnode));\n");
  //malloc memory currption
  //i had to do (final*) and sizeof(final) to prevent it
  final* fnode = (final*) malloc(sizeof(final));
  
  //printf("strcpy(fnode->first, first->text); \n");
  strcpy(fnode->first, first->text);

  strcpy(fnode->second, second->text);

  //printf("fnode->jsd = getJSD(first, second);\n");
  fnode->jsd = getJSD(first, second);

  //printf("fnode->sum = first->total + second->total;\n");
  fnode->sum = first->total + second->total;

  fnode->next = NULL;
  
  return fnode;
}


void addFinal(final* fnode){

  final* current = finalHead;

  final* node = fnode;


  if(node->jsd < -1){
    return;
  }
  
  if(current == NULL){
    node->next = current;
    finalHead = node;
    return;
  }
  
  while( current!= NULL){
    
    if(strcmp(current->first, node->first)==0 &&
       strcmp(current->second, node->second) == 0){
	 
      printf("ERROR, the same files were compared before.\n");
	 
      return;
    }

    if(current->next == NULL) break;
    current = current->next;
  }//current == last node                                                                                                                                                            
  
  current->next = node;
  node->next = NULL;
  
  return;
  
}

void sortFinal(){

  final* current = finalHead;
  final* next;
  int temp;
  float ftemp;
  char ctemp[1000];
  
  while(current != NULL){

    //printf(" next = current->next;\n");
    next = current->next;

    //printf("Current: %d\n", current->sum);
    
    
    while(next != NULL){

      //printf("Next: %d\n", next->sum);
      
      if(current->sum > next->sum){
	
	temp = next->sum;
	next->sum = current->sum;
	current->sum = temp;

	ftemp = next->jsd;
	next->jsd = current->jsd;
	current->jsd = ftemp;
	
	//first filename swap
	strcpy(ctemp, next->first);
	strcpy(next->first, current->first);
	strcpy(current->first, ctemp);
	
	//second filename swap
	strcpy(ctemp, next->second);
	strcpy(next->second, current->second);
	strcpy(current->second, ctemp);

	//printf("SWAP Current: %d\n", current->sum);
	//printf("SWAP Next: %d\n", next->sum);
      }

      next= next->next;	  
    }
    
    current = current->next;
  }

}


void printColor(float jsd){

  if(jsd >= 0 &&
     jsd <= 0.1){

    printf("\033[0;31m");//color red
    printf("%f\n", jsd);
    printf("\033[0m");//return to default color
  }
  else if(jsd >0.1 &&
	  jsd <= 0.15){

    printf("\033[0;33m");//color yellow
    printf("%f\n", jsd);
    printf("\033[0m");
  }
  else if(jsd>0.15 &&
	  jsd <= 0.2){

    printf("\033[0;32m");//color green
    printf("%f\n", jsd);
    printf("\033[0m");
  }
  else if(jsd >0.2 &&
	  jsd <= 0.25){

    printf("\033[0;36m");//color cyan
    printf("%f\n", jsd);
    printf("\033[0m");
  }
  else if(jsd > 0.25 &&
	  jsd <=0.3){

    printf("\033[0;34m"); //color blue
    printf("%f\n", jsd);
    printf("\033[0m");
  }
  else if(jsd > 0.3){
    printf("\033[0;37m");//color white
    printf("%f\n", jsd);
    printf("\033[0m");
  }
  
}


void printFinal(){

  final* current = finalHead;

  int number = 1;

  printf("JSD COMPARISON:\n");
  
  while(current != NULL){

    printf("%d. %s and %s has a total of %d words and a JSD: ",
	   number, current->first, current->second, current->sum);//, current->jsd);

    printColor(current->jsd);//printout jsd with color
    
    current = current->next;

    number++;
  }

}

  
void* dirReader( void* target_path)
{
  DIR* dir;
  struct dirent* entry;

  int i = 0;
  
  //pthread_t c;

  void* result;
  
  dir = opendir(target_path);

  //int* ret = malloc(sizeof(int));

  if(dir == NULL){
    printf("ERROR: Inaccesible directory\n");
    //return;
  }
  
  
  if( dir != NULL){

    //printf("current dir: %s\n", (char*)target_path);
    
    while(( entry = readdir(dir)) != NULL){
      
      if(entry->d_type == DT_DIR &&
	 strcmp(entry->d_name, ".") != 0 && //current dir ignored
	 strcmp(entry->d_name, "..") != 0){ // parent dir ignored
	
	//printf("./%s\n", entry->d_name);

	//create the next pathway for subdir
	char* next= malloc(strlen(target_path) + strlen(entry->d_name) +2);
	strcpy(next, target_path);
	strcat(next, "/");
	strcat(next, entry->d_name);

	//printf("entry name: %s\n", entry->d_name);
	
	//printf("%s coming next\n", next);
	
	pthread_create(&tid[i], NULL, &dirReader,(void*) next);
	//pthread_join(tid[i], &result);
	i++;
	
	//dirReader(next);
      }
      else if(entry->d_type == DT_REG){//a file
	//we can't use != DT_DIR, because socket, fifo, and so on would be included
	//instructions wanted us to ignore everything not a file/directory
				
	//create file's pathway
	char* filename = malloc(strlen(target_path) + strlen(entry->d_name) + 2);
	strcpy(filename, target_path);
	strcat(filename, "/");
	strcat(filename, entry->d_name);


	pthread_create(&tid[i], NULL, &fileHandler, (void*) filename);

	i++;
	
      }
    }//while end

    int j = 0;
    while(j < i){
      pthread_join(tid[j], &result);

      //sleep(5);
      
      //pthread_exit(&result);
      j++;
    }

    closedir(dir);

    }
    
  else{
    printf("ERROR: file pathway is not found.\n");
  }
  return NULL;
}


void* fileHandler(void* pathway){

  
  FILE *fp;

  fp = fopen(pathway, "r");

  char c;
  char word[1000];

  //int used = 0;
  
  if( fp == NULL ) {
    printf("Cannot open file.\n");
    return NULL;
  }
  
  pthread_mutex_lock(&lock);//locking the pthread
  
  int length = 0;
  //word begins


  //hyphens == alphabets
  //so '----' is a tok, '-abc' is a tok, 'abc-' is a tok
  //int returned = 0;//records where the hyphen ends
  //is next is not alphabet or hyphen, return length to before hyphen
  

  //int run = 0;//if 0 run while, else nothing
  
  strcpy(tempFile, (char*) pathway);
  
  fileN* currFile = createFile(tempFile);

  addFile(currFile);

  //printf("file: %s\n",currFile->text);
  //printf("words: %d\n",currFile->total);


  //char t = fgetc(fp);
  
  /*if(t == EOF){//blank file still counts

    totalFiles++;
  }
  */

  totalFiles++;
  //accessible files all count, even blank!
  
  while( (c = fgetc(fp) ) != EOF){
    
    if ( isspace(c) != 0 ){

      /*
      if( word[length-1]=='-' ){//char before space
	//if ^ true, remove all hyphens

	length = returned;
	//here is where hyphen appeared
	//next we replace length with '\0'

      }
      */
      word[length] = '\0';

      if(word[0] == '\0')continue;//repeated isspace

      //printf("Word found: %s\n", word);
      
      addWord(currFile, word);

      //hyphen is included, zero everything
      //returned = 0;
      length = 0;
      word[0] = '\0';
      
    }
    
    else{
      
      if ( (c == 'a') || (c=='A')
	   || (c=='b') || (c=='B')
	   || (c=='c') || (c=='C')
	   || (c=='d') || (c=='D')
	   || (c=='e') ||  (c=='E')
	   || (c== 'F') || (c=='f')
	   || (c== 'g') ||  (c=='G')
	   ||(c == 'h') ||  (c=='H')
	   || (c== 'i') ||  (c=='I')
	   || (c== 'j') || (c=='J')
	   || (c== 'k')|| (c=='K')
	   || (c== 'l')|| (c=='L')
	   || (c == 'M')|| (c=='m')
	   || (c == 'n')|| (c=='N')
	   || (c == 'o')|| (c=='O')
	   || (c== 'p')|| (c=='P')
	   || (c == 'q')|| (c=='Q')
	   || (c== 'r')|| (c=='R')
	   || (c== 's')|| (c=='S')
	   || (c == 't')||  (c=='T')
	   || (c== 'u')|| (c=='U')
	   || (c== 'v')|| (c=='V')
	   || (c=='w')|| (c=='W')
	   ||(c=='x')|| (c=='X')
	   ||(c=='y')|| (c=='Y')
	   ||(c=='z')|| (c=='Z')
	   ||(c=='-')
	   ){
	
	/*
	if(length==0 &&
	c == '-'){
	//if ^ true, then hyphen is not sandwiched between chars
	//ignore
	continue;
	}
	*/
	//else{
	char lower = tolower(c);
	
	word[length] = lower;
	
	/*
	  if(c =='-' &&
	  returned == 0){
	  
	  //hyphen appears and no hyphen appear previously
	  
	  returned = length;
	  //this is where before the hyphen appears
	    //if hyphen not between chars, we will ignore it
	    
	    }
	*/
	length++;
	//}
	//add char c into word array, until word is created
	
      }
      else continue;
    }
    
    //not a blank file
    //blank files still count as a file
    //add to total files

  }
  
 
  
  pthread_mutex_unlock(&lock);
  
  fclose(fp);


  return NULL;
}


void printList(){

  fileN* file = filepath;

  int number = 1;
  
  while(file!= NULL){

    if(file->total >= 1){
      
      printf("%d. Filename: %s, total words: %d\n",
	     number, file->text, file->total);

      number++;
      
      struct node* node = file->wordList;
    
      while(node != NULL){

	printf("%s appeared %d times, ratio %0.5f\n",
	       node->word, node->count, node->ratio);

	//number++;
	
	node = node->next;

      }

    }

    else{//blank file
      printf("%d. Filename: %s, total words: %d\n",
             number, file->text, file->total);

      number++;
    }
    
    file = file->next;

  }

  return;
}


void computer(){

  fileN* file = filepath;

  fileN* next;

  final* temp;
  
  while( file != NULL){

    //printf("next= file->next;\n");
    next= file->next;

    //printf("if(next == NULL)\n");
    if(next == NULL){
      //file = file->next;
      //continue;
      break;
    }

    while(next != NULL){
      //printf("temp = createFinal(file, next);\n");
      //printf("file: %s + next: %s\n", file->text, next->text);
      temp = createFinal(file, next);
    
      //printf("addFinal(temp);\n");
      addFinal(temp);

      next = next->next;
    }
    
    file = file->next;
  }

}


int main(int argc, char* argv[]){
  
  dirReader(argv[1]);

  //if true, not enough files to run JSD
  //emit warning and stop
  if(totalFiles < 2){

    printf("WARNING: NOT ENOUGH FILES, ONLY %d FILES FOUND\n",
	   totalFiles);

    return 0;

  }

  //printf("sortFile();\n");
  sortFile();
  //printf("wordRatio();\n");
  wordRatio();
  
  printList();

  // printf("computer();\n");
  computer();

  //printf("sortFinal();\n"); 
  sortFinal();

  //printf("printFinal();\n");
  printFinal();
  
  
  pthread_mutex_destroy(&lock);

  //printList();
  
  return EXIT_SUCCESS;

}

