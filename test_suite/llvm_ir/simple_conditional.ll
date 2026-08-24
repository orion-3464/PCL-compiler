; int G;
; int H;
;
; int test(bool cond) {
;     int x;
;     if (cond) {
;         x = G;
;     } else {
;         x = H;
;     }
;     return X;
; }

@G = global i32 0
@H = global i32 0

define i32 @main(i1 %Condition) {
    entry:
        br i1 %Condition, label %cond_true, label %cond_false

    cond_true:
        %X.0 = load i32, ptr @G
  	br label %cond_next
	
    cond_false:
	%X.1 = load i32, ptr @H
  	br label %cond_next

    cond_next:
	%X.2 = phi i32 [ %X.1, %cond_false ], [ %X.0, %cond_true ]
	ret i32 %X.2
}

