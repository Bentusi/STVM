PROGRAM SystemTest
VAR
    result: INT;
    name: STRING;
END_VAR

(* ===================================== *)
(*  SYSTEM 内置函数测试                  *)
(*  调用操作系统命令并返回退出码         *)
(* ===================================== *)

PRINT('=== SYSTEM Function Test ===\n\n');

(* 测试 1: 简单命令 - echo *)
PRINT('Test 1: echo command\n');
result := SYSTEM('echo Hello from STVM!');
PRINT('Exit code: %d\n\n', result);

(* 测试 2: 列出当前目录 *)
PRINT('Test 2: List current directory\n');
result := SYSTEM('ls -la');
PRINT('Exit code: %d\n\n', result);

(* 测试 3: 显示日期 *)
PRINT('Test 3: Display date\n');
result := SYSTEM('date');
PRINT('Exit code: %d\n\n', result);

(* 测试 4: 创建目录 *)
PRINT('Test 4: Create test directory\n');
result := SYSTEM('mkdir -p /tmp/stvm_test');
IF result = 0 THEN
    PRINT('Directory created successfully\n');
ELSE
    PRINT('Failed to create directory (exit code: %d)\n', result);
END_IF;
PRINT('\n');

(* 测试 5: 写入文件 *)
PRINT('Test 5: Write to file\n');
result := SYSTEM('echo "Hello from STVM" > /tmp/stvm_test/test.txt');
IF result = 0 THEN
    PRINT('File written successfully\n');
ELSE
    PRINT('Failed to write file (exit code: %d)\n', result);
END_IF;
PRINT('\n');

(* 测试 6: 读取文件 *)
PRINT('Test 6: Read file contents\n');
result := SYSTEM('cat /tmp/stvm_test/test.txt');
PRINT('Exit code: %d\n\n', result);

(* 测试 7: 检查文件是否存在 *)
PRINT('Test 7: Check if file exists\n');
result := SYSTEM('test -f /tmp/stvm_test/test.txt && echo "File exists" || echo "File not found"');
PRINT('Exit code: %d\n\n', result);

(* 测试 8: 获取当前用户名 *)
PRINT('Test 8: Get current user\n');
result := SYSTEM('whoami');
PRINT('Exit code: %d\n\n', result);

(* 测试 9: 显示系统信息 *)
PRINT('Test 9: System information\n');
result := SYSTEM('uname -a');
PRINT('Exit code: %d\n\n', result);

(* 测试 10: 清理 - 删除测试目录 *)
PRINT('Test 10: Cleanup - remove test directory\n');
result := SYSTEM('rm -rf /tmp/stvm_test');
IF result = 0 THEN
    PRINT('Cleanup successful\n');
ELSE
    PRINT('Cleanup failed (exit code: %d)\n', result);
END_IF;
PRINT('\n');

(* 测试 11: 失败的命令（不存在的命令）*)
PRINT('Test 11: Non-existent command (should fail)\n');
result := SYSTEM('this_command_does_not_exist_12345');
PRINT('Exit code: %d (non-zero indicates error)\n\n', result);

PRINT('=== All Tests Complete ===\n');

END_PROGRAM
