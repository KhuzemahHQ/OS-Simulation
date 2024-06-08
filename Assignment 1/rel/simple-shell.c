// Khuzemah Hassan Qazi (24100092)
// Qaboos Ali Khan (24100153)

/**
 * 
 * Simple shell interface program.
 *
 * Operating System Concepts - Tenth Edition
 * Copyright John Wiley & Sons - 2018
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#include <sys/types.h> 
#include <sys/wait.h> 
#include <sys/stat.h>

#define MAX_LINE 80 /* 80 chars per line, per command */

struct node
{
    char *data;
    struct node *left_child;
    struct node *right_child;
};

struct tree
{
    struct node *root_pointer;
};

int find_length(char *arr)
{
    int counter = 0;
    int i = 0;
    while (arr[i] != '\0')
    {
        counter = counter + 1;
        i = i + 1;
    }
    // printf("length is %d", counter);
    return counter;
}

char *parser(char *arr, int starting_index)
{
    char *return_array = (char *)malloc(30 * sizeof(char));
    int i = 0;
    for (i = 0; i < 30; i = i + 1)
    {
        return_array[i] = '\0';
    }
    return_array[0] = '(';

    i = 1;
    int j = starting_index + 1;
    int count = 1;
    while (count != 0)
    {
        return_array[i] = arr[j];
        if (arr[j] == '(')
        {
            count = count + 1;
        }
        if (arr[j] == ')')
        {
            count = count - 1;
        }
        i = i + 1;
        j = j + 1;
    }

    return return_array;
}

struct node *tree_maker(char *args)
{
    // printf("\nargs are %s\n", args);
    struct node *root = malloc(sizeof(struct node));
    /////////////////////////////////////////
    //////////////////////////////////////
    ///////////////////////////////////////
    int start = 1;
    for (start = 1; args[start] != ','; start = start + 1)
    {
    }
    int number_chars = start - 1;
    ////now allocate space
    root->data = (char *)malloc((number_chars + 1) * sizeof(char));

    for (int dd = 0; dd < number_chars; dd = dd + 1)
    {
        root->data[dd] = args[1 + dd];
    }
    root->data[number_chars] = '\0';
    ///////////////////////////////////////////
    //////////////////////////////////////
    ////////////////////////////////////////////
    // printf("the number of characters are %d", number_chars);
    start = start + 1;
    // printf("start is %d", start);
    char *right_args = parser(args, start);
    int p_two = find_length(right_args) + start + 1;
    char *left_args = parser(args, p_two);

    int right_length = find_length(right_args);
    int left_length = find_length(left_args);
    // printf("right args are %s\n", right_args);
    // printf("left args are %s\n", left_args);

    if (right_length == 2)
    {
        root->right_child = NULL;
        // return root.right_child;
    }
    else
    {

        root->right_child = tree_maker(right_args);
        // return root.right_child;
    }

    if (left_length == 2)
    {

        root->left_child = NULL;
        // return root.left_child;
    }
    else
    {

        root->left_child = tree_maker(left_args);
        // return root.left_child;
    }

    struct node *left_leaf = (root->left_child);
    struct node *right_leaf = root->right_child;

    /*
    if (root->left_child)
    {
        printf("the data in roots' sleft child is %s", left_leaf->data);
    }
    if (root->right_child)
    {
        printf("the data in roots' right child is %s", right_leaf->data);
    }
    */

    return root;
}

// Linked List Code
struct llnode {
   char data[100];
   int key;
   struct llnode *next;
};

struct llnode *head = NULL;
struct llnode *current = NULL;

//display the history list
void printList() {
   struct llnode *ptr = head;

   if (head != NULL){
    while(ptr->next != ptr) {
      printf("\n %s ",ptr->data);
      fflush(stdout);
      ptr = ptr->next;
   }	
   printf("\n %s",ptr->data);
   fflush(stdout);
   }
    else{
        printf("There is no history \n");
        fflush(stdout);
    }
   
}

//insert link at the first location
void insert(int key, char data[]) {
   struct llnode *link = (struct node*) malloc(sizeof(struct llnode));
   link->key = key;
   strcpy(link->data, data);
	
   if (head == NULL) {
      head = link;
      head->next = head;
   } else {
      
      link->next = head;
      head = link;
   }    
}

struct llnode* search(int index){

    struct llnode *ptr = head;

    if (head != NULL)
    {
        int counter = 1;
        while(counter != index) 
        {
            ptr = ptr->next;
            counter++;    
        }
        return ptr;
    
    }
    else{
        printf("There is no history \n");
        fflush(stdout);
    }
    
}


void printInorder(struct node* node, char* treelist,int a)
{
    // since the tree maker function is inserting left children on the right, we have modified the code so that it prints right to left
    if (node == NULL)
    {
        return;
    }
        
    printInorder(node->right_child,treelist,a);
    treelist[a++] = node->data;
    printInorder(node->left_child,treelist,a+1);
}

