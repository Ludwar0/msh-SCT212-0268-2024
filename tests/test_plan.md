# Mini-shell test plan

Record the actual result on the machine used for final verification. Do not mark a test passed until it has been run.

| Input / action | Expected behaviour | Actual result |
|---|---|---|
| Blank line | Fresh prompt, no error | Not yet run |
| `echo hello world` | Prints text; next prompt shows status 0 | Not yet run |
| `ls /nonexistent` | `ls` reports error; prompt shows non-zero status | Not yet run |
| `nosuchcommand` | Shell reports command not found; status 127 | Not yet run |
| `cd /tmp`, then `pwd` | Prints `/tmp` | Not yet run |
| `sleep 3 &` | Prompt returns immediately and PID is printed | Not yet run |
| `echo hi > f.txt`, then `cat f.txt` | File contains `hi` | Not yet run |
| `sleep 100`, then Ctrl-C | Sleep stops; shell survives; status 130 | Not yet run |
| Ctrl-D | Shell exits cleanly | Not yet run |
| `cd` | Changes to `$HOME` | Not yet run |
| `help` | Displays built-in help | Not yet run |
| `printf test > out.txt` | Creates/truncates file and writes output | Not yet run |

The repository includes `tests/test_msh.sh`; run it with `make test` for
repeatable non-interactive smoke checks. Ctrl-C and background-job behavior
should additionally be checked from a real terminal session.
