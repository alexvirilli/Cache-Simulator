#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>

//int fifoSize = 0;

struct Node{
    int valid; //1 for valid 0 for invalid
    long int data;
    struct Node* next;
};

void freeAll(struct Node* head) {
    if(head==NULL){
        exit(EXIT_SUCCESS);
    }
    struct Node* ptr;
    while(head!=NULL){
        ptr = head->next;
        free(head);
        head = ptr;
    }
}

//searches for first node containing 'data'
//and deletes that node from the LL
struct Node* delete(struct Node* head, int num){
    struct Node* prev = head;
    struct Node* curr = head;
    if(head==NULL){
        return head;
    }
    if(head->data == num) {
        prev = curr->next;
        free(head);
        //fifoSize--;
        return prev;
    }
    while(curr->next!=NULL && curr->data!=num){
        prev = curr;
        curr = curr->next;
        if(curr->data == num) {
            prev->next = curr->next;
            free(curr);
            //fifoSize--;
            return head;
        }
    }
    //fifoSize--;
    return head;
}

struct Node* fifoInsert(struct Node* head, int num, int maxSize){ //pass in more parameters
    
    int count = 0;
    if(head!=NULL){
        struct Node* cnt = head;
        while(cnt!=NULL){
            count++;
            cnt = cnt->next;
        }
    }
    
    if(count >= maxSize) {
        struct Node* tmp = head;
        head = head->next;
        free(tmp);
        //fifoSize--;
    }
        if(head==NULL) {  //case: no nodes
            head = malloc(sizeof(struct Node));
            head->data = num;
            //add other elements of node here
            head->next = NULL;
            //fifoSize++;
            return head;
        }
        struct Node* new = malloc(sizeof(struct Node));
        struct Node* prev = head;
        struct Node* curr = head;
        new->data = num;
        //add new elements of node here
        new->next = NULL;
        while(curr!=NULL) {
            prev = curr;
            curr = curr->next;
        }
        prev->next = new;
        new->next = curr;
        //fifoSize++;
        return head;
}

//move the element just accessed to the top
struct Node* updateLRU(struct Node* head, int num, int mSize) {
    struct Node* ptr = head;
    while(ptr->data != num && ptr->next!=NULL) {
        ptr = ptr->next;
    }
    //assuming the element was found
    head = delete(head, ptr->data);
    return fifoInsert(head, num, mSize);
}

void printlist(struct Node* head)
{
    if(head==NULL) {
        printf("EMPTY\n");
    } else {
        while(head!=NULL) {
            printf("%ld ",head->data);
            head = head->next;
        }
        printf("\n");
    }
}

//return 1 for a cache hit, 0 for a cache miss
int cacheHit(struct Node* head, int tag) {
    struct Node* temp = head;
    if(temp==NULL) return 0;
    while(temp->data != tag && temp->next != NULL){
        temp = temp->next;
    }
    if(temp->data == tag){
        return 1;
    } else {
        return 0;
    }
}

int main(int argc, char* argv[argc+1]){

    int hits = 0;
    int misses = 0;
    int read = 0;
    int write = 0;

    int cacheSize = atoi(argv[1]);
    int assoc = atoi(&(argv[2][6]));
    int blockSize = atoi(argv[4]); 
    int sets = cacheSize / (blockSize * assoc);
    int maxSize = assoc;

    //make an array of sets
    struct Node* cache[sets];
    for(int i = 0; i < sets; i++){
        struct Node* head = NULL;
        cache[i] = head;
    }

    int indexBits = (int)log2(sets);
    int offsetBits = log2(blockSize);
    int tagBits = 48 - offsetBits - indexBits;
    char WL[256];
    long unsigned int addr = 0;

    FILE* fPointer = fopen(argv[5], "r");
    while(fscanf(fPointer, "%s %lx", WL, &addr)!=EOF){
        
        int index = (addr >> offsetBits) & (sets - 1);
        int tag = ((addr >> (offsetBits + indexBits)) & ((int)pow(2,tagBits)-1));
        //printf("index: %d\ntag: %d\n",index,tag);
        
        if(cacheHit(cache[index],tag)==1){ 
            hits++; 
        } else {
            misses++;
        }
            
        if(strcmp(WL,"W")==0){
            //write
            if(cacheHit(cache[index],tag)==1){
                write++;
                if(strcmp(argv[3],"lru")==0) cache[index] = updateLRU(cache[index],tag,maxSize);
            } else {
                write++;
                read++;
                cache[index] = fifoInsert(cache[index], tag, maxSize);
            }
        } else {
            //read
            if(cacheHit(cache[index],tag)==0){
                read++;
                cache[index] = fifoInsert(cache[index], tag, maxSize);
            } else {
                if(strcmp(argv[3],"lru")==0) cache[index] = updateLRU(cache[index],tag,maxSize);
            }
        }
        //printlist(cache[index]);
    }

    // printf("\nlists:\n");
    // for(int i = 0; i < sets; i++){
    //     printlist(cache[i]);
    // }
    // printf("\n");

    printf("memread:%d\n",read);
    printf("memwrite:%d\n",write);
    printf("cachehit:%d\n",hits);
    printf("cachemiss:%d\n",misses);

    for(int i = 0; i < sets; i++){
        freeAll(cache[i]);
    }
}
