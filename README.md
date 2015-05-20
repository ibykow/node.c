# node.c

1. clone
2. cd
3. $ make test

### Example Usage

Create a new string node:
```c
struct node_s *str_node = str_node_new("Hello");
```

Print out the node string:
```c
printf("%s\n", str_node_buf(str_node));
```

Add a child node:
```c
node_push(str_node, str_node_new("World"));
```

Print out the child node string:
```c
printf("%s\n", str_node_buf(str_node->table[0]));
```

Release a child node:
```c
struct node_s *child = node_release(str_node, 0);
```

Free a node and all its children (and their children's children, and so on):
```c
free_node(str_node);
```

The expansion of the ```str_node_new``` and ```str_init``` macros illustrates how ```node_new``` is used.
```c
/* This call */
struct node_s *str_node = str_node_new("Hello");

/* becomes this */
struct node_s *str_node =
    node_new(node_type_str,
        &(struct str_s) {.buf = "Hello", .len = strlen("Hello")});
```

### API

##### node
**Create**:
```c
struct node_s *node_new(const struct node_type_s *type, const void *d);
```
See the expansion of str_node_new and str_init above for a description on how this functions is used.

**Destroy**:
```c
void node_free(struct node_s *n, bool recurse);
```  
Recursively frees all data associated with the node and its children (and children's children, etc).

##### node table
**Insert**:
```c
size_t node_put(struct node_s *n, size_t index, struct node_s *new);
```
Inserts a child node into a parent node at a given index.

**Warning**! This function replaces whatever was in the table at the index previously. It's your responsibility to check and keep track of the node that might be present at that index.

*inputs*  
 ```struct node_s *n``` - a pointer to the parent node  
 ```size_t index``` - the index at which you want to insert the child node  
 ```struct node_s *c``` - the index at which you want to insert the child node

*output*  
```size_t``` - the new length of the parent node table, or 0 if an insertion did not take place.

*notes*
- This function grows the parent node's table as necessary.
- After insertion you can look up the child node's current index in the parent's array with ```c->id```, where ```c``` is the pointer to the child node.

**Release**:
```c
struct node_s *node_release(struct node_s *n, size_t index);
```
*inputs*  
```struct node_s *n``` - a pointer to the parent node of the node you want released  
```size_t index``` - the index of the child node within the parent node's table.

*ouput*  
```struct node_s *``` - a pointer to the child node if one exists, or 0 otherwise

*notes*
 - This function shrinks the parent node's table as necessary.
 - If you have a node pointer c belonging to another node n you can look up
    c's index in n's table with ```c->id```
