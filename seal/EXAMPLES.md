# Seal Shell - Exemplos Práticos

Este documento contém exemplos práticos de uso do shell para demonstrar todos os recursos implementados.

## 1. Comandos Básicos

```bash
$ ./seal
seal> pwd
/home/user/seal

seal> ls -la
total 128
drwxr-xr-x  3 user user  4096 Jan 19 22:00 .
drwxr-xr-x 42 user user  4096 Jan 19 21:00 ..
-rw-r--r--  1 user user  3503 Jan 19 22:00 shell.h
-rw-r--r--  1 user user  3020 Jan 19 22:00 main.c
...

seal> echo "Testing the shell"
Testing the shell

seal> date
Sun Jan 19 22:00:00 -04 2026
```

## 2. Redirecionamento de Saída

### Output Redirection (>)

```bash
seal> echo "First line" > test.txt
seal> cat test.txt
First line

seal> ls -la > files.txt
seal> head -3 files.txt
total 128
drwxr-xr-x  3 user user  4096 Jan 19 22:00 .
drwxr-xr-x 42 user user  4096 Jan 19 21:00 ..
```

### Append Redirection (>>)

```bash
seal> echo "Second line" >> test.txt
seal> echo "Third line" >> test.txt
seal> cat test.txt
First line
Second line
Third line
```

## 3. Redirecionamento de Entrada

```bash
seal> cat > input.txt
Line 1
Line 2
Line 3
^D

seal> wc -l < input.txt
3

seal> sort < input.txt
Line 1
Line 2
Line 3

seal> grep "2" < input.txt
Line 2
```

## 4. Redirecionamento de Stderr

### Stderr to File (2>)

```bash
seal> ls /nonexistent 2> errors.txt
seal> cat errors.txt
ls: cannot access '/nonexistent': No such file or directory

seal> find / -name "seal" 2> /dev/null
/home/user/seal
/usr/local/bin/seal
```

### Stderr to Stdout (2>&1)

```bash
seal> ls /nonexistent 2>&1 | grep "cannot"
ls: cannot access '/nonexistent': No such file or directory

seal> make 2>&1 | tee build.log
gcc -Wall -Wextra -std=gnu11 -O2 -c main.c -o main.o
...
```

## 5. Pipelines

### Simple Pipes

```bash
seal> ls | wc -l
15

seal> cat /etc/passwd | grep "root"
root:x:0:0:root:/root:/bin/bash

seal> ps aux | grep seal
user    12345  0.0  0.1  12345  1234 pts/0    S+   22:00   0:00 ./seal
```

### Complex Pipelines

```bash
seal> cat /etc/passwd | cut -d: -f1 | sort | head -5
bin
daemon
games
mail
man

seal> ls -la | grep "\.c$" | wc -l
9

seal> ps aux | awk '{print $11}' | sort | uniq -c | sort -rn | head -10
  42 /usr/bin/bash
  12 /usr/bin/python3
   8 /usr/bin/node
...

seal> find . -name "*.c" | xargs cat | wc -l
1523
```

## 6. Job Control - Background Execution

### Simple Background Jobs

```bash
seal> sleep 10 &
[1] 12345

seal> sleep 20 &
[2] 12346

seal> sleep 30 &
[3] 12347

seal> jobs
[1]  Running		sleep 10 &
[2]  Running		sleep 20 &
[3]  Running		sleep 30 &
```

### Long Running Background Tasks

```bash
seal> find / -name "*.txt" > all_txt_files.txt 2>&1 &
[1] 12348

seal> jobs
[1]  Running		find / -name "*.txt" > all_txt_files.txt 2>&1 &

seal> # Continue working while job runs...
seal> ls -la
...

seal> jobs
[1]  Done		find / -name "*.txt" > all_txt_files.txt 2>&1 &
```

## 7. Job Control - Foreground/Background

### Bringing Jobs to Foreground

```bash
seal> sleep 60 &
[1] 12349

seal> jobs
[1]  Running		sleep 60 &

seal> fg 1
# (Job agora em foreground - prompt não aparece)
# Pressione Ctrl-Z para parar
^Z

[1]+ Stopped sleep 60

seal> jobs
[1]  Stopped		sleep 60
```

### Sending Jobs to Background

```bash
seal> sleep 100
# Pressione Ctrl-Z
^Z

[1]+ Stopped sleep 100

seal> jobs
[1]  Stopped		sleep 100

seal> bg 1
[1]+ sleep 100 &

seal> jobs
[1]  Running		sleep 100 &
```

## 8. Job Control - Sinais

### Ctrl-C (SIGINT)

```bash
seal> sleep 1000
# Pressione Ctrl-C
^C

seal> # Shell continua funcionando normalmente
```

