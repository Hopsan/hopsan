# libNumHop

A simple string parsing numerical calculation library written in C++.
It interprets strings with mathematical expressions and allows creation and reading of internal or external variables.

Internal variables are local within the library, external variables are such variables that may be present in some other code using this library.

The library supports the following operators: = + - * / ^ and expressions within ()
Scripts can have LF, CRLF or CR line endings


## Usage examples

```
# The Hash character is used to comment a line
a = 1; b = 2; c = 3    # Multiple expressions can be written on the same line separated by ;
d = a+b*c              # d Should now have the value 7
d                      # evaluate d (show value of d)
d = (a+b)*c            # d Should now have the value 9
d = d / 3              # d will now have the value 3
```


For more examples see the included "test" executable

## Build instructions
The library uses qmake and QtCreator as the build system, but the library is plain C++ and Qt is technically not required.
The libNumHop files can be directly included in an external project or be build as a static library.

## Implementation details
The library builds a tree from the expressions, each detected operator will branch the tree and finally the leaves will contain numerical values or variable names.
The operators are processed (tree is branched) in the following order, =, +-, */, ^ 

The internal variable storage can be extended with access to external variables by overloading members in a pure virtual class made for this purpose.
This way you can access your own variables in your own code to set and get variable values.

See the doxygen documentation for further details.

