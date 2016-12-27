# LLVM Practice

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
