/*
    COMP3511 Fall 2022 
    PA3: Page-Replacement Algorithms

    Your name: Liangyawei Kuang
    Your ITSC email: lkuang@connect.ust.hk 

    Declaration:

    I declare that I am not involved in plagiarism
    I understand that both parties (i.e., students providing the codes and students copying the codes) will receive 0 marks. 

*/

// Note: Necessary header files are included
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Constants
#define UNFILLED_FRAME -1
#define MAX_QUEUE_SIZE 10
#define MAX_FRAMES_AVAILABLE 10 
#define MAX_REFERENCE_STRING 30
#define ALGORITHM_FIFO "FIFO"
#define ALGORITHM_OPT "OPT"
#define ALGORITHM_LRU "LRU"

// Keywords (to be used when parsing the input)
#define KEYWORD_ALGORITHM "algorithm"
#define KEYWORD_FRAMES_AVAILABLE "frames_available"
#define KEYWORD_REFERENCE_STRING_LENGTH "reference_string_length"
#define KEYWORD_REFERENCE_STRING "reference_string"

// Useful string template used in printf()
// We will use diff program to auto-grade the submissions
// Please use the following templates in printf to avoid formatting errors
//
// Example:
//
//   printf(template_total_page_fault, 0)    # Total Page Fault: 0 is printed on the screen
//   printf(template_no_page_fault, 0)       # 0: No Page Fault is printed on the screen

const char template_total_page_fault[] = "Total Page Fault: %d\n";
const char template_no_page_fault[] = "%d: No Page Fault\n";

// Assume that we only need to support 2 types of space characters: 
// " " (space), "\t" (tab)
#define SPACE_CHARS " \t"


// Global variables
// Very important!
char algorithm[10]; 
int reference_string[MAX_REFERENCE_STRING]; 
int reference_string_length; 
int frames_available;
int frames[MAX_FRAMES_AVAILABLE]; 

// 以下代码不用细看
// Helper function: Check whether the line is a blank line (for input parsing)
int is_blank(char *line) {
    char *ch = line;
    while ( *ch != '\0' ) {
        if ( !isspace(*ch) )
            return 0;
        ch++;
    }
    return 1;
}
// Helper function: Check whether the input line should be skipped
int is_skip(char *line) {
    if ( is_blank(line) )
        return 1;
    char *ch = line ;
    while ( *ch != '\0' ) {
        if ( !isspace(*ch) && *ch == '#')
            return 1;
        ch++;
    }
    return 0;
}
// Helper: parse_tokens function
void parse_tokens(char **argv, char *line, int *numTokens, char *delimiter) {
    int argc = 0;
    char *token = strtok(line, delimiter);
    while (token != NULL)
    {
        argv[argc++] = token;
        token = strtok(NULL, delimiter);
    }
    *numTokens = argc;
}

// Helper: parse the input file
void parse_input() {
    FILE *fp = stdin;
    char *line = NULL;
    ssize_t nread;
    size_t len = 0;

    char *two_tokens[2]; // buffer for 2 tokens
    char *reference_string_tokens[MAX_REFERENCE_STRING]; // buffer for the reference string
    int numTokens = 0, n=0, i=0;
    char equal_plus_spaces_delimiters[5] = "";

    strcpy(equal_plus_spaces_delimiters, "=");
    strcat(equal_plus_spaces_delimiters,SPACE_CHARS);

    while ( (nread = getline(&line, &len, fp)) != -1 ) {
        if ( is_skip(line) == 0)  {
            line = strtok(line,"\n");
            
            if (strstr(line, KEYWORD_ALGORITHM)) {
                parse_tokens(two_tokens, line, &numTokens, equal_plus_spaces_delimiters);
                if (numTokens == 2) {
                    strcpy(algorithm, two_tokens[1]);
                }
            } 
            else if (strstr(line, KEYWORD_FRAMES_AVAILABLE)) {
                parse_tokens(two_tokens, line, &numTokens, equal_plus_spaces_delimiters);
                if (numTokens == 2) {
                    sscanf(two_tokens[1], "%d", &frames_available);
                }
            }
            else if (strstr(line, KEYWORD_REFERENCE_STRING_LENGTH)) {
                parse_tokens(two_tokens, line, &numTokens, equal_plus_spaces_delimiters);
                if (numTokens == 2) {
                    sscanf(two_tokens[1], "%d", &reference_string_length);
                }
            } 
            else if (strstr(line, KEYWORD_REFERENCE_STRING)) {

                parse_tokens(two_tokens, line, &numTokens, "=");
                // printf("Debug: %s\n", two_tokens[1]);
                if (numTokens == 2) {
                    parse_tokens(reference_string_tokens, two_tokens[1], &n, SPACE_CHARS );
                    for (i=0; i<n; i++) {
                        sscanf(reference_string_tokens[i], "%d", &reference_string[i]);
                    }
                }
            }
            


        }
    }
}
// Helper: Display the parsed values
void print_parsed_values() {
    int i;
    printf("%s = %s\n", KEYWORD_ALGORITHM, algorithm);
    printf("%s = %d\n", KEYWORD_FRAMES_AVAILABLE, frames_available);
    printf("%s = %d\n", KEYWORD_REFERENCE_STRING_LENGTH, reference_string_length);
    printf("%s = ", KEYWORD_REFERENCE_STRING);
    for (i=0; i<reference_string_length;i++)
        printf("%d ", reference_string[i]);
    printf("\n");

}
// 以上代码不用细看

