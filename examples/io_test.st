PROGRAM IOTest
VAR_EXTERNAL
    button  AT "%IX0.0" : BOOL;
    led  AT "%QX0.0" : BOOL;
END_VAR

BEGIN
    // 从按钮读取，控制 LED
    led := button;
END_PROGRAM
