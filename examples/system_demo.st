PROGRAM SimpleSystemDemo
VAR
    exit_code: INT;
END_VAR

PRINT('=== Simple SYSTEM Function Demo ===\n\n');

(* 1. Display message *)
PRINT('1. Echo a message:\n');
exit_code := SYSTEM('echo "STVM can call system commands!"');
PRINT('   Exit code: %d\n\n', exit_code);

(* 2. Show current directory *)
PRINT('2. Current directory:\n');
exit_code := SYSTEM('pwd');
PRINT('   Exit code: %d\n\n', exit_code);

(* 3. List files *)
PRINT('3. List example files:\n');
exit_code := SYSTEM('ls examples/*.st');
PRINT('   Exit code: %d\n\n', exit_code);

(* 4. Create and use a temp file *)
PRINT('4. Working with files:\n');
exit_code := SYSTEM('echo "Data from STVM" > /tmp/stvm_demo.txt && cat /tmp/stvm_demo.txt && rm /tmp/stvm_demo.txt');
PRINT('   Exit code: %d\n\n', exit_code);

(* 5. Check status *)
IF exit_code = 0 THEN
    PRINT('All operations completed successfully!\n');
ELSE
    PRINT('Some operations failed with exit code: %d\n', exit_code);
END_IF;

END_PROGRAM
