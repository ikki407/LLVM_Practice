; ModuleID = 'loop.c'
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.9.0"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

; Function Attrs: nounwind ssp uwtable
define i32 @loop(i32 %n) #0 {
  %1 = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 %n, i32* %1, align 4
  store i32 0, i32* %i, align 4
  br label %2

; <label>:2                                       ; preds = %9, %0
  %3 = load i32* %i, align 4
  %4 = load i32* %1, align 4
  %5 = icmp slt i32 %3, %4
  br i1 %5, label %6, label %12

; <label>:6                                       ; preds = %2
  %7 = load i32* %i, align 4
  %8 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([4 x i8]* @.str, i32 0, i32 0), i32 %7)
  br label %9

; <label>:9                                       ; preds = %6
  %10 = load i32* %i, align 4
  %11 = add nsw i32 %10, 1
  store i32 %11, i32* %i, align 4
  br label %2

; <label>:12                                      ; preds = %2
  %13 = load i32* %i, align 4
  ret i32 %13
}

declare i32 @printf(i8*, ...) #1

attributes #0 = { nounwind ssp uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = metadata !{metadata !"Apple LLVM version 6.0 (clang-600.0.56) (based on LLVM 3.5svn)"}
