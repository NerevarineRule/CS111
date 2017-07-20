// UCLA CS 111 Lab 1 command internals

enum command_type
  {
    AND_COMMAND,         // A && B
    SEQUENCE_COMMAND,    // A ; B
    OR_COMMAND,          // A || B
    PIPE_COMMAND,        // A | B
    SIMPLE_COMMAND,      // a simple command
    SUBSHELL_COMMAND,    // ( A )
  };

// Data associated with a command.
struct command
{
  enum command_type type;

  // Exit status, or -1 if not known (e.g., because it has not exited yet).
  int status;

  // I/O redirections, or null if none.
  char *input;
  char *output;
  
  // left and right status for PIPE execution
  int* write;
  int* read;

  union
  {
    // for AND_COMMAND, SEQUENCE_COMMAND, OR_COMMAND, PIPE_COMMAND:
    struct command *command[2];

    // for SIMPLE_COMMAND:
    char **word;

    // for SUBSHELL_COMMAND:
    struct command *subshell_command;
  } u;

  struct command** before; //commands that current command depends on
  struct command** after; //commands that depend on current command
	
  int bsize; //size of before array
  int asize; //size of after array
 
  pid_t pid; //process id of current command  
};

struct wStream
{
	char** wordStream;
	char** inputSet;
	char** outputSet;
	int nword;
	int nin;
	int nout;		

};

struct cstack
{
  struct cstack* next;
  struct cstack* prev;
  struct command* cmd;
  int count;	
  struct wStream* warray;
};

struct queue
{
 	struct command* ptr;
	struct queue* next;
};


