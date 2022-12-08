/*
    COMP3511 Fall 2022 
    PA2: Simplified Multi-Level Feedback Queue (MLFQ)_Final

    Your name: Liangyawei Kuang
    Your ITSC email:           lkuang@connect.ust.hk 

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

// Define MAX_NUM_PROCESS
// For simplicity, assume that we have at most 10 processes

#define MAX_NUM_PROCESS 10
#define MAX_QUEUE_SIZE 10
#define MAX_PROCESS_NAME 5
#define MAX_GANTT_CHART 300

// Keywords (to be used when parsing the input)
#define KEYWORD_TQ0 "tq0"
#define KEYWORD_TQ1 "tq1"
#define KEYWORD_PROCESS_TABLE_SIZE "process_table_size"
#define KEYWORD_PROCESS_TABLE "process_table"

// Assume that we only need to support 2 types of space characters: 
// " " (space), "\t" (tab)
#define SPACE_CHARS " \t"

// Process data structure
// Helper functions:
//  process_init: initialize a process entry
//  process_table_print: Display the process table
struct Process {
    char name[MAX_PROCESS_NAME];
    int arrival_time ;
    int burst_time;
    int remain_time; // remain_time is needed in the intermediate steps of MLFQ 
};
void process_init(struct Process* p, char name[MAX_PROCESS_NAME], int arrival_time, int burst_time) {
    strcpy(p->name, name);
    p->arrival_time = arrival_time;
    p->burst_time = burst_time;
    p->remain_time = 0;
}
// The MOST important print in this file, clarifying wtf are printed
void process_table_print(struct Process* p, int size) {
    int i;
    printf("Process\tArrival\tBurst\n");
    for (i=0; i<size; i++) {
        printf("%s\t%d\t%d\n", p[i].name, p[i].arrival_time, p[i].burst_time);
    }
}

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
    return q->count == MAX_QUEUE_SIZE; //10
}

int queue_peek(struct Queue* q) {
    return q->values[q->front];
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
    printf("queue size is %d ", c);
    int cur = q->front;
    printf(", and queue values are ");
    while ( c > 0 ) {
        if ( cur == MAX_QUEUE_SIZE )
            cur = 0;
        printf("%d ", q->values[cur]);
        cur++;
        c--;
    }
    printf("\n");
}

// A simple GanttChart structure
// Helper functions:
//   gantt_chart_update: append one item to the end of the chart (or update the last item if the new item is the same as the last item)
//   gantt_chart_print: display the current chart
struct GanttChartItem {
    char name[MAX_PROCESS_NAME];
    int duration;
};
void gantt_chart_update(struct GanttChartItem chart[MAX_GANTT_CHART], int* n, char name[MAX_PROCESS_NAME], int duration) {
    int i;
    i = *n;
    // The new item is the same as the last item
    if ( i > 0 && strcmp(chart[i-1].name, name) == 0) 
    {
        chart[i-1].duration += duration; // update duration
    } 
    else
    {
        strcpy(chart[i].name, name);
        chart[i].duration = duration;
        *n = i+1;
    }
}
void gantt_chart_print(struct GanttChartItem chart[MAX_GANTT_CHART], int n) {
    int t = 0;
    int i = 0;
    printf("Gantt Chart = ");
    printf("%d ", t);
    for (i=0; i<n; i++) {
        t = t + chart[i].duration;     
        printf("%s %d ", chart[i].name, t);
    }
    printf("\n");
}

// Global variables
int tq0 = 0, tq1 = 0;
int process_table_size = 0;
struct Process process_table[MAX_NUM_PROCESS];


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
    char *three_tokens[3]; // buffer for 3 tokens
    int numTokens = 0, n=0, i=0;
    char equal_plus_spaces_delimiters[5] = "";

    char process_name[MAX_PROCESS_NAME];
    int process_arrival_time = 0;
    int process_burst_time = 0;

    strcpy(equal_plus_spaces_delimiters, "=");
    strcat(equal_plus_spaces_delimiters,SPACE_CHARS);

    while ( (nread = getline(&line, &len, fp)) != -1 ) {
        if ( is_skip(line) == 0)  {
            line = strtok(line,"\n");

            if (strstr(line, KEYWORD_TQ0)) {
                // parse tq0
                parse_tokens(two_tokens, line, &numTokens, equal_plus_spaces_delimiters);
                if (numTokens == 2) {
                    sscanf(two_tokens[1], "%d", &tq0);
                }
            } 
            else if (strstr(line, KEYWORD_TQ1)) {
                // parse tq1
                parse_tokens(two_tokens, line, &numTokens, equal_plus_spaces_delimiters);
                if (numTokens == 2) {
                    sscanf(two_tokens[1], "%d", &tq1);
                }
            }
            else if (strstr(line, KEYWORD_PROCESS_TABLE_SIZE)) {
                // parse process_table_size
                parse_tokens(two_tokens, line, &numTokens, equal_plus_spaces_delimiters);
                if (numTokens == 2) {
                    sscanf(two_tokens[1], "%d", &process_table_size);
                }
            } 
            else if (strstr(line, KEYWORD_PROCESS_TABLE)) {

                // parse process_table
                for (i=0; i<process_table_size; i++) {

                    getline(&line, &len, fp);
                    line = strtok(line,"\n");  

                    sscanf(line, "%s %d %d", process_name, &process_arrival_time, &process_burst_time);
                    process_init(&process_table[i], process_name, process_arrival_time, process_burst_time);

                }
            }

        }
        
    }
}
// Helper: Display the parsed values
void print_parsed_values() {
    printf("%s = %d\n", KEYWORD_TQ0, tq0);
    printf("%s = %d\n", KEYWORD_TQ1, tq1);
    printf("%s = \n", KEYWORD_PROCESS_TABLE);
    process_table_print(process_table, process_table_size);
}


// TODO: Implementation of MLFQ algorithm

int min(int a, int b){
    return a>b?b:a;
}

void mlfq() {

    // Initialize the gantt chart
    struct GanttChartItem chart[MAX_GANTT_CHART];
    int sz_chart = 0; //size of chart
    // TODO: implement your MLFQ algorithm here
    // printf("tq0 %d,\ntq1 %d,\n", tq0, tq1);
    //printf("%d \n", process_table[1].arrival_time);
    //printf("%d \n", process_table_size);
    // printf("initialization of 4 queues. \n");    
    struct Queue queue0[MAX_QUEUE_SIZE];
    struct Queue queue1[MAX_QUEUE_SIZE];
    struct Queue queue2[MAX_QUEUE_SIZE];
    struct Queue queue[MAX_QUEUE_SIZE];
    queue_init(queue0); // RR1
    queue_init(queue1); // RR2
    queue_init(queue2); // FCFS 
    queue_init(queue); // beginning queue
    //printf("%d \n", queue0[0].rear);
    
    
    // queue_enqueue(queue0, 0);
    // process_table[0].remain_time = tq0;
    // printf("queue0: "); queue_print(queue0);
    
    
    
    int t = 0; // start time
    int sum = 0; // sum of burst time
    int i = 0; // iteration variable
    //int duration = 0; 

    for (i=0; i<process_table_size; i++) sum += process_table[i].burst_time;
    // printf("sum of burst time is %d \n", sum);

    for (i=0; i<process_table_size; i++) queue_enqueue(queue, i+1); // add all processes to beginning queue
    // printf("beginning queue: "); queue_print(queue);

    // printf("\n\n");
    // printf("%d \n", !queue_is_empty(queue0));

    //int it = 0;

    //int n_dur = 0;
    //int left0 = tq0, left1 = tq1;
    //while((!queue_is_empty(queue0))||(!queue_is_empty(queue1))||(!queue_is_empty(queue2)))
    while(t <= sum)
    {
    //    it++;
    // printf("t = %d\n", t);
    // printf("before processed \n");
    // printf("queue: \n");queue_print(queue);printf("\n");
    // printf("queue0: \n");queue_print(queue0);printf("\n");
    // printf("queue1: \n");queue_print(queue1);printf("\n");
    // printf("queue2: \n");queue_print(queue2);printf("\n");
    
    //
    if (!queue_is_empty(queue0))
    {
        if (process_table[queue_peek(queue0)-1].remain_time != 0){
            //gantt_chart_print(chart, sz_chart);
            //queue_print(queue0);printf("\n");
            //printf("%d %s \n", queue_peek(queue0), process_table[queue_peek(queue0)].name);
            gantt_chart_update(chart, &sz_chart, process_table[queue_peek(queue0)-1].name, 1);
            //gantt_chart_print(chart, sz_chart);
            process_table[queue_peek(queue0)-1].burst_time -= 1;
            process_table[queue_peek(queue0)-1].remain_time -= 1;
            // printf("queue0: first process still runs!\n");
        }
        if (process_table[queue_peek(queue0)-1].remain_time == 0 && process_table[queue_peek(queue0)-1].burst_time != 0){
            process_table[queue_peek(queue0)-1].remain_time = tq1;
            queue_enqueue(queue1, queue_peek(queue0));
            queue_dequeue(queue0);
            // printf("queue0: queue1 updates!\n");
        }
        else if (process_table[queue_peek(queue0)-1].burst_time == 0){
            queue_dequeue(queue0);
            // printf("queue0: first process runs out!\n");
        }   
        // printf("queue0 processed!\n\n");
    }
    else if (!queue_is_empty(queue1))
    {

        if (process_table[queue_peek(queue1)-1].remain_time != 0){
            gantt_chart_update(chart, &sz_chart, process_table[queue_peek(queue1)-1].name, 1);
            process_table[queue_peek(queue1)-1].burst_time -= 1;
            process_table[queue_peek(queue1)-1].remain_time -= 1;
            // printf("queue1: first process still runs!\n");
        }
        if (process_table[queue_peek(queue1)-1].remain_time == 0 && process_table[queue_peek(queue1)-1].burst_time != 0){
            process_table[queue_peek(queue1)-1].remain_time = process_table[queue_peek(queue1)-1].burst_time;
            queue_enqueue(queue2, queue_peek(queue1));
            queue_dequeue(queue1);
            // printf("queue1: queue2 updates!\n");
        }
        else if (process_table[queue_peek(queue1)-1].burst_time == 0){
            queue_dequeue(queue1);
            // printf("queue1: first process runs out!\n");
        }
        // printf("queue1 processed!\n\n");
    }
    else if (!queue_is_empty(queue2))
    {
        gantt_chart_update(chart, &sz_chart, process_table[queue_peek(queue2)-1].name, 1);
        process_table[queue_peek(queue2)-1].burst_time -= 1;
        process_table[queue_peek(queue2)-1].remain_time -= 1;
        if (process_table[queue_peek(queue2)-1].remain_time == 0){
            queue_dequeue(queue2);
        }
        // printf("queue2 processed!\n\n");
    }

        // if process comes at time t from the beginning time
    if (process_table[queue_peek(queue)-1].arrival_time == t)
    {
        process_table[queue_peek(queue)-1].remain_time = tq0;
        queue_enqueue(queue0, queue_peek(queue));
        queue_dequeue(queue);
        // printf("queue: first process arrives!\n\n");
    }

    // printf("after processed \n");
    // printf("queue: \n");queue_print(queue);printf("\n");
    // printf("queue0: \n");queue_print(queue0);printf("\n");
    // printf("queue1: \n");queue_print(queue1);printf("\n");
    // printf("queue2: \n");queue_print(queue2);printf("\n");

    //gantt_chart_print(chart, sz_chart);
    // printf("\n");
    t++;
    // printf("t update!, and t is %d \n\n\n\n", t);
    //if (t == 9) break;
    }
    gantt_chart_print(chart, sz_chart);
    // 
    //below are trash codes
    //


    //int left1 = tq1 - chart[sz_chart-1].duration;
    // if (!queue_is_empty(queue0))
    // {
    //     //printf("left0: %d\n", left0);
    //     printf("queue arrive left is %d, queue0 burst is %d \n", (process_table[queue_peek(queue)].arrival_time-t), process_table[queue_peek(queue0)].burst_time);
    //     duration = min(tq0 ,min(process_table[queue_peek(queue)].arrival_time-t,process_table[queue_peek(queue0)].burst_time));
    //     printf("current duration = %d\n", duration);
        

    //     if (process_table[queue_peek(queue0)].burst_time==duration){
    //         gantt_chart_update(chart, &sz_chart, process_table[queue_peek(queue0)].name, duration);
    //         //t += duration;
    //         //left0 = tq0;
    //         queue_dequeue(queue0);
    //         printf("queue0 after dequeue");
    //         gantt_chart_print(chart, sz_chart);
    //     }
    //     if ((process_table[queue_peek(queue0)].burst_time > duration) && (left0 == 0)){
    //         queue_enqueue(queue1, queue_peek(queue0));
    //         gantt_chart_update(chart, &sz_chart, process_table[queue_peek(queue0)].name, duration);
    //         //t += duration;
    //         queue_dequeue(queue0);
    //         process_table[queue_peek(queue1)].burst_time -= duration;
    //         //printf("burst time: %d\n", process_table[queue_peek(queue1)].burst_time);
    //         process_table[queue_peek(queue1)].remain_time -= duration;
    //         //left0 = tq0;
    //     }
    //     if ((process_table[queue_peek(queue)].arrival_time-t)==duration){
    //         printf("enqueued %d\n", queue_peek(queue));
    //         process_table[queue_peek(queue)].remain_time = tq0;
    //         //t += duration;
    //         left0 -= duration;
    //         gantt_chart_update(chart, &sz_chart, process_table[queue_peek(queue0)].name, duration);
    //         queue_enqueue(queue0, queue_peek(queue));
    //         queue_dequeue(queue);
    //     }
        
    //     //sz_chart += 1;
    //     // left0 = tq0 - chart[sz_chart-1].duration;
    //     // if (left0 == 0)
    //     //     left0 = tq0;

    //     t += duration;
    // }
    // else if (!queue_is_empty(queue1))
    // {
    //     printf("queue1:");
    //     queue_print(queue1);
    //     duration = min(left1,min((process_table[queue_peek(queue)].arrival_time-t),process_table[queue_peek(queue1)].burst_time));
    //     printf("duration = %d\n", duration);

    //     //gantt_chart_update(chart, &sz_chart, process_table[queue_peek(queue1)].name, duration);
    //     if (process_table[queue_peek(queue1)].burst_time==duration){
    //         gantt_chart_update(chart, &sz_chart, process_table[queue_peek(queue1)].name, duration);
    //         //t += duration;
    //         left1 = tq1;
    //         queue_dequeue(queue1);
    //     }
    //     if (process_table[queue_peek(queue1)].burst_time > duration && (0==left1)){
    //         queue_enqueue(queue2, queue_peek(queue1));
    //         gantt_chart_update(chart, &sz_chart, process_table[queue_peek(queue1)].name, duration);
    //         //t += duration;
    //         left1 = tq1;
    //         queue_dequeue(queue1);
    //         process_table[queue_peek(queue2)].burst_time -= duration;
    //         process_table[queue_peek(queue2)].remain_time -= duration;
    //     }
    //     if ((process_table[queue_peek(queue)].arrival_time-t)==duration){
    //         process_table[queue_peek(queue)].remain_time = tq1;
    //         left1 -= duration;
    //         gantt_chart_update(chart, &sz_chart, process_table[queue_peek(queue1)].name, duration);
    //         queue_enqueue(queue1, queue_peek(queue));
    //         queue_dequeue(queue);
    //     }
        
    //     //sz_chart += 1;
    //     // left1 = tq1 - chart[sz_chart-1].duration;
    //     // if (left1 == 0)
    //     //     left1 = tq1;

    //     t += duration;
    // }
    // else if(!queue_is_empty(queue2))
    // {
    //     duration = process_table[queue_peek(queue1)].burst_time;
    //     queue_dequeue(queue2);
    //     gantt_chart_update(chart, &sz_chart, process_table[queue_peek(queue2)].name, duration);
    //     //sz_chart += 1;
    //     t += duration;
    // }

    // printf("queue: \n");queue_print(queue);
    // printf("queue0: \n");queue_print(queue0);
    // printf("queue1: \n");queue_print(queue1);
    // printf("queue2: \n");queue_print(queue2);
    
    // gantt_chart_print(chart, sz_chart);
    // printf("\n\n");
    // // if (it == 4)
    // //     break;
    // t++
    // }
    //gantt_chart_update(chart, &sz_chart, process_table[queue_peek(queue0)].name, duration);
    // At the end, uncomment this line to display the final Gantt chart
    
}


int main() {
    parse_input();
    print_parsed_values();
    // printf("here \n");
    mlfq();
    return 0;
}
