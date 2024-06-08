#include <stdio.h>
#include <string.h>

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

void printInorder(struct node* node, char* list,int a)
{
    // since the tree maker function is inserting left children on the right, we have modified the code so that it prints right to left
    if (node == NULL)
    {
        return;
    }
        
    printInorder(node->right_child,list,a);
    list[a++] = node->data;
    printInorder(node->left_child,list,a+1);
}

int main(){

   char *args[80 / 2 + 1];
   while(1)
   {
      printf("input>");
      fflush(stdout);
      char str[100];

      // getting input
      fgets(str,100,stdin);
      str[strcspn(str, "\n")] = 0;

      char* token = strtok(str, " ");
            int k = 0;  
            while (token != NULL) {
                args[k] = token;
                token = strtok(NULL, " ");
                k++;
            }
            args[k] = "\0";

      if (strcmp(args[0], "Tree") == 0)
      {
         char *usertree[100];
         strcpy(usertree,args[1]);
         struct node *r = tree_maker(usertree);
         char *list[3];
         printInorder(r,list,0);
         
         int limit = find_length(list);
         for (int i = 0; i< limit; i++)
         {
            // printf("element");
            
            char word[2];
            // strncpy(word, list[i], 2);
            printf("%s",word);
         }

        // Tree (root,(first,(),()),(last,(),()))

        printf("%s \n",r->data);
      }

   }
   return 0;
}