### Ctrl-Z (SIGTSTP)

```bash
seal> cat
# Pressione Ctrl-Z
^Z

[1]+ Stopped cat

seal> jobs
[1]  Stopped		cat

seal> fg 1
# (cat retoma execução)
```

## 9. Built-in Commands

### cd - Change Directory

```bash
seal> pwd
/home/user/seal

seal> cd /tmp
seal> pwd
/tmp

seal> cd
seal> pwd
/home/user

seal> cd seal
seal> pwd
/home/user/seal
```

### export - Environment Variables

```bash
seal> export MY_VAR=hello
seal> echo $MY_VAR
# (Nota: expansão de variáveis não implementada ainda)

# Mas a variável está disponível para comandos externos:
seal> env | grep MY_VAR
MY_VAR=hello
```

### help - Show Help

```bash
seal> help
Seal Shell - Custom Shell with Job Control

Built-in commands:
  cd [dir]       Change directory
  exit [status]  Exit shell
  jobs           List active jobs
  fg [job_id]    Bring job to foreground
  bg [job_id]    Send job to background
  help           Show this help
  export VAR=val Set environment variable

Redirection operators:
  <              Redirect input
  >              Redirect output (truncate)
  >>             Redirect output (append)
  2>             Redirect stderr
  2>&1           Redirect stderr to stdout
  |              Pipe

Job control:
  &              Run command in background
  Ctrl-C         Send SIGINT to foreground job
  Ctrl-Z         Send SIGTSTP to foreground job
```

## 10. Combinações Avançadas

### Redirecionamento + Pipe

```bash
seal> cat < input.txt | grep "pattern" > output.txt

seal> ls -la 2>&1 | grep "seal" > seal_files.txt

seal> find / -name "*.log" 2> /dev/null | head -20 > logs.txt
```

### Pipeline + Background

```bash
seal> cat large_file.txt | sort | uniq > sorted_unique.txt &
[1] 12350

seal> jobs
[1]  Running		cat large_file.txt | sort | uniq > sorted_unique.txt &
```

### Complex Pipeline Example

```bash
seal> ps aux | grep -v "grep" | awk '{print $2, $11}' | sort -k2 | head -20
12345 /usr/bin/bash
12346 /usr/bin/python3
12347 /usr/bin/seal
...

seal> cat /etc/passwd | cut -d: -f1,3,6 | sort -t: -k2 -n | column -t -s:
root      0      /root
daemon    1      /usr/sbin
bin       2      /bin
...
```

## 11. Scripting (Modo Não-Interativo)

```bash
# Criar um script
$ cat > test_script.txt << EOF
echo "Starting tests..."
ls -la > files.txt
wc -l < files.txt
cat files.txt | grep "seal"
sleep 1 &
jobs
echo "Tests completed"
EOF

# Executar script
$ cat test_script.txt | ./seal
Starting tests...
15
-rwxr-xr-x  1 user user  12345 Jan 19 22:00 seal
[1] 12351
[1]  Running		sleep 1 &
Tests completed
```

## 12. Casos de Teste para Validação

### Teste 1: Leaks de FD

```bash
seal> ls > out1.txt
seal> cat < out1.txt > out2.txt
seal> wc -l < out2.txt > out3.txt
seal> # Verificar /proc/PID/fd - não deve ter leaks
```

### Teste 2: Process Groups

```bash
seal> sleep 100 | sleep 100 | sleep 100 &
[1] 12352

# Verificar com: ps -o pid,pgid,comm | grep sleep
# Todos os sleeps devem ter o mesmo PGID
```

### Teste 3: Signal Handling

```bash
seal> sleep 1000
# Ctrl-C deve matar apenas o sleep, não o shell
^C

seal> echo "Shell still alive"
Shell still alive
```

### Teste 4: TTY Control

```bash
seal> cat
# Ctrl-Z
^Z

[1]+ Stopped cat

seal> fg 1
# cat deve retomar com terminal funcionando corretamente
```

## 13. Performance Test

```bash
seal> time cat large_file.txt | grep "pattern" | sort | uniq > result.txt

seal> # Pipeline em background para não bloquear
seal> cat huge_file.txt | sort | uniq -c | sort -rn > top_lines.txt &
[1] 12353

seal> # Continue trabalhando enquanto processa...
```

## Conclusão

Estes exemplos demonstram todos os recursos implementados no Seal Shell:

✅ Redirecionamento I/O completo  
✅ Pipelines simples e complexos  
✅ Job control robusto  
✅ Signal handling correto  
✅ Built-in commands funcionais  
✅ Conformidade com padrões Unix  

Para mais informações, consulte o README.md ou execute `seal> help`.
