#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MIN_ENTRIES 2
#define MAX_ENTRIES 4

struct point {
    int x;
    int y;
};
typedef struct point point;

struct rectangle {
    point bottom_left;
    point top_right;
};
typedef struct rectangle rectangle;


struct Node;

typedef struct arrayNodes {
    struct Node * parent;
    struct Node ** nodepointer; //descendants
    int count;
    bool is_leaf;
} arrayNodes;

typedef struct Node {
    rectangle * mbr;
    arrayNodes * childpointer; // Pointer to its descendants
    struct arrayNodes * array; 
    int lhv; 
} Node;

arrayNodes * createArrayNodes(bool is_leaf);

int h_value(rectangle * rect);

arrayNodes * choose_leaf(arrayNodes * root, rectangle * rect);

void HandleOverflow(arrayNodes * arr, rectangle * rect);

void AdjustTree(arrayNodes * arr, Node ** set, Node * nn);

Node ** getSetNodes(arrayNodes * arr, Node * nn);

rectangle * create_rectangle(point bottom_left, point top_right) 
{
    rectangle * rect = malloc(sizeof(rectangle));
    rect->bottom_left = bottom_left;
    rect->top_right = top_right;
    return rect;
}

Node* createNode(bool is_leaf,point low,point high)
{
    Node* newnode = (Node * ) malloc(sizeof(Node));
    newnode->mbr = create_rectangle(low,high);
    newnode->lhv = h_value(newnode->mbr);
    newnode->array = (arrayNodes *) malloc(sizeof(arrayNodes));
    if (!is_leaf)
        newnode->childpointer = (arrayNodes *) malloc(sizeof(arrayNodes));
    return newnode;
}

arrayNodes * createArrayNodes(bool is_leaf) 
{
    arrayNodes * array = (arrayNodes *) malloc(sizeof(arrayNodes));
    array->is_leaf = is_leaf;
    array->count = 0;
    array->nodepointer = (Node **) malloc(sizeof(Node *)*MAX_ENTRIES);

    for (int i = 0;i<MAX_ENTRIES;i++) 
    {
        array->nodepointer[i] = (Node *) malloc(sizeof(Node));
        array->nodepointer[i]->array = array;
    }
    array->parent = (Node *) malloc(sizeof(Node));
    array->parent = NULL;
    return array;
}


void rotate(int num, int *x, int *y, int rot_x, int rot_y) {
    
    if (rot_y == 0) 
    {
        if (rot_x == 1) 
        {
            *x = (num-1) - *x;
            *y = (num-1) - *y;
        }

        //Swap x and y
        int z  = *x;
        *x = *y;
        *y = z;
    }
}



int hilbert_value(int num, rectangle * rect) 
{
    int x = (rect->bottom_left.x+rect->top_right.x)/2;
    int y = (rect->bottom_left.y+rect->top_right.y)/2;
    int rotx, roty, i, j=0;
    
    for (i=num/2; i>0; i/=2) 
    {
        rotx = (x & i) > 0;
        roty = (y & i) > 0;
        j += i * i * ((3 * rotx) ^ roty);
        rotate(num, &x, &y, rotx, roty);
    }

    return j;
}

int h_value(rectangle * rect) 
{
    return hilbert_value(16,rect);
}

void create_mbr(arrayNodes * arr) 
{
    int x_min = arr->nodepointer[0]->mbr->bottom_left.x;
    int x_max = arr->nodepointer[0]->mbr->top_right.x;
    int y_min = arr->nodepointer[0]->mbr->bottom_left.y;
    int y_max = arr->nodepointer[0]->mbr->top_right.y;

    for (int i = 0;i<arr->count;i++) 
    {
        if (arr->nodepointer[i]->mbr->bottom_left.x < x_min) x_min = arr->nodepointer[i]->mbr->bottom_left.x;
        if (arr->nodepointer[i]->mbr->top_right.x > x_max) x_max = arr->nodepointer[i]->mbr->top_right.x;
        if (arr->nodepointer[i]->mbr->bottom_left.y < y_min) y_min = arr->nodepointer[i]->mbr->bottom_left.y;
        if (arr->nodepointer[i]->mbr->top_right.y > y_max) y_max = arr->nodepointer[i]->mbr->top_right.y;
    }

    arr->parent->mbr = create_rectangle((point) {x_min,y_min},(point) {x_max,y_max});

    return;
}