int main(void)
{
    char *args[MAX_LINE / 2 + 1]; /* command line (of 80) has max of 40 arguments */
    int should_run = 1;

    while (should_run)
    {
        printf("osh>");
        fflush(stdout);
        char str[100];
        char str2[100];

        // getting input
        fgets(str,100,stdin);
        str[strlen(str)-1]='\0';

        char detect1[10];     
        char detect2[10];

        strncpy(detect1, str, 2);
        detect1[2] = 0; 
        strncpy(detect2, str, 4);
        detect2[4] = 0; 
        if (strcmp(detect1, "!!") == 0)
        {
            // don't add this to history
        }
        else if (strcmp(detect2, "hist") == 0)
        {
            // don't add this to history
        }
        else if (strcmp(detect2, "exit") == 0)
        {
            exit(1);
        }
        else
        {
            // adding whole input to history

            insert(should_run,str);
        }
        
        runn: ;
        char*comm_list[40];
        char* comm = strtok(str,"&&");
        int i = 0;  
        while (comm != NULL) {
            comm_list[i] = comm;
            comm = strtok(NULL, "&&");
            i++;
        }
        comm_list[i] = comm;

        for (int j=0;j<i;j++){

            char *pipeline[MAX_LINE / 2 + 1];
            char* comp = strtok(comm_list[j], "|");
            int l = 0;  
            while (comp != NULL) {
                pipeline[l] = comp;
                comp = strtok(NULL, "|");
                l++;
            }
            pipeline[l] = comp;
            

            if (l>1)
            {

                char cmd1[100];
                strcpy(cmd1,pipeline[0]);
                char *args1[MAX_LINE / 2 + 1];
                char* token1 = strtok(cmd1, " ");    
                int k2 = 0;  
                while (token1 != NULL) {
                    args1[k2]=token1;
                    token1 = strtok(NULL, " ");
                    k2++;
                }
                args1[k2] = token1;

                char cmd2[100];
                strcpy(cmd2,pipeline[1]);
                char *args2[MAX_LINE / 2 + 1];
                char* token2 = strtok(cmd2, " ");    
                int k3 = 0;  
                while (token2 != NULL) {
                    args2[k3] = token2;
                    token2 = strtok(NULL, " ");
                    k3++;
                }
                args2[k3] = token2;

                pid_t pid;

                int dir[2];

                if (pipe(dir) < 0)
                {
                    printf("Error in piping \n");
                    exit(1);
                }

                pid = fork();
                if (pid == 0 )
                {

                    dup2(dir[1],STDOUT_FILENO);
                    
                    execvp(args1[0],args1);
                    exit(1);
                }
                else if (pid > 0)
                {
                    waitpid(pid,NULL,0);
                    pid = fork();
                    if (pid == 0)
                    {

                        dup2(dir[0],STDIN_FILENO);
                        close(dir[1]);
                        close(dir[0]);
                        
                        execvp(args2[0],args2);
                        exit(1);
                        
                    }
                    else if (pid > 0)
                    {
                        int status;
                        close(dir[0]);
                        close(dir[1]);
                        while ((pid = wait(&status)) > 0);
                    }
                    else
                    {
                        printf("Can't fork");
                    }
                }
                    
            }
            else
            {
                // seperating each command by spaces
                char* token = strtok(comm_list[j], " ");    
                int k = 0;  
                while (token != NULL) {
                    args[k] = token;
                    token = strtok(NULL, " ");
                    k++;
                }
                args[k] = token;

                if (strcmp(args[0], "!!") == 0)
                {
                    
                    printf("%s",head->data);
                    printf("\n");
                    fflush(stdout);

                    char *last_command[100];
                    strcpy(last_command,head->data);
                    insert(should_run,last_command);

                    strcpy(str,last_command);
                    goto runn;

                }
                else if (strcmp(args[0], "Tree") == 0)
                {
                    char *usertree[100];
                    strcpy(usertree,args[1]);
                    tree_maker(usertree);

                    // make inorder traversal
                    char *usertree[100];
                    strcpy(usertree,args[1]);
                    struct node *r = tree_maker(usertree);
                    char *list[40];
                    printInorder(r,list,0);
                    
                    int limit = find_length(list);

                    // make pipelines
                    int limit = find_length(list);

                    // unable to complete this part

                    // goto runn;

                }

                else if (strcmp(args[0], "hist") == 0)
                {

                    if (k > 1)
                    {
                        int current_idx = head->key;
                        int x = atoi(args[1]);
                        x = -x;
                        if (x > current_idx)
                        {
                            printf("No command found \n");
                            fflush(stdout);
                        }
                        else
                        {

                            char *target[100];
                            strcpy(target,search(x)->data);
                            insert(should_run,target);
                            printf("%s",target);
                            printf("\n");
                            fflush(stdout);

                            strcpy(str,target);
                            goto runn;

                        }
                        
                    }
                    else
                    {
                        printList();
                        printf("\n");   
                        fflush(stdout);
                    }
                    
                }
                
                pid_t bruh = fork();
                
                if (bruh == 0){
                    execvp(args[0],args);
                    exit(1);
                }
                else if (bruh < 0){
                    printf("Error in forking");
                    fflush(stdout);
                }
                else{
                    pid_t w;
                    while ((w = wait(NULL)) > 0);
                }

            }
            
        }

        pid_t w2;
        while ((w2 = wait(NULL)) > 0);

        /**
         * After reading user input, the steps are:
         * (1) fork a child process
         * (2) the child process will invoke execvp()
         * (3) if command includes &, parent and child will run concurrently
         */
        should_run++;
    }
    printf("exited loop for some reason");
    fflush(stdout);

    return 0;
}