// A simple integer queue implementation using a fixed-size array
// Helper functions:
//   queue_init: initialize the queue
//   queue_is_empty: return true if the queue is empty, otherwise false
//   queue_is_full: return true if the queue is full, otherwise false
//   queue_peek: return the current front element of the queue
//   queue_enqueue: insert one item at the end of the queue
//   queue_dequeue: remove one item from the beginning of the queue
//   queue_print: display the queue content, it is useful for debugging
struct Queue {
    int values[MAX_QUEUE_SIZE];
    int front, rear, count;
};
void queue_init(struct Queue* q) {
    q->count = 0;
    q->front = 0;
    q->rear = -1;
}
int queue_is_empty(struct Queue* q) {
    return q->count == 0;
}
int queue_is_full(struct Queue* q) {
    return q->count == MAX_QUEUE_SIZE;
}
int queue_is_ful(struct Queue* q) {
    return q->count == frames_available;
}
int queue_peek(struct Queue* q) {
    return q->values[q->front];
}
int queue_rear(struct Queue* q) {
    return q->values[q->rear];
}
void queue_enqueue(struct Queue* q, int new_value) {
    if (!queue_is_full(q)) {
        if ( q->rear == MAX_QUEUE_SIZE -1)
            q->rear = -1;
        q->values[++q->rear] = new_value;
        q->count++;
    }
}
void queue_dequeue(struct Queue* q) {
    q->front++;
    if (q->front == MAX_QUEUE_SIZE)
        q->front = 0;
    q->count--;
}
void queue_print(struct Queue* q) {
    int c = q->count;
    printf("size = %d\n", c);
    int cur = q->front;
    printf("values = ");
    while ( c > 0 ) {
        if ( cur == MAX_QUEUE_SIZE )
            cur = 0;
        printf("%d ", q->values[cur]);
        cur++;
        c--;
    }
    printf("\n");
}
void queue_printt(struct Queue* q) {
    int c = q->count;
    int cur = q->front;
    printf("values = ");
    while ( c > 0 ) {
        if ( cur == MAX_QUEUE_SIZE )
            cur = 0;
        printf("%d ", q->values[cur]);
        cur++;
        c--;
    }
    printf("\n");
}
int queue_select(struct Queue* q, int i) {
    int cur = q->front;
    if (cur + i == MAX_QUEUE_SIZE)
        return q->values[0];
    return q->values[cur+i];
}
void queue_replace(struct Queue* q, int i, int j) {
    int cur = q->front;
    if (cur + i == MAX_QUEUE_SIZE)
        q->values[0] = j;
    else
        q->values[cur+i] = j;
}

