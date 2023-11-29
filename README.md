# cLoxLox
cLoxLox is a modern c++ implementation heavily reliant upon *jlox* from Part II of [Crafting Interpreters](https://www.craftinginterpreters.com/) by Robert Nystrom. This project was developed as an assignment for CS 403 at the University of Alabama during Fall of 2023. Throughout the development of cLL, I attempted to stay as true to the original implementation as I could. Since the orginal language of *jlox* was Java, it was a bit challenge for a novice programmer to translate this into c++. For more information of my challenges, look under [Challenges](#challenges) or skip it if you don't want to hear me complain.

cLL has been tested on Windows 10 and 11 with Microsoft's VS Code. If you would like to test it on your own system, please follow the instructions found under the [Building](#building) header. If you have any issues, please email me at: hbrousseau@crimson.ua.edu

## Files and Reading the Code
I have submitted a zip file for the Lox interpreter partitioned for each chapter that I have worked through (up until chapter 13, which is the final piece of the Lox puzzle). I attempted to comment throughout all of the chapters what the code was attempting to do and when/where I was finding issues. Up until chapter 13, I ran tests through a REPL to verify the output of each section.

## Building
Clone the repository to a location most convient to you.
```
git clone https://github.com/hbrousseau/cLoxLox.git
```

Navigate inside ch13Checkpoint to build the entire project.
`cd ch13Checkpoint`

To compile cLL.cpp the program, run `make cLL`

To compile GenerateAST.cpp, run `make GenerateAST`

I found test cases and a Makefile for a c++ implementation of Lox [here](https://github.com/the-lambda-way/CppLox/tree/c9d65280108cfd2ce7e42a9d2a7fc09c95c21296/chapter13)

Tests cases can be run individually by `make test-name` or run all at once by `make test-all`

## Usage
cLL has two usages: either as a REPL (read, print, eval loop) or, given a source file, cLL will attempt to execute the code and exit the program. To run the program as a REPL:
```
./cLL.exe
```

Otherwise, to excute a Lox source file:
```
./cLL.exe [Lox script]
```

## Testing
Testing, as described in [Building](#building), can be run all at once or individually. The test cases run through the cruxes of each chapter, mainly using code provided by the textbook. I will say that I am not very familiar with testing suites; however, all of the output statements were printed as:
```
testing cLL with test-statements.lox ...
--- tests/test-statements.lox.expected  2023-11-18 07:15:44.270884600 -0600
+++ -   2023-11-29 14:08:51.869431200 -0600
@@ -1,3 +1,3 @@
-one
-true
-3.000000
+one
+true
+3.000000
make[1]: *** [Makefile:89: test-statements] Error 1
```
The individual who created the testing suite had the same output running his/her tests as I did. With that being said, I understand that it says that there are differences between the expected and my output. However, if I were to run the REPL with the same test case, my output looks as expected. Do with this information as you will. 

## Challenges 
Translating Java to c++ provided many interesting challenges for me. Beginning in chapter 8, I had run into some frustrating bugs and is reflected in comments within my code. From then on, I decided it was best for me to make notes where code had either been updated or added to pre-existing code for easier debugging. In chapter 12, I had to completely restructure my Interpreter.hpp file to follow the structure the book had laid out for me. I didn't notice that the author gave us where exactly to order our code. I was already in too deep when I noticed this and thought it wouldn't matter. But, knowing c++, I should have known better. Numerous days of debugging led to this discovery. Also, I prefer using <unordered_map>s in c++; however, the compiler on my laptop was none-too-thrilled about it. So, ultimately, I changed to using <map> instead. 


