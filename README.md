**Student:** Emmanuel Ludwaro  
**Registration number:** SCT212-0268/2024  
**Unit:** ICS 2305 – Systems Programming

## Build and run

```sh
make
./msh
```

Use `make clean` to remove the executable. Type `exit` or press Ctrl-D to leave.

## Feature checklist

| Task | Status | Implementation |
|---|---|---|
| 1. Prompt, arbitrary-length input, EOF cleanup | Complete | `getline()` and `free()` |
| 2. Whitespace tokenisation and NULL-terminated argv | Complete | `strtok_r()` |
| 3. `fork()`, `execvp()`, `waitpid()` and errors | Complete | Child/parent execution path |
| 4. `cd`, `help`, `exit` | Complete | Parent-process built-ins |
| 5. Previous status in prompt | Complete | Exit and signal status handling |
| 6. Background `&` and reaping | Complete | `WNOHANG` reap pass at prompts |
| 7. Output redirection `>` | Complete | Child uses `open()`, `dup2()`, `close()` |
| 8. Ctrl-C behavior | Complete | Shell handler forwards SIGINT; child restores default |

## Design explanation

The program is organised around a simple read–parse–execute loop. At the start of each iteration it checks for completed background children without blocking, then displays the most recent command status in the prompt. `getline()` reads the complete input line, so the shell does not depend on a fixed line buffer. `strtok_r()` separates spaces, tabs and line endings into an argument vector, and the final pointer is set to `NULL` as required by `execvp()`.

Commands that change the shell itself are handled before creating a child. `cd` calls `chdir()` in the shell process, so its effect remains in later commands; running it in a child would change only that child's working directory. `exit` is also built in because terminating a child would not terminate the interactive shell. `help` prints the available syntax.

For an external command, the shell calls `fork()`. The child optionally opens a redirection target and connects it to standard output with `dup2()`, then calls `execvp()` to replace its process image. If execution fails, it reports the error and exits with a conventional status. The parent either waits for a foreground process and records its exit/signal status, or prints the PID of a background process and immediately returns to the prompt. `waitpid(..., WNOHANG)` periodically collects completed background children, preventing zombies. The shell handles Ctrl-C without exiting and forwards the signal to its active foreground child.

## Tests

See [`tests/test_plan.md`](tests/test_plan.md). Run the listed interactive tests manually; the plan does not claim results that have not been observed on your machine.

## Known limitations

This is a teaching shell rather than a POSIX shell. It has no quoting, variable expansion, pipelines, append redirection, multiple redirections, job-control process groups, or command history. The argument vector is capped at 255 entries. Background children are reaped when the shell returns to its prompt, rather than by a dedicated SIGCHLD handler.

## Sources and AI-tool declaration

The implementation follows the assignment brief and standard Linux/POSIX interfaces (`getline`, `fork`, `execvp`, `waitpid`, `open`, `dup2`, `sigaction`). ChatGPT was used to help draft and review the code and documentation. The student should read, understand, compile, test, and be prepared to explain every part before submission.

## Local smoke tests

Run `make test` to compile the shell and exercise the built-ins, external command
execution, working-directory changes, output redirection, and an invalid command.
The tests are intentionally small smoke tests; also try interactive Ctrl-C and
background jobs in a terminal before submission.

## Local smoke tests

Run `make test` to compile the shell and exercise the built-ins, external command
execution, working-directory changes, output redirection, and an invalid command.
Also try interactive Ctrl-C and background jobs in a terminal before submission.
