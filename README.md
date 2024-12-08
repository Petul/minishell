# minishell
Minishell is a POSIX shell prototype and minimal Bash clone. It implements basic shell features such as
* Command parsing and execution
* Builtin commands (`cd`, `echo`, `env`, `exit`, `export`, `pwd` and `unset` )
* Pipes
* Redirections (`<`, `>`, `>>`) and heredoc (`<<`)
* Expansion of environmental variables
* Logical operators (`&&`, `||`) and parentheses for priorities
* Handling of signals SIGINT, SIGQUIT and EOF
* Follows same exit code convetions and has similar error messages as Bash

## Requirements
Depending on your system, you may need to install the following dependencies

- `libreadline`

Using APT:

```
sudo apt install libreadline-dev
```

## Build
```
make
```

## Run
```
./minishell
```