Node ** getSetNodes(arrayNodes * arr, Node * nn) 
{
    if (!arr->is_leaf) 
    {
        int total_count = 0;

        for (int i = 0;i<arr->count;i++) 
        {
            total_count += arr->nodepointer[i]->childpointer->count;
        }
        Node ** x = (Node **) malloc(sizeof(Node *)*(total_count+1));
        int flag = 1;
        int k = 0;
        int a = 0;
        
        for (int i = 0;i<arr->count;i++) 
        {
            for (int j = 0;j<arr->nodepointer[i]->childpointer->count;j++) 
            {
                x[k++] = arr->nodepointer[i]->childpointer->nodepointer[j];

                if (nn->lhv > arr->nodepointer[i]->childpointer->nodepointer[j]->lhv) 
                {
                    flag = 0;
                    a = k;
                }
            }
        }

        if (a == total_count) 
        {
            x[total_count] = nn;
        } 
        
        else 
        {
            for (int i = total_count;i>a;i--) 
            {
                x[i] = x[i-1];
            }
            x[a] = nn;
        }
        return x;
    } 
    
    else 
    {
        int total_count = MAX_ENTRIES;
        Node ** x = (Node **) malloc(sizeof(Node *)*(total_count+1));
        int flag = 1;
        int k = 0;
        int a = 0;

        for (int i = 0;i<arr->count;i++) 
        {
            x[k++] = arr->nodepointer[i];

            if (nn->lhv > arr->nodepointer[i]->lhv) 
            {
                flag = 0;
                a = k;
            }
        }

        if (a == total_count) 
        {
            x[total_count] = nn;
        } 
        
        else 
        {
            for (int i = total_count;i>a;i--) 
            {
                x[i] = x[i-1];
            }
            x[a] = nn;
        }
        return x;
    }
}

