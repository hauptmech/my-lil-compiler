; Declare the string constant as a global constant. 
@.LC1 = internal constant [20 x i8] c"hello bigger world\0A\00"      ; [13 x i8]* 

; External declaration of the puts function 
declare i32 @puts(i8*)
; Definition of main function
define i32 @other() {   ; i32()*  
  ; Convert [13 x i8]* to i8  *... 
  %cast210 = getelementptr [20 x i8]* @.LC1, i64 0, i64 0   ; i8* 

  ; Call puts function to write out the string to stdout. 
  call i32 @puts(i8* %cast210)           ; i32 
  ret i32 0 
}

; Named metadata
;!1 = metadata !{i32 41}
;!foo =  !{i32 41,null}
