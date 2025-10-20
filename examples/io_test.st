PROGRAM IOTest
VAR_EXTERNAL
    button : BOOL AT "%IX0.0";
    led : BOOL AT "%QX0.0";
END_VAR

BEGIN
    // 从按钮读取，控制 LED
    led := button;
END_PROGRAM