void HandleOverflowNode(arrayNodes * arr, Node * nn) 
{
    if (arr->nodepointer[0]->childpointer->is_leaf) 
    {
        rectangle * rect = nn->mbr;
        int total_count = 0;
        int hv = h_value(rect);
        int a,b;

        for (int i = 0;i<arr->count;i++) 
        {
            total_count += arr->nodepointer[i]->childpointer->count;
        }

        if (total_count == MAX_ENTRIES*MAX_ENTRIES) 
        {
            Node * y = createNode(true,rect->bottom_left,rect->top_right);
            Node ** y1 = (Node **) malloc(sizeof(Node *)*(total_count+1));
            y1 = getSetNodes(arr,y);
            
            AdjustTree(arr,y1,y);
        } 
        
        else 
        {
            rectangle ** x = (rectangle **) malloc(sizeof(rectangle *)*(total_count+1));
            int flag = 1;
            int k = 0;
            int a = 0;

            for (int i = 0;i<arr->count;i++) 
            {
                for (int j = 0;j<arr->nodepointer[i]->childpointer->count;j++) 
                {
                    x[k++] = arr->nodepointer[i]->childpointer->nodepointer[j]->mbr;

                    if (hv > h_value(arr->nodepointer[i]->childpointer->nodepointer[j]->mbr)) 
                    {
                        flag = 0;
                        a = k;
                    } 
                }
            }

            if (a == total_count) 
            {
                x[total_count] = rect;
            } 
            
            else 
            {
                for (int i = total_count;i>a;i--) 
                {
                    x[i] = x[i-1];
                }
                x[a] = rect;
            }

            for (int i = 0;i<arr->count;i++) 
            {
                free(arr->nodepointer[i]->childpointer);
            }
            total_count++;

            if (total_count % MAX_ENTRIES == 0) 
            {
                arr->count = total_count/MAX_ENTRIES;
            } 
            
            else 
            {
                arr->count = total_count/MAX_ENTRIES + 1;
            }
            arr->is_leaf = false;
            k = 0;
            int sum = 0;
            int j1 = total_count/arr->count;
            int j2 = total_count%arr->count;

            for (int i = 0;i<arr->count;i++) 
            {
                arr->nodepointer[i]->childpointer = (arrayNodes *) malloc(sizeof(arrayNodes));
                arr->nodepointer[i]->childpointer->is_leaf = true;
                arr->nodepointer[i]->childpointer->nodepointer = (Node **) malloc(sizeof(Node *)*MAX_ENTRIES);
                arr->nodepointer[i]->childpointer->parent = arr->nodepointer[i];

                for (int j = 0;j<MAX_ENTRIES;j++) 
                {
                    arr->nodepointer[i]->childpointer->nodepointer[j] = (Node *) malloc(sizeof(Node));
                }

                if (j2 == 0) 
                {
                    for (int j = 0;j<j1 && k<total_count;j++) 
                    {
                        arr->nodepointer[i]->childpointer->nodepointer[j]->mbr = x[k];
                        arr->nodepointer[i]->childpointer->nodepointer[j]->lhv = h_value(x[k]);
                        k++;
                    }
                    arr->nodepointer[i]->childpointer->count = j1;
                    arr->nodepointer[i]->lhv = arr->nodepointer[i]->childpointer->nodepointer[j1-1]->lhv;
                } 
                
                else 
                {
                    if (i < j2) 
                    {
                        for (int j = 0;j<j1+1 && k<total_count;j++) 
                        {
                            arr->nodepointer[i]->childpointer->nodepointer[j] = (Node *) malloc(sizeof(Node));
                            arr->nodepointer[i]->childpointer->nodepointer[j]->mbr = x[k];
                            arr->nodepointer[i]->childpointer->nodepointer[j]->lhv = h_value(x[k]);
                            k++;
                        }
                        arr->nodepointer[i]->childpointer->count = j1+1;
                        arr->nodepointer[i]->lhv = arr->nodepointer[i]->childpointer->nodepointer[j1]->lhv;
                    } 
                    
                    else 
                    {
                        for (int j = 0;j<j1 && k<total_count;j++) 
                        {
                            arr->nodepointer[i]->childpointer->nodepointer[j] = (Node *) malloc(sizeof(Node));
                            arr->nodepointer[i]->childpointer->nodepointer[j]->mbr = x[k];
                            arr->nodepointer[i]->childpointer->nodepointer[j]->lhv = h_value(x[k]);
                            k++;
                        }
                        arr->nodepointer[i]->childpointer->count = j1;
                        arr->nodepointer[i]->lhv = arr->nodepointer[i]->childpointer->nodepointer[j1-1]->lhv;
                    }
                }
                create_mbr(arr->nodepointer[i]->childpointer);
            }
            Node * par = arr->parent;
            while (par != NULL) 
            {
                par->lhv = arr->nodepointer[arr->count-1]->lhv;
                create_mbr(arr);
                par = par->array->parent;
            }
        }   
    } 
    
    else 
    {
        int total_count = 0;

        for (int i =0;i<arr->count;i++)
        {
            total_count += arr->nodepointer[i]->childpointer->count;
        }

        if (total_count == MAX_ENTRIES*MAX_ENTRIES) 
        {
            Node * y = createNode(true,nn->mbr->bottom_left,nn->mbr->top_right);
            Node ** y1 = (Node **) malloc(sizeof(Node *)*(total_count+1));
            y1 = getSetNodes(arr,y);
            AdjustTree(arr,y1,y);
        } 
        
        else 
        {
            Node ** x = getSetNodes(arr, nn);
            bool is_leaf = arr->nodepointer[0]->childpointer->is_leaf;
            
            for (int i = 0;i<arr->count;i++) 
            {
                free(arr->nodepointer[i]->childpointer);
            }
            total_count++;

            if (total_count % MAX_ENTRIES == 0) 
            {
                arr->count = total_count/MAX_ENTRIES;
            } 
            
            else 
            {
                arr->count = total_count/MAX_ENTRIES + 1;
            }

            arr->is_leaf = false;
            int k = 0;
            int sum = 0;
            int j1 = total_count/arr->count;
            int j2 = total_count%arr->count;

            for (int i = 0;i<arr->count;i++) 
            {
                arr->nodepointer[i]->childpointer = (arrayNodes *) malloc(sizeof(arrayNodes));
                arr->nodepointer[i]->childpointer->is_leaf = is_leaf;
                arr->nodepointer[i]->childpointer->nodepointer = (Node **) malloc(sizeof(Node *)*MAX_ENTRIES);
                arr->nodepointer[i]->childpointer->parent = arr->nodepointer[i];
                
                arr->nodepointer[i]->array = arr;

                for (int j = 0;j<MAX_ENTRIES;j++) 
                {
                    arr->nodepointer[i]->childpointer->nodepointer[j] = (Node *) malloc(sizeof(Node));
                }

                if (j2 == 0) 
                {
                    for (int j = 0;j<j1 && k<total_count;j++) 
                    {
                        arr->nodepointer[i]->childpointer->nodepointer[j] = x[k];
                        k++;
                    }

                    arr->nodepointer[i]->childpointer->count = j1;
                    arr->nodepointer[i]->lhv = arr->nodepointer[i]->childpointer->nodepointer[j1-1]->lhv;
                } 
                
                else 
                {
                    if (i < j2) 
                    {
                        for (int j = 0;j<j1+1 && k<total_count;j++) 
                        {
                            arr->nodepointer[i]->childpointer->nodepointer[j] = x[k];
                            k++;
                        }

                        arr->nodepointer[i]->childpointer->count = j1+1;
                        arr->nodepointer[i]->lhv = arr->nodepointer[i]->childpointer->nodepointer[j1]->lhv;
                    } 
                    
                    else 
                    {
                        for (int j = 0;j<j1 && k<total_count;j++) 
                        {
                            arr->nodepointer[i]->childpointer->nodepointer[j] = x[k];
                            k++;
                        }

                        arr->nodepointer[i]->childpointer->count = j1;
                        arr->nodepointer[i]->lhv = arr->nodepointer[i]->childpointer->nodepointer[j1-1]->lhv;
                    }
                }

                create_mbr(arr->nodepointer[i]->childpointer);
            }

            Node * par = arr->parent;
            while (par != NULL) {
                par->lhv = arr->nodepointer[arr->count-1]->lhv;
                create_mbr(arr);
                par = par->array->parent;
            }
        }
    }
}

