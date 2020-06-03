#include <stdio.h>
#include <stdlib.h>

struct process
{
    int bt;               //burst_time
    int at;               //arrival_time
    int pr;               //priority of the process; higher pr indicates better priority
    int bt_temp;          //this is initially same as the burst time but it keeps decreasing every time process is scheduled and will eventually indicate termination of a process
    int id;               //id to recognize each process
    int run;              //this stores run duration of processes in run-state
    struct process *next; //this holds the next process node on the linked list
};
struct answer //Once we have turnAround_time, wait_time for each process, we shall store them in here
{
    int P_id;
    int wait_time;
    int turn_around_time;
};
struct answer answers[6];

void createLinkedList(struct process **head_ref, struct process *new_node); //this function will insert each individual process in the linked list based on its priority.
struct process *newProcess(int id, int pr, int bt, int at);                 //this function will create a process node using input from the file and return the pointer to newly created node
struct process *newRunProcess(struct process **head_ref);                   //this function will return highest priority process from the queue while also pointing the head to new process on the ready queue

struct process *head;
struct process *temp;
struct process *run_state = NULL;

int main()
{
    head = (struct process *)malloc(sizeof(struct process));
    temp = (struct process *)malloc(sizeof(struct process));
    int q = 10; //time quantum
    int counter = 0;
    for (int time = 0; time < 95; time++) //This will 95 times since we expect all of our processes to be done by 95
    {
        //Reading from file
        int pr, bt, at, id;
        FILE *fptr;
        if ((fptr = fopen("process.txt", "r")) == NULL)
        {
            printf("Failed to read file");
            exit(1); //exit if failed to read file
        }

        int c = fgetc(fptr);
        while (1)
        {
            fscanf(fptr, "%d\t%d\t%d\t%d\n", &id, &pr, &bt, &at);
            if (feof(fptr))
                break; //NOTE: I tried doing this condition check in the 'while' above, but it would read the last line twice, therefore I opted to use 'break' which breaks right after we come to end of file.

            if (time == at) //When arrival time of process will equal the current time
            {
                struct process *new_pro = newProcess(id, pr, bt, at); //'newProcess' function will return pointer to the newly created process struct
                createLinkedList(&head, new_pro);                     //Passing the new process pointer to this function which will concatenate the process to the linked list
                if (run_state == NULL)
                { //Here run_state is empty therefore the first process from the ready queue is brought to run_state
                    run_state = newRunProcess(&head);
                }
            }
            c = fgetc(fptr);
        }
        fclose(fptr); //Done reading from file, so we close the file
        if (head->pr > run_state->pr)
        { //Higher Priority: Here we preempt the running process because our ready queue contains process with higher priority
            temp = run_state;
            run_state = newRunProcess(&head);
            printf("\nContext Switch (Higher Priority) at: %ds Prempted: P%d  New_Process:P%d\n", time, temp->id, run_state->id);
            createLinkedList(&head, temp); //passing pointer to the pointer of head so that we can make change to head pointer itself.
        }
        if ((run_state->run == 10) && (run_state->pr == head->pr))
        { //Round-Robin: Prempting while a process has run for 10 units, which is the time quantum specified. This occurs when there is another process of the same priority waiting in the queue.
            temp = run_state;
            run_state = newRunProcess(&head);
            printf("\nContext Switch (Round-Robin) at: %ds Prempted: P%d  New_Process:P%d\n", time, temp->id, run_state->id);
            createLinkedList(&head, temp);
        }

        run_state->bt_temp = run_state->bt_temp - 1;
        run_state->run = run_state->run + 1;
        if (run_state->bt_temp == 0)
        { //when a process runs and exhaust its burst-time, we terminate those process. We achieve it by simply replacing run_state by head of the queue.
            printf("\nCompletion of Process: P%d at %ds \n", run_state->id, (time + 1));

            //Before we release what's in the run_state, we calculate TAT and WT for these process on the verge of termination
            answers[counter].P_id = run_state->id;
            answers[counter].turn_around_time = (time + 1) - run_state->at;
            answers[counter].wait_time = (time + 1) - run_state->at - run_state->bt;
            counter = counter + 1;

            //freeing the process which is done running, and bringing in the process from ready queue
            run_state = newRunProcess(&head);
        }
    }
    printf("\n----------------------------------------------------\n");
    printf("\nProcess\t   Turn_Around_Time\tWait_time");
    for (int z = 0; z < counter; z++)
    {
        printf("\n  P%d ", answers[z].P_id);
        printf("\t\t%d   ", answers[z].turn_around_time);
        printf("\t\t   %d \n", answers[z].wait_time);
    }
    printf("\n----------------------------------------------------\n\n");
}

struct process *newRunProcess(struct process **head_ref) //this function will return highest priority process from the queue while also pointing the head to new process on the ready queue
{
    // If linked list is empty
    if (*head_ref == NULL)
        return NULL;

    struct process *new_process =
        (struct process *)malloc(sizeof(struct process));
    new_process->id = (*head_ref)->id;
    new_process->pr = (*head_ref)->pr;
    new_process->bt = (*head_ref)->bt;
    new_process->at = (*head_ref)->at;
    new_process->run = 0;
    new_process->bt_temp = (*head_ref)->bt_temp;
    new_process->next = NULL;

    // Store head node
    struct process *tempr = *head_ref;
    *head_ref = tempr->next; // Change head
    free(tempr);             // free old head

    return new_process;
}

struct process *newProcess(int id, int pr, int bt, int at)
{
    /* allocating node */
    struct process *new_process =
        (struct process *)malloc(sizeof(struct process));

    /* putting in the data  */
    new_process->id = id;
    new_process->pr = pr;
    new_process->bt = bt;
    new_process->at = at;
    new_process->run = 0;
    new_process->bt_temp = bt;
    new_process->next = NULL;

    return new_process;
}

void createLinkedList(struct process **head_ref, struct process *new_node)
{ //this function will insert each individual process in the linked list based on its priority.

    struct process *current;
    /* Special case for the head end */
    if (*head_ref == NULL || (*head_ref)->pr < new_node->pr)
    {
        new_node->next = *head_ref;
        *head_ref = new_node;
    }
    else
    {
        /* Locate the node before the point of insertion */
        current = *head_ref;
        while (current->next != NULL &&
               current->next->pr >= new_node->pr)
        {
            current = current->next;
        }
        new_node->next = current->next;
        current->next = new_node;
    }
}
/*
void printList(struct process *head)
{
    struct process *temp = head;
    while (temp != NULL)
    {
        printf("P%d: %d  ", temp->id, temp->pr);
        temp = temp->next;
    }
}
*/