


# OUR STRUCTS 

## MEMENTRY 
+ segmented paging sort of
+ 32 bits 


```
struct memEntry{
    unsigned int    valid: 1;
    unsigned int    isfree:1;
    unsigned int    request_size:23;

    unsigned int    right_dependent:1; 

    unsigned int    UNUSED;6;
}

```

+ bit fields
    * size (23bits) (maxes out at 8mb)
    * valid (1bit)
        - helps with freeing (cleaning up fragments) 






## PAGE TABLE ENTRY
+ each thread has their own page table 

+ one page dedicated to ucontexts 
+ is a page resident in memory right now?
    + isloaded 
+ something about left dependent 



```
struct page_table_entry{

    unsigned int    UNUSED:4;
    unsigned int    used:1;                  // Is the page currently used 
    unsigned int    resident:1;              // Is the page resident in memory 
    unsigned int    left_dependent:1;       // Do we need to load the next page
    unsigned int    right_dependent:1; 
    unsigned int    largest_available:12;   // Size of largest fragment availble inside the page
    unsigned int    page_number:12;         // Offset of page in memory (if it is loaded)

}
```
 
**Still have 5 bits available!  Defaulting mementry size to 32bits**


## Super Table Array 
+ One pointer per page_size chunk of page table entries 
    * Size of array = 32 pointer entries  


```
(struct page_table_entry*) super_table_array[32];
```





## MEMORY 
+ 8MB 
+ a character array 
+ inside memory-manger's stack 

```
char memory[8 million];          // char is 1 bytes.  Need 8MB
```



## SWAP FILE 
+ 16MB 
+ file pointer





__________________________________________

+ begining of memory-manager.c 
    * lseek(16MB)



## What is going on here 

```
#define SEG(type, base, lim, dpl) (struct segdesc)    \
{ ((lim) >> 12) & 0xffff, (uint)(base) & 0xffff,      \
  ((uint)(base) >> 16) & 0xff, type, 1, dpl, 1,       \
  (uint)(lim) >> 28, 0, 0, 1, 1, (uint)(base) >> 24 }
#define SEG16(type, base, lim, dpl) (struct segdesc)  \
{ (lim) & 0xffff, (uint)(base) & 0xffff,              \
  ((uint)(base) >> 16) & 0xff, type, 1, dpl, 1,       \
(uint)(lim) >> 16, 0, 0, 1, 0, (uint)(base) >> 24 }

```