void splitRoot(arrayNodes * arr, Node ** set) 
{
    if (!arr->is_leaf)  
    {
        arrayNodes * arr1 = createArrayNodes(false);
        arrayNodes * arr2 = createArrayNodes(false);

        int total_count = MAX_ENTRIES*MAX_ENTRIES+1;
        int k = 0;
        int j;

        for (int i = 0;i<3;i++) {
            arr1->nodepointer[i] = (Node *) malloc(sizeof(Node));
            arr1->nodepointer[i]->childpointer = createArrayNodes(arr->nodepointer[i]->childpointer->is_leaf);
            arr1->nodepointer[i]->childpointer->parent = arr1->nodepointer[i];
            arr1->nodepointer[i]->array = arr1;

            if (i == 2) j = 3;
            else j = 4;

            for (int k1 =0;k1<j && k<total_count;k1++) 
            {
                arr1->nodepointer[i]->childpointer->nodepointer[k1] = set[k];
                arr1->nodepointer[i]->childpointer->count++;
                k++;
            }

            arr1->nodepointer[i]->lhv = arr1->nodepointer[i]->childpointer->nodepointer[j-1]->lhv;
            create_mbr(arr1->nodepointer[i]->childpointer);
        }

        arr1->count = 3;

        for (int i = 0;i<2;i++) 
        {
            arr2->nodepointer[i] = (Node *) malloc(sizeof(Node));
            arr2->nodepointer[i]->childpointer = createArrayNodes(arr->nodepointer[i]->childpointer->is_leaf);
            arr2->nodepointer[i]->childpointer->parent = arr2->nodepointer[i];
            arr2->nodepointer[i]->array = arr2;
            
            j = 3;

            for (int k1 =0;k1<j && k<total_count;k1++) 
            {
                arr2->nodepointer[i]->childpointer->nodepointer[k1] = set[k];
                arr2->nodepointer[i]->childpointer->count++;
                k++;
            }

            arr2->nodepointer[i]->lhv = arr2->nodepointer[i]->childpointer->nodepointer[j-1]->lhv;
            create_mbr(arr2->nodepointer[i]->childpointer);
        }

        arr2->count = 2;

        arr->count = 2;
        arr->is_leaf = false;

        arr->nodepointer[0]->childpointer = arr1;
        arr1->parent = arr->nodepointer[0];
        arr->nodepointer[0]->lhv = arr1->nodepointer[arr1->count-1]->lhv;
        create_mbr(arr1);

        arr->nodepointer[1]->childpointer = arr2;
        arr2->parent = arr->nodepointer[1];
        arr->nodepointer[1]->lhv = arr2->nodepointer[arr2->count-1]->lhv;
        create_mbr(arr2);

        arr->parent = NULL;
        return;
    } 
    
    else 
    {
        arrayNodes * arr1 = createArrayNodes(true);
        arrayNodes * arr2 = createArrayNodes(true);

        int total_count = MAX_ENTRIES*MAX_ENTRIES+1;
        int k = 0;
        int j;

        for (int i = 0;i<3;i++) 
        {
            arr1->nodepointer[i] = (Node *) malloc(sizeof(Node));
            arr1->nodepointer[i]->array = arr1;
            arr1->nodepointer[i]->mbr = set[i]->mbr;
            arr1->nodepointer[i]->lhv = set[i]->lhv;
        }

        arr1->count = 3;

        for (int i = 0;i<2;i++) 
        {
            arr2->nodepointer[i] = (Node *) malloc(sizeof(Node));
            arr2->nodepointer[i]->array = arr2;
            arr2->nodepointer[i]->mbr = set[i+3]->mbr;
            arr2->nodepointer[i]->lhv = set[i+3]->lhv;
        }

        arr2->count = 2;

        arr->count = 2;
        arr->is_leaf = false;

        arr->nodepointer[0]->childpointer = arr1;
        arr1->parent = arr->nodepointer[0];
        arr->nodepointer[0]->lhv = arr1->nodepointer[arr1->count-1]->lhv;
        create_mbr(arr1);

        arr->nodepointer[1]->childpointer = arr2;
        arr2->parent = arr->nodepointer[1];
        arr->nodepointer[1]->lhv = arr2->nodepointer[arr2->count-1]->lhv;
        create_mbr(arr2);

        arr->parent = NULL;
        return;
    }
}

