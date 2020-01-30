# C Style Guide

## Defining Variables
Variables will be defined using lower camel case. All names should be
unique (within reason) and descriptive but not overly verbose and wordy.

Ex.
```c
int someVariable = 4;
```

## Defining Structs
Struct definitions will be defined using upper camel case.

Ex.
```c
typedef struct {
    int latitude;
    int longitude;
} GpsPoint;
```

## Defining Functions
Functions will be defined using lower camel case.

Ex.
```c
void sayHelloTo(char* name) {
    printf("Hello %s", name);
}
```

## Curly Braces
Curly braces should open on the same line as a function or struct followed by a
space. Curly braces should close one line after the body of the struct of
function. See previous two examples.

*Never* use a conditional or loop block without curly braces

```c
// NOT ALLOWED EVER!!!
if (violateThisRule)
    cutMyHandsOff();
```

## Comments
Use single line when you wish to make brief commentary. Use multiline comments
for commenting out sections of code or leaving detailed explanations. Write
code so that comments should be unnecessary for you to understand it, but
comment it so that someone reading it for the first time can understand it as
well.

In general, write comments in full sentences with correct grammar, unless very
short clarification is all that is required.

Commenting out a section of code is acceptable in the short term for
exploration, but any code that is still commented out during a pull request
to master should be commented with the reason it has not been deleted.

Ex.
```c
// Feelings a person can have
bool personHungry, personTired, personBored;

/*
Code to set feeling states... ... ...
*/

// The person is only in a bad mood if they are bored or if they are either
// hungry or tired
bool personInGoodMood = (!personHungry || !personTired) && !personBored;

/* Commented out because we might need to do this in the future
int suspectVar;
suspectFunction(suspectVar);
*/
```

## Spacing
Use four spaces instead of tabs. Most text editors have this configuration
option. If a single statement needs to be broken into multiple lines, indent
the second line either four spaces or start the line at the equivalent "stepping
off point" from the previous line.

Ex.
```c
int myFirstFunctionReturnValue = veryLongFunctionName(firstArgumentThatHasALongName,
                                                      secondArgumentThatHasALongName);

int mySecondFunctionReturnValue = veryLongFunctionName(
    firstArgumentThatHasALongName,
    secondArgumentThatHasALongName);
```

Additional lines within code sections or block entry/termination are
acceptable if they improve readability. Each line should only extend to 80
characters, but must extend less than 120. When a single statement needs to be
broken into multiple lines, do so in ways that are convenient and improve
readability.