struct Que {
    int values[MAX_REFERENCE_STRING];
    int front, rear, count;
};
void que_init(struct Que* q) {
    q->count = 0;
    q->front = 0;
    q->rear = -1;
}
int que_is_empty(struct Que* q) {
    return q->count == 0;
}
int que_is_full(struct Que* q) {
    return q->count == MAX_REFERENCE_STRING;
}
int que_is_ful(struct Que* q) {
    return q->count == frames_available;
}
int que_peek(struct Que* q) {
    return q->values[q->front];
}
int que_rear(struct Que* q) {
    return q->values[q->rear];
}
void que_enqueue(struct Que* q, int new_value) {
    if (!que_is_full(q)) {
        if ( q->rear == MAX_REFERENCE_STRING -1)
            q->rear = -1;
        q->values[++q->rear] = new_value;
        q->count++;
    }
}
void que_dequeue(struct Que* q) {
    q->front++;
    if (q->front == MAX_REFERENCE_STRING)
        q->front = 0;
    q->count--;
}
void que_print(struct Que* q) {
    int c = q->count;
    printf("size = %d\n", c);
    int cur = q->front;
    printf("values = ");
    while ( c > 0 ) {
        if ( cur == MAX_REFERENCE_STRING )
            cur = 0;
        printf("%d ", q->values[cur]);
        cur++;
        c--;
    }
    printf("\n");
}
void que_printt(struct Que* q) {
    int c = q->count;
    int cur = q->front;
    printf("values = ");
    while ( c > 0 ) {
        if ( cur == MAX_REFERENCE_STRING )
            cur = 0;
        printf("%d ", q->values[cur]);
        cur++;
        c--;
    }
    printf("\n");
}
int que_select(struct Que* q, int i) {
    int cur = q->front;
    if (cur + i == MAX_REFERENCE_STRING)
        return q->values[0];
    return q->values[cur+i];
}
void que_replace(struct Que* q, int i, int j) {
    int cur = q->front;
    if (cur + i == MAX_REFERENCE_STRING)
        q->values[0] = j;
    else
        q->values[cur+i] = j;
}

// Helper function:
// This function is useful for printing the fault frames in this format:
// current_frame: f0 f1 ...
//
// For example: the following 4 lines can use this helper function to print 
//
// 7: 7     
// 0: 7 0   
// 1: 7 0 1 
// 2: 2 0 1 
//
// For the non-fault frames, you should use template_no_page_fault (see above)
//
void display_fault_frame(int current_frame) {
    int j;
    printf("%d: ", current_frame);
    for (j=0; j<frames_available; j++) {
        if ( frames[j] != UNFILLED_FRAME )
            printf("%d ", frames[j]);
        else 
            printf("  ");
    }
    printf("\n");
}


void algorithm_FIFO() {
    // TODO: Implement the FIFO algorithm here
    struct Queue temp_queue[frames_available];
    queue_init(temp_queue);
    int  fault = 0;
    int  i, j;
    int  position = 0;
    int  in, peek;
    for (i=0; i<reference_string_length; i++){
    //for (i=0; i<2; i++){
        //printf("i = %d \n", i);
        in = 0;
        for (j=0; j<frames_available; j++) {
            if (frames[j] == reference_string[i]) 
                in = 1;
        }
        //printf("in is %d \n", in);
        if (in == 1){
        //    printf("in = 1, no fault! \n");
            printf(template_no_page_fault, reference_string[i]);
        }
        if (in == 0){
        //    printf("in = 0, fault! \n");
            fault += 1;
        //    printf("\n current queue is:");
        //    queue_print(temp_queue);
        //    printf("\n");
        //    printf("queue is full or not: %d \n", queue_is_ful(temp_queue));
            if (queue_is_ful(temp_queue) == 0){
        //        printf("queue is NOT full \n");
                queue_enqueue(temp_queue, reference_string[i]);
                frames[position] = reference_string[i];
                position += 1;
            }
            else if (queue_is_ful(temp_queue) != 0){
        //        printf("queue is full \n");
                peek =  queue_peek(temp_queue);
                for (j=0; j<frames_available; j++) {
                    if (frames[j] == peek) 
                    frames[j] = reference_string[i];
                }
                queue_dequeue(temp_queue);
                queue_enqueue(temp_queue, reference_string[i]);
            }
        //    printf("current frame is %d ", frames[0]);
        //    printf("%d ", frames[1]);
        //    printf("%d \n", frames[2]);
            display_fault_frame(reference_string[i]);
        //    printf("\n");
        }
    }
    //queue_enqueue(temp_queue, 1);
    //display_fault_frame(temp_queue);
    
    printf(template_total_page_fault, fault);
}