void AdjustTree(arrayNodes * arr, Node ** set, Node * nn) 
{
    if (arr->parent == NULL) 
    {
        splitRoot(arr, set);   // if there is no space upwards, we call the splitRoot function that creates a new node and changes the root.
    } 
    
    else 
    {
        arrayNodes * parent = arr->parent->array;
        int total_count = 0;

        for (int i = 0;i<parent->count;i++) 
        {
            total_count += parent->nodepointer[i]->childpointer->count;
        }
        Node * new_node = (Node *) malloc(sizeof(Node));
        new_node->childpointer = createArrayNodes(arr->nodepointer[0]->childpointer->is_leaf);
        new_node->childpointer->parent = new_node;
        new_node->childpointer->is_leaf = false;
        Node ** x = (Node **) malloc(sizeof(Node *) * (MAX_ENTRIES+1));

        int total = MAX_ENTRIES*MAX_ENTRIES+1;
        int k = 0;
        int j;

        for (int i = 0;i<MAX_ENTRIES+1;i++) 
        {
            x[i] = (Node *) malloc(sizeof(Node));
            x[i]->array = (arrayNodes *) malloc(sizeof(arrayNodes));
            x[i]->childpointer = createArrayNodes(arr->nodepointer[0]->childpointer->is_leaf);
            x[i]->childpointer->parent = x[i];

            if (i >= 2) j = 3;
            else j = 4;

            for (int k1 =0;k1<j && k<total;k1++) 
            {
                x[i]->childpointer->nodepointer[k1]->mbr = set[k]->mbr;
                x[i]->childpointer->nodepointer[k1]->lhv = set[k]->lhv;
                x[i]->childpointer->nodepointer[k1]->childpointer = set[k]->childpointer;
                x[i]->childpointer->nodepointer[k1]->array = x[i]->childpointer;
                x[i]->childpointer->count++;
                k++;
            }

            x[i]->lhv = x[i]->childpointer->nodepointer[j-1]->lhv;
            create_mbr(x[i]->childpointer);
        }
        for (int i = 0;i<5;i++) 
        {
            if (i<4) 
            {
                arr->nodepointer[i]->mbr = x[i]->mbr;
                arr->nodepointer[i]->childpointer = x[i]->childpointer;
                arr->nodepointer[i]->lhv = x[i]->lhv;
                arr->nodepointer[i]->array = arr;
            } 
            
            else 
            {
                new_node->childpointer->nodepointer[i-4]->mbr = x[i]->mbr;
                new_node->childpointer->nodepointer[i-4]->lhv = x[i]->lhv;
                new_node->childpointer->nodepointer[i-4]->array = new_node->childpointer;
                new_node->childpointer->nodepointer[i-4]->childpointer = x[i]->childpointer;
                new_node->childpointer->nodepointer[i-4]->childpointer->parent = new_node->childpointer->nodepointer[i-4];
                new_node->childpointer->count++;
                new_node->childpointer->parent = new_node;
            }
        }

        new_node->childpointer->count = 1;

        new_node->lhv = new_node->childpointer->nodepointer[new_node->childpointer->count-1]->lhv;
        create_mbr(new_node->childpointer);
        
        arr->count = 4;
        arr->parent->lhv = arr->nodepointer[arr->count-1]->lhv;
        
        create_mbr(arr);

        if (arr->parent->array->count <MAX_ENTRIES) 
        {
            HandleOverflowNode(arr->parent->array,new_node->childpointer->nodepointer[0]);
        }
 
    }
}

