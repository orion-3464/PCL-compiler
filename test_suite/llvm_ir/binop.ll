define i32 @main() {
entry:
    ; Float Casting and Addition
    %0 = sitofp i32 1 to float 
    %1 = sitofp i32 2 to float
    %2 = fadd float %0, %1
    %3 = fsub float %0, %1
    %4 = fdiv float %0, %1
    
    ret i32 0
}
