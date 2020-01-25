## Defining Variables
variables will be defined using snake case.

Ex.
```c
int some_variable = 4;
```

## Defining Structs
Struct definitions will be defined using upper camel case.

Ex.
```c
typedef struct {
    int latitude;
    int longitude;
} gps_point;
```

## Defining Functions
Functions will be defined using snake case

Ex.
```c
void say_hello_to(char* name) {
    printf("Hello %s", name);
}
```

## Curly Braces
Curly braces should open on the same line as a function or struct followed by a space. Curly braces should close one line after the body of the struct of function. See previous two examples.

## Comments
Use single line when you wish to make commentary. Use multiline when you are trying to debug.
In general, try to code so your code reads like a book and does not require many comments.

Ex.
```c
bool person_hungry;
bool person_tired;
bool person_bored;

// Can't seem to come up with any good commentary for this example
bool person_in_good_mood = (!person_hungry || !person_tired) && !person_bored;

/*
int suspect_var;
suspect_function(suspect_var);
*/
```