void insert(arrayNodes * arr, Node * n) 
{
    // check if the node we wish to insert in is a leaf or not
    if (arr->is_leaf) 
    {
        int hv = n->lhv;
        // if yes, checks if our root node is full or not 
        if (arr->count < MAX_ENTRIES) 
        {
            // if it's not full, we add the rectangle and store it in a sorted manner based on their hilbert values
            rectangle * rect = n->mbr;
            int hv = n->lhv;
            rectangle * x = arr->nodepointer[0]->mbr;

            for (int i = 0;i<arr->count;i++) 
            {
                x = arr->nodepointer[i]->mbr;
                if (hv < h_value(x)) 
                {
                    arr->nodepointer[i]->mbr = rect;
                    arr->nodepointer[i]->lhv = hv;
                    arr->nodepointer[i]->array = arr;
                    rect = x;
                    hv = h_value(rect);
                }
            }

            arr->nodepointer[arr->count]->array = arr;
            arr->nodepointer[arr->count]->lhv = hv;
            arr->nodepointer[arr->count++]->mbr = rect;
            

            // if the node is not the root node, we will update the parent's minimum bounding rectangle and the largest hilbert value stored
            Node * par = arr->parent;
            while (par != NULL) 
            {
                arr->parent->lhv = hv;
                create_mbr(par->childpointer);
                par = par->array->parent;
            }
            
        } 
        
        else 
        {   // if our node is a root node and is completely filled(4 entries), then calls the splitroot function 
            Node ** node_set = (Node **) malloc(sizeof(Node *) * (MAX_ENTRIES+1));

            node_set = getSetNodes(arr, n);
            splitRoot(arr, node_set);
        }
    } 
    
    else 
    {   // if our node is not a leaf node, we choose the best leaf arrayNode to insert the rectangle in
        arrayNodes * arr2 = choose_leaf(arr,n->mbr);

        // if our best leaf arrayNode is not full, we can call the insert function again to insert it directly
        if (arr2->count < MAX_ENTRIES){
            insert(arr2,n);
        } 
        
        // else we call HandleOverflowNode to check the sibling nodes for accommodation
        else 
        {
            HandleOverflowNode(arr2->parent->array,n);
        }
    }
}

int find_best_node(arrayNodes * arr, int h) 
{
        
    int i = 0;
    int best_node = -1;  // to store index of appropriate node in array

    for(i; i < arr->count; i++) 
    {
        if(h < arr->nodepointer[i]->lhv) 
        {
            if (i == 0) 
            {
                best_node = 0;
            }

// checking which LHV the hilbert value is closest to and assigning it as best_node

         else 
         {
              if ((h-arr->nodepointer[i-1]->lhv)<(arr->nodepointer[i]->childpointer->nodepointer[0]->lhv-h)) {
                    best_node = i-1; }
              else best_node = i;
          }
            break;
        }
    }

// if hilbert value exceeds all LHVs in current arrayNode then assigning node with highest LHV as best_node

    if (best_node == -1) 
    { 
        best_node = arr->count - 1;
    }
    
    return best_node;
}

arrayNodes * choose_leaf(arrayNodes * arr,rectangle * rect) 
{
    int h = h_value(rect); 

// descending down the tree by iteratively assigning the array of child nodes under the best node in the array as the new arr

    while (arr->is_leaf != true) 
    {
        arr = arr->nodepointer[find_best_node(arr, h)]->childpointer; 
    }

    return arr;
}

