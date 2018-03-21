# SCCS
### Source Code Change Sets

Small console application with diff-like functionality. Main points are:

- Developing generic template-based LCS (longest common subsequence) algorithm.

- Demonstrating core exception-handling C++ application design.

The task is originating from Tokyo programming contests. In fact, this demo project was developed lately in 2007. Now it with some face-lifting and refactoring presented here. Most critical changes happen in exception-handling part. Now it conforms/requires latest C++11/14 standard.

## Description

Create a console program that can perform the following operations:

- generate a change set file CAB for any given pair of text files (TA, TB)

- apply a given change set file CAB to an input file TX and produce the target file TY.

NOTE: when applied to the input file TA the change set file CAB must produce the original file TB

## Detailed specification

- Files TA and TB are plain ANSI charset text files (ordered collection of lines delimited by ‘\n’ character or ‘\r\n’ character combination).

- The change set file CAB should contain human readable instructions to convert file TA into file TB. It is part of your work to design your own specification of the conversion instructions and change set file format. 

- The conversion instructions should work with whole lines (do not refer to words or symbols inside a line). Lines that differ at least in one character should be considered different (treat differences in whitespace as significant).

- The conversion instructions must be bound to the surrounding lines context (SLC). It is not allowed to bind conversion instructions to absolute or relative line numbers.

- Each SLC in CAB must identify the lines being added, deleted, or modified uniquely in the scope of change set CAB if it is applied to the source file TA. In other words, applying the change set instructions from CAB to the original file TA must never be ambiguous.

- The program must allow applying the change set file to any text file if the contents of the text file are compatible with the contexts found in the change set file. Otherwise the error description should be printed out to the console. The two basic types of errors are: “required context not found” and “change set applying is ambiguous”.

## Program name

Executable file name for the program should be “sccs.exe”.

## Program arguments

### Use case 1

```
	sccs.exe input_file_1 input_file_2 changeset_file
```

Analyze input files input_file_1 and input_file_2, generate instructions to convert input_file_1 to input_file_2, and output the conversion instructions into the changeset_file.

### Use case 2

```
	sccs.exe input_file output_file changeset_file /apply
```

Apply the changeset_file to the input_file and output the results to the output_file.

## Example

*Source file 1*

```
#include <iostream>

int main(int, char**)
{
   int i;
   cout << “Hello world!”;
}
```

*Source file 2*

#include <iostream>

```
using namespace std;

void main()
{
   cout << “Hello world!”;
}
```

*Change set file*

```
[begin]

[insert]
>using namespace std;
>
[between]
>#include <iostream>
>
[and]
>int main(int, char**)

[replace]
>int main(int, char**)
[with]
>void main()

[delete]
>   int i;
[between]
{
[and]
>   cout << “Hello world!’;

[end]
```