void algorithm_OPT() {
    // TODO: Implement the OPT algorithm here
    struct Que position_que[MAX_REFERENCE_STRING];
    que_init(position_que);
    struct Queue dead_queue[MAX_QUEUE_SIZE];
    queue_init(dead_queue);
    struct Queue full_queue[frames_available];
    queue_init(full_queue);
    int  fault = 0;
    int  i, j, k;
    int  position = 0;
    int  in, find, position_dead;
    int  find_last, position_last = -1;
    int  dead, pick;
    for (i=0; i<reference_string_length; i++){
        que_enqueue(position_que, -1);
    }
    //que_print(position_que); // the initial position queue
    for (i=0; i<MAX_QUEUE_SIZE; i++){
        queue_enqueue(dead_queue, -1);
    }
    //queue_print(dead_queue); // to be added by strings will not occur, with ten digits
    //queue_print(full_queue); // frames without orders
    //printf("\n");

    for (i=0; i<reference_string_length; i++){
    //for (i=0; i<11; i++){
        //printf("current iteration number %d\n\n", i);
        //printf("current string i is %d\n", reference_string[i]);
        in = 0;
        find = 0;
        find_last = 0;

        // to judge whether current string is in current frames or not

        //printf("%d ", frames[0]);
        //printf("%d ", frames[1]);
        //printf("%d \n", frames[2]);

        for (j=0; j < position; j++) {
            if (frames[j] == reference_string[i]) 
                in = 1;
        }
        /*printf("current frame length is %d\n", position+1);
        printf("current string in current frames? %d\n", in);

        printf("%d ", frames[0]);
        printf("%d ", frames[1]);
        printf("%d \n", frames[2]);*/

        // to judge whether current string occurs in the later strings with a last position
        for (j=i+1; j<reference_string_length; j++){
            //printf("j is %d ", j);
            //que_printt(position_que);
            if (que_select(position_que, j) != -1) {
                find_last = 1;
                //printf("%d ", que_select(position_que, j));
                position_last = j;
                //printf("%d\n", position_last);
            }
        }
        //printf("find_last %d\n", find_last);
        //printf("position_last %d\n", position_last);

        if (in == 1){// current string is in current frames, there is NO fault
            //printf("\n");
            printf(template_no_page_fault, reference_string[i]);
        }
        if (in == 0){// current string is NOT in current frames,there is A fault
            fault += 1;
            if (queue_is_ful(full_queue) == 0){ //queue is NOT full 
                //printf("\n");
                //printf("full_queue is Not full\n");
                queue_enqueue(full_queue, reference_string[i]);
                frames[position] = reference_string[i];
                position += 1;
            }
            else if (queue_is_ful(full_queue) != 0){ //queue is full 
                //printf("\n");
                //printf("full_queue is full\n");
                dead = 0;
                for (j=0; j < MAX_QUEUE_SIZE; j++){
                    for (k=0; k < position; k++){
                        if (dead == 1) continue;
                        if (queue_select(dead_queue, j) == frames[k]){
                            dead = 1;
                            pick = queue_select(dead_queue, j);
                            //printf("dead is 1, and pick is %d", pick);
                        }
                    }
                }
                //printf("is current string dead? %d\n", dead);
                if (dead == 1){//some current strings are dead
                //printf("dead is 1 \n");
                    for (j=0; j<frames_available; j++) {
                        if (frames[j] == pick) 
                        frames[j] = reference_string[i];
                    }
                }
                if (dead == 0){//current strings are still alive
                //printf("dead is 0 \n");
                //printf("position last is %d \n",position_last);
                //printf("position last string is %d \n",reference_string[position_last]);
                    for (j=0; j<frames_available; j++) {
                        if (frames[j] == que_select(position_que, position_last)) {
                            frames[j] = reference_string[i];
                            que_replace(position_que, position_last, -1);
                        }
                    }
                }
            }
            //printf("\n");
            display_fault_frame(reference_string[i]);
            //printf("\n");
        }

        // to judge whether current string will occur later with a later position or not
        for (j=i+1; j<reference_string_length; j++){
            if (find == 1) continue;
            if (reference_string[j] == reference_string[i]) {
                find = 1;
                position_dead = j;
            }
        }
        //printf("find %d\n",find);
        //printf("position_dead %d\n",position_dead);

        if (find == 0){// current string will not occur any more
            queue_replace(dead_queue, reference_string[i], reference_string[i]); //add current string to the dead_queue
        }
        if (find == 1){// current string has occur in the later strings
        //mark the next later string which is the same as current string
            que_replace(position_que, position_dead, reference_string[i]);
        // reset the last position in position_que to be -1
            //que_replace(position_que, position_last, -1); 
        }
        que_replace(position_que, i, -1); //reset current string to be -1

        //test
        //que_print(position_que); // the initial position queue
        //queue_print(dead_queue); // to be added by strings will not occur, with ten digits

        //printf("\n\n\n");
    }
    printf(template_total_page_fault, fault);
}