void preOrder(arrayNodes *node)
{
    for(int i=0;i<node->count;i++)
    {
        printf("Bottom Left: %d,%d",node->nodepointer[i]->mbr->bottom_left.x, node->nodepointer[i]->mbr->bottom_left.y);
        printf(" Top Right: %d,%d",node->nodepointer[i]->mbr->top_right.x, node->nodepointer[i]->mbr->top_right.y);                 //prints the coordinates of extremities of nodes, regardless of internal/external node
        
        if(node->is_leaf)
        {
            printf(" - Leaf Node\n");     //when it is a leaf node, it specifies so.
        }

        else
        {
            printf(" - Internal Node\n");  // specifies when external node.
            arrayNodes *x = node->nodepointer[i]->childpointer;
            preOrder(x);      // and traverses through its children in such a way that all of a nodes descendants are covered first before printing the next node in the arrayNode
        }
    }
}

arrayNodes * insertData()
{

    char * filename = "data.txt";
    FILE* fptr = fopen(filename, "r");

    if (fptr == NULL)
    {
        printf("Error in opening file");
        return NULL;
    }

    arrayNodes * root_arr = createArrayNodes(true);
    int x;
    int y;

    while (fscanf(fptr, "%d %d", &x, &y) != EOF)
    {
        point new_point;
        new_point.x = x;
        new_point.y = y;
        insert(root_arr, createNode(true, new_point, new_point));
    }

    return root_arr;
}

bool overlap(rectangle *x1,rectangle *y1)
{
    if((x1->bottom_left.x <= y1->top_right.x) && (x1->top_right.x >= y1->top_right.x))    //checks for overlap in all the possible cases where 2 rectangles may overlap(right and left along with left and right)
    {
        if((x1->bottom_left.y <= y1->top_right.y) && (x1->top_right.y >= y1->top_right.y))
        {
            return true;
        }

        else if((x1->bottom_left.y <= y1->bottom_left.y) && (x1->top_right.y >= y1->bottom_left.y))
        {
            return true;
        }
    
    }
    
    else if((x1->bottom_left.x <= y1->bottom_left.x) && (x1->top_right.x >= y1->bottom_left.x))
    {
        if((x1->bottom_left.y <= y1->top_right.y) && (x1->top_right.y >= y1->top_right.y))
        {
            return true;
        }

        else if((x1->bottom_left.y <= y1->bottom_left.y) && (x1->top_right.y >= y1->bottom_left.y))
        {
            return true;
        }
    }

    else if((y1->top_right.x >= x1->top_right.x)&&(y1->bottom_left.x <= x1->bottom_left.x)&&(y1->top_right.y >= x1->top_right.y )&&(y1->bottom_left.y <= x1->bottom_left.y))  //the one case that is left out by the above is when one rectangle is inside the other
                                                                                                                                                                              // this case is handled by this code     
   {
    return true;
   }
    
        return false;
    
}


void search(arrayNodes *node,rectangle *searcharea)
{

    if(node->is_leaf == true)       //checks if node is a leaf node or not
    {
        for(int i=0;i<node->count;i++)
        {
            Node *x = node->nodepointer[i];

            if(overlap(x->mbr,searcharea)==true)    //checks if there is any overlap between node and searcharea
            {

                printf("Bottom Left: %d,%d",x->mbr->bottom_left.x,x->mbr->bottom_left.y);      
                printf(" Top Right: %d,%d\n",x->mbr->top_right.x,x->mbr->top_right.y);     //prints out values of all the nodes that intersect/overlap with the searcharea.
                
            }
        }
    }

    else
    {
        for(int i=0;i<node->count;i++)                        //this is the case for internal nodes
        {
            arrayNodes *x = node->nodepointer[i]->childpointer;      

            if(overlap(node->nodepointer[i]->mbr,searcharea) == true)
            {
                search(x,searcharea);             // the code recurses, checking for overlap in all the descendants of an overlapping node.
            }
        }
    } 
}

int main () 
{
    printf("%d\n",h_value(create_rectangle((point){7,11},(point){7,11})));
    arrayNodes * arr = insertData();
    preOrder(arr);
    search(arr,create_rectangle((point){6,5},(point){15,15}));
    printf("123\n");
    return 0;
}

