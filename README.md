# LLVM Practice

LLVM version: 3.5 in MacOS/X

## Command for compile

```
g++ -g ./src/dcc.cpp -I./inc `llvm-config --cxxflags --ldflags --libs` -c -o ./obj/dcc.o
g++ -g ./src/AST.cpp -I./inc `llvm-config --cxxflags --ldflags --libs` -c -o ./obj/AST.o
g++ -g ./src/codegen.cpp -I./inc `llvm-config --cxxflags --ldflags --libs` -c -o ./obj/codegen.o
g++ -g ./src/lexer.cpp -I./inc `llvm-config --cxxflags --ldflags --libs` -c -o ./obj/lexer.o
g++ -g ./src/parser.cpp -I./inc `llvm-config --cxxflags --ldflags --libs` -c -o ./obj/parser.o

```

## Mac OS/X

Check clang verion installed by Xcode.

`clang -v`

Install LLVM of the clang version with homebrew.

`brew install llvm35`

Add PATH

```
vi ~/.bashrc
source ~/.bashrc
```

After installation, rename binary files to make it easier.

`for i in *-3.5; do rename -s $i ${i%-*} * ; done}`