/*
void algorithm_LRU() {
    // TODO: Implement the LRU algorithm here
    struct Queue adjacent_queue[frames_available];
    queue_init(adjacent_queue);
    int  fault = 0;
    int  i, j;
    int  position = 0;
    int  in, peak, rear;
    int  skip;
    for (i=0; i<reference_string_length; i++){
    //for (i=0; i<8; i++){
        //printf("i = %d \n", i);
        in = 0;
        for (j=0; j<frames_available; j++) {
            if (frames[j] == reference_string[i]) 
                in = 1;
        }
        //printf("in is %d \n", in);
        if (in == 1){//current string is inside the current frames
        //    printf("in = 1, no fault! \n");
            skip = 0;
            printf(template_no_page_fault, reference_string[i]);
            //queue_printt(adjacent_queue);
            //printf("Adjacent_queue printed! \n");
            //printf("current string %d \n", reference_string[i]);
            //printf("%d \n", queue_select(adjacent_queue, 0));
            //printf("%d \n", queue_select(adjacent_queue, 1));
            //printf("%d \n", queue_select(adjacent_queue, 2));
            for (j=0; j<frames_available-1; j++){
                printf("current j is %d \n", j);
                printf("queue j is %d \n", queue_select(adjacent_queue, j));
                printf("%d \n", queue_select(adjacent_queue, 0));
                printf("%d \n", queue_select(adjacent_queue, 1));
                printf("%d \n", queue_select(adjacent_queue, 2));
                if (queue_select(adjacent_queue, j) != reference_string[i] && skip == 0) {
                    printf("Continued! \n");
                    continue;
                }
                if (queue_select(adjacent_queue, j) == reference_string[i]) {
                    printf("Same! \n");
                    skip = 1;
                }
                if (skip == 1){
                    queue_replace(adjacent_queue, j, queue_select(adjacent_queue, j+1));
                    printf("Replaced! \n");
                    queue_printt(adjacent_queue);
                }
            }
            if (skip == 1)
            {
                queue_replace(adjacent_queue, frames_available-1, reference_string[i]);
                printf("Last one! \n");
                queue_printt(adjacent_queue);
            }
            printf("Done! \n");
            printf("\n\n\n");
        }
        if (in == 0){
        //    printf("in = 0, fault! \n");
            fault += 1;
            //printf("current adjacent queue is:\n");
        //    queue_print(temp_queue);
            //queue_print(adjacent_queue);
            //printf("\n");
        //    printf("queue is full or not: %d \n", queue_is_ful(temp_queue));
            if (queue_is_ful(adjacent_queue) == 0){
        //        printf("queue is NOT full \n");
                queue_enqueue(adjacent_queue, reference_string[i]);
                frames[position] = reference_string[i];
                position += 1;
            }
            else if (queue_is_ful(adjacent_queue) != 0){
        //        printf("queue is full \n");
                peak = queue_peek(adjacent_queue);
                //printf("rear is %d \n", rear);
                //printf("peak is %d \n", peak);
                for (j=0; j<frames_available; j++) {
                    if (frames[j] == peak) {
                    //printf ("peek is %d \n", frames[j]);
                    frames[j] = reference_string[i];
                    }
                }
                printf("%d \n", queue_select(adjacent_queue, 0));
                printf("%d \n", queue_select(adjacent_queue, 1));
                printf("%d \n", queue_select(adjacent_queue, 2));

                queue_dequeue(adjacent_queue);

                printf("current iteration: %d \n", i);
                printf("current reference_string: %d \n", reference_string[i]);
                printf("%d \n", queue_select(adjacent_queue, 0));
                printf("%d \n", queue_select(adjacent_queue, 1));
                printf("%d \n", queue_select(adjacent_queue, 2));
                queue_print(adjacent_queue);

                queue_enqueue(adjacent_queue, reference_string[i]);

                printf("changed adjacent queue is:\n");
                queue_print(adjacent_queue);
                printf("%d \n", queue_select(adjacent_queue, 0));
                printf("%d \n", queue_select(adjacent_queue, 1));
                printf("%d \n", queue_select(adjacent_queue, 2));
                printf("\n");
            }
            printf("current frame is %d ", frames[0]);
            printf("%d ", frames[1]);
            printf("%d \n", frames[2]);
            display_fault_frame(reference_string[i]);
            printf("\n\n\n");
        }
    }
    //queue_enqueue(temp_queue, 1);
    //display_fault_frame(temp_queue);
    
    printf(template_total_page_fault, fault);
}
*/
void algorithm_LRU() {
    // TODO: Implement the LRU algorithm here
    struct Queue adjacent_queue[frames_available];
    queue_init(adjacent_queue);
    int  fault = 0;
    int  i, j;
    int  position = 0;
    int  in, peak, rear;
    int  skip;
    for (i=0; i<reference_string_length; i++){
    //for (i=0; i<8; i++){
        //printf("i = %d \n", i);
        in = 0;
        for (j=0; j<frames_available; j++) {
            if (frames[j] == reference_string[i]) 
                in = 1;
        }
        //printf("in is %d \n", in);
        if (in == 1){//current string is inside the current frames
            skip = 0;
            printf(template_no_page_fault, reference_string[i]);
            for (j=0; j<frames_available-1; j++){
                if (queue_select(adjacent_queue, j) != reference_string[i] && skip == 0) {
                    //printf("Continued! \n");
                    continue;
                }
                if (queue_select(adjacent_queue, j) == reference_string[i]) {
                    //printf("Same! \n");
                    skip = 1;
                }
                if (skip == 1){
                    queue_replace(adjacent_queue, j, queue_select(adjacent_queue, j+1));
                    //printf("Replaced! \n");
                    //queue_printt(adjacent_queue);
                }
            }
            if (skip == 1)
            {
                queue_replace(adjacent_queue, position-1, reference_string[i]);
                //printf("Last one! \n");
                //queue_printt(adjacent_queue);
            }
            //printf("Done! \n");
            //printf("\n\n\n");
        }
        if (in == 0){
            fault += 1;
            if (queue_is_ful(adjacent_queue) == 0){
                queue_enqueue(adjacent_queue, reference_string[i]);
                frames[position] = reference_string[i];
                position += 1;
            }
            else if (queue_is_ful(adjacent_queue) != 0){
                peak = queue_peek(adjacent_queue);
                for (j=0; j<frames_available; j++) {
                    if (frames[j] == peak) {
                    frames[j] = reference_string[i];
                    }
                }

                queue_dequeue(adjacent_queue);

                queue_enqueue(adjacent_queue, reference_string[i]);
            }
            display_fault_frame(reference_string[i]);
            //printf("\n\n\n");
        }
    }
    //queue_enqueue(temp_queue, 1);
    //display_fault_frame(temp_queue);
    
    printf(template_total_page_fault, fault);
}

void initialize_frames() {
    int i;
    for (i=0; i<frames_available; i++)
        frames[i] = UNFILLED_FRAME;
}



int main() {
    parse_input();
    print_parsed_values();
    initialize_frames();
    if (strcmp(algorithm, ALGORITHM_FIFO) == 0) {
        algorithm_FIFO();
    } 
    else if (strcmp(algorithm, ALGORITHM_OPT) == 0) {
        algorithm_OPT();
    }
    else if (strcmp(algorithm, ALGORITHM_LRU) == 0) {
        algorithm_LRU();
    }    
    return 0;
}