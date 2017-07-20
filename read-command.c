// UCLA CS 111 Lab 1 command reading

#include "alloc.h"
#include "command.h"
#include "command-internals.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <string.h>
#include <error.h>


struct command_stream
{ 
 struct cstack* commands;
 struct queue* independent;
 struct queue* dependent;
};


int norm_char(char elem)
{
  if (isalpha(elem) || isdigit(elem) || elem == '!' || elem == '%' || elem == '+' ||
	   elem == ',' || elem == '-' || elem == '.' || elem == '/' ||elem == ':' || 
	   elem == '@' || elem == '^' || elem == '_' || elem == '{' || elem == '}')
  {
	return 1;
  }
  return 0;
}

int special_char(char elem, int ignore_open_paren)
{
  if (ignore_open_paren == 0 && (elem == ';' || elem == '|' || elem == '&' || elem == '(' ||
      elem == ')' || elem == '<' || elem == '>'))
  {
	return 1;
  }
  else if(elem == ';' || elem == '|' || elem == '&' ||  elem == '<' || elem == '>'||
      elem == ')' )
  {
	return 1;
  }
  return 0;
}

struct command* build_command(char* stream, struct wStream* warray)
{
 struct command* cmd = (struct command*)checked_malloc(sizeof(struct command));
 struct command* subshell_cmd = (struct command*)checked_malloc(sizeof(struct command));
 char* inputw = NULL;
 char* outputw = NULL;
 int hassubshell = 0;
 int sindex = 0;
 int complexcommand = 0;

 //get size of command
 while(stream[sindex] != '\000')
 {
	sindex++;
 }
 

 sindex--; 
 int end = sindex;
 int input = 0;
 int output = 0;
 while(sindex != 0 && (norm_char(stream[sindex]) || stream[sindex] == '<' || stream[sindex] == '>' || stream[sindex] == ' '))
 {
	if(stream[sindex] == '>')
	{
		output = sindex;
	}
	if(stream[sindex] == '<')
	{
		input = sindex;
	}
	sindex--;
	
 }

 //handle I/O
 if(input != 0 && output != 0)
 {
	input++;
	char* token = (char*) checked_malloc(500*sizeof(char));
	int sizeof_token = 500;
	int tindex = 0;
	while(input != output)
	{
		if(tindex >= sizeof_token)
		{
			sizeof_token += 500;
			token = checked_realloc(token, sizeof_token+1);
		}
		token[tindex] = stream[input];
		input++;
		tindex++;
	}
	if(norm_char(stream[input]))
	{
		token[tindex] = stream[input];
	}
	inputw = token;
	token = (char*) checked_malloc(500*sizeof(char));
	tindex = 0;
	output++;
	while(output != end)
	{
		if(tindex >= sizeof_token)
		{
			sizeof_token += 500;
			token = checked_realloc(token, sizeof_token+1);
		}
		token[tindex] = stream[output];
		output++;
		tindex++;
	}
	if(norm_char(stream[output]))
	{
		token[tindex] = stream[output];
	}
	outputw = token;
 }
 else if(input != 0)
 {
	input++;
	char* token = (char*) checked_malloc(500*sizeof(char));
	int sizeof_token = 500;
	int tindex = 0;
	while(input != end)
	{
		if(tindex >= sizeof_token)
		{
			sizeof_token += 500;
			token = checked_realloc(token, sizeof_token+1);
		}
		token[tindex] = stream[input];
		input++;
		tindex++;
	}
	if(norm_char(stream[input]))
	{
		token[tindex] = stream[input];
	}
	inputw = token;

 }
 else if(output != 0)
 {
	output++;
	char* token = (char*) checked_malloc(500*sizeof(char));
	int sizeof_token = 500;
	int tindex = 0;
	while(output != end)
	{
		if(tindex >= sizeof_token)
		{
			sizeof_token += 500;
			token = checked_realloc(token, sizeof_token+1);
		}
		token[tindex] = stream[output];
		output++;
		tindex++;
	}
	if(norm_char(stream[output]))
	{
		token[tindex] = stream[output];
	}
	outputw = token;
 }
 //handle simple command
 if(sindex == 0)
 {
	char** word = (char**)checked_malloc(50*sizeof(char*));
	int sizeof_word = 50;
	int windex = 0;
	char* token = (char*) checked_malloc(500*sizeof(char));
	int sizeof_token = 500;
	int tindex = 0;
	cmd->type = SIMPLE_COMMAND;
	while(sindex <= end && stream[sindex] != '<' && stream[sindex] != '>')
	{
		if(windex >= sizeof_word)
		{
			sizeof_word += 50;
			word = checked_realloc(word, sizeof_word+1);
		}
		if(tindex >= sizeof_token)
		{
			sizeof_token += 500;
			token = checked_realloc(token, sizeof_token+1);
		}
		if(norm_char(stream[sindex]))
		{
			token[tindex] = stream[sindex];
			sindex++;
			tindex++;
		}
		else if(stream[sindex] == ' ')
		{
			word[windex] = token;
			warray->wordStream[warray->nword] = token;
			warray->nword++;
			sindex++;
			windex++;
			token = (char*) checked_malloc(500*sizeof(char));
			tindex = 0;
		}
	}
	if(norm_char(stream[sindex]))
	{
		token[tindex] = stream[sindex];
	}
	word[windex] = token;
	warray->wordStream[warray->nword] = token;
	warray->nword++;
	windex++;
	word[windex] = NULL;
	if(outputw)
	{
		warray->outputSet[warray->nout] = outputw;
		warray->nout++;
	}
	if(inputw)
	{
		warray->inputSet[warray->nin] = inputw;
		warray->nin++;
	}
	cmd->input = inputw;
	cmd->output = outputw;
	cmd->u.word = word;
 }
 //handle subshell
 else if(stream[sindex] == ')')
 {
	sindex--;
	int shellend = sindex;
	int subshells = 1;
	while(sindex != 0 && subshells != 0)
	{
		if(stream[sindex] == ')')
		{
			subshells++;
		}
		if(stream[sindex] == '(')
		{
			subshells--;
		}
		sindex--;
	}
	char* sub = (char*) checked_malloc(end*sizeof(char)+1);
	if(sindex == 0)
	{
		cmd->type = SUBSHELL_COMMAND;
		if(outputw)
		{
			warray->outputSet[warray->nout] = outputw;
			warray->nout++;
		}
		if(inputw)
		{
			warray->inputSet[warray->nin] = inputw;
			warray->nin++;
		}
		cmd->input = inputw;
		cmd->output = outputw;
		stream++;
		strncpy(sub, stream, (size_t)(shellend-sindex));
		cmd->u.subshell_command = build_command(sub, warray);
	}
	else
	{
		hassubshell = 1;
		subshell_cmd->type = SUBSHELL_COMMAND;
		if(outputw)
		{
			warray->outputSet[warray->nout] = outputw;
			warray->nout++;
		}
		if(inputw)
		{
			warray->inputSet[warray->nin] = inputw;
			warray->nin++;
		}
		subshell_cmd->input = inputw;
		subshell_cmd->output = outputw;
		sindex += 2;
		char* temp = stream;
		temp += sindex;
		strncpy(sub, temp, (size_t)(shellend-(sindex-1)));
		subshell_cmd->u.subshell_command = build_command(sub, warray);
		sindex -=3;

	}
 }
 //handle sequence commands
 int checker = sindex;
 int ignore = 0;
 while(checker != 0 && (stream[checker] != ';' || ignore != 0))
 {
	if(stream[checker] != ')')
	{
		ignore++;
	}
	if(stream[checker] != '(')
	{
		ignore--;
	}
	if(stream[checker] != '&' || stream[checker] != '|' || stream[checker] != '?')
	{
		complexcommand = 1;
	}
	checker--;
 }
 if(checker != 0)
 {
	char* temp = stream;
	strcpy(temp, stream);
	char* front = (char*) checked_malloc(strlen(stream));
	char* back = (char*) checked_malloc(strlen(stream));
	strncpy(front, temp, (size_t)(checker-1));
	if(hassubshell == 0 || complexcommand != 0)
	{
		temp = stream;
		strcpy(temp, stream);
		temp += (checker + 2);
		strncpy(back, temp, ((size_t)(end-(checker + 1))));
		cmd->type = SEQUENCE_COMMAND;
		cmd->u.command[0] = build_command(front, warray);
		cmd->u.command[1] = build_command(back, warray);
		return cmd;
	}
	else
	{
		cmd->type = SEQUENCE_COMMAND;
		cmd->u.command[0] = build_command(front, warray);
		cmd->u.command[1] = subshell_cmd;
		return cmd;
	}

 }

 // handle and and or commands
 checker = sindex;
 ignore = 0;
 while(checker != 0 && ((stream[checker] != '&' && stream[checker] != '?') || ignore != 0))
 {
	if(stream[checker] != ')')
	{
		ignore++;
	}
	if(stream[checker] != '(')
	{
		ignore--;
	}
	if(stream[checker] != '|')
	{
		complexcommand = 1;
	}

	checker--;
 }
 if(checker != 0)
 {
	if(stream[checker] == '&')
	{
		cmd->type = AND_COMMAND;
	}
	else
	{
		cmd->type = OR_COMMAND;
	}
	char* temp = stream;
	strcpy(temp, stream);
	char* front = (char*) checked_malloc(strlen(stream));
	char* back = (char*) checked_malloc(strlen(stream));
	strncpy(front, temp, (size_t)(checker-1));
	if(hassubshell == 0 || complexcommand != 0)
	{
		temp = stream;
		strcpy(temp, stream);
		temp += (checker + 2);
		strncpy(back, temp, ((size_t)(end-(checker + 1))));
		cmd->u.command[0] = build_command(front, warray);
		cmd->u.command[1] = build_command(back, warray);
		return cmd;
	}
	else
	{
		cmd->u.command[0] = build_command(front, warray);
		cmd->u.command[1] = subshell_cmd;
		return cmd;
	}
 }

 // handle pipe commands
 checker = sindex;
 ignore = 0;
 while(checker != 0 && (stream[checker] != '|' || ignore != 0))
 {
	if(stream[checker] != ')')
	{
		ignore++;
	}
	if(stream[checker] != '(')
	{
		ignore--;
	}
	checker--;
 }
 if(checker != 0)
 {
	cmd->type = PIPE_COMMAND;
	char* temp = stream;
	strcpy(temp, stream);
	char* front = (char*) checked_malloc(strlen(stream));
	char* back = (char*) checked_malloc(strlen(stream));
	strncpy(front, temp, (size_t)(checker-1));
	if(hassubshell == 0)
	{
		temp = stream;
		strcpy(temp, stream);
		temp += (checker + 2);
		strncpy(back, temp, ((size_t)(end-(checker + 1))));
		cmd->u.command[0] = build_command(front, warray);
		cmd->u.command[1] = build_command(back, warray);
		return cmd;
	}
	else
	{
		cmd->u.command[0] = build_command(front, warray);
		cmd->u.command[1] = subshell_cmd;
		return cmd;
	}
 }




 return cmd;
}

struct cstack* create_command(struct cstack* cmds, char* stream)
{
 cmds->warray = (struct wStream*)checked_malloc(sizeof(struct wStream));
 cmds->warray->wordStream = (char**)checked_malloc(50*sizeof(char*));
 cmds->warray->inputSet = (char**)checked_malloc(50*sizeof(char*));
 cmds->warray->outputSet = (char**)checked_malloc(50*sizeof(char*));
 cmds->warray->nword = 0;
 cmds->warray->nin = 0;
 cmds->warray->nout = 0;
 struct command* subshell_cmd = (struct command*)checked_malloc(sizeof(struct command));
 int hassubshell = 0;
 char* inputw = NULL;
 char* outputw = NULL;
 stream += cmds->count;
 int sindex = 0;
 int complexcommand = 0;

 //return null if no command
 if(stream[sindex] == EOF)
 {
	cmds->cmd = NULL;
	return cmds;
 }

 //get size of command
 while(stream[sindex] != '\n' && stream[sindex] != EOF)
 {
	sindex++;
 }
 
 //mark beginning of next command or EOF
 if(stream[sindex] == '\n')
 {
	sindex++;
	cmds->count = sindex+cmds->count;
	sindex -= 2;
 }
 else
 {
	cmds->count = sindex+cmds->count;
	sindex--;

 }
 
 int end = sindex;
 int input = 0;
 int output = 0;
 while(sindex != 0 && (norm_char(stream[sindex]) || stream[sindex] == '<' || stream[sindex] == '>' || stream[sindex] == ' '))
 {
	if(stream[sindex] == '>')
	{
		output = sindex;
	}
	if(stream[sindex] == '<')
	{
		input = sindex;
	}
	sindex--;
	
 }

 //handle I/O
 if(input != 0 && output != 0)
 {
	input++;
	char* token = (char*) checked_malloc(500*sizeof(char));
	int sizeof_token = 500;
	int tindex = 0;
	while(input != output)
	{
		if(tindex >= sizeof_token)
		{
			sizeof_token += 500;
			token = checked_realloc(token, sizeof_token+1);
		}
		token[tindex] = stream[input];
		input++;
		tindex++;
	}
	if(norm_char(stream[input]))
	{
		token[tindex] = stream[input];
	}
	inputw = token;
	token = (char*) checked_malloc(500*sizeof(char));
	tindex = 0;
	output++;
	while(output != end)
	{
		if(tindex >= sizeof_token)
		{
			sizeof_token += 500;
			token = checked_realloc(token, sizeof_token+1);
		}
		token[tindex] = stream[output];
		output++;
		tindex++;
	}
	if(norm_char(stream[output]))
	{
		token[tindex] = stream[output];
	}
	outputw = token;
 }
 else if(input != 0)
 {
	input++;
	char* token = (char*) checked_malloc(500*sizeof(char));
	int sizeof_token = 500;
	int tindex = 0;
	while(input != end)
	{
		if(tindex >= sizeof_token)
		{
			sizeof_token += 500;
			token = checked_realloc(token, sizeof_token+1);
		}
		token[tindex] = stream[input];
		input++;
		tindex++;
	}
	if(norm_char(stream[input]))
	{
		token[tindex] = stream[input];
	}
	inputw = token;

 }
 else if(output != 0)
 {
	output++;
	char* token = (char*) checked_malloc(500*sizeof(char));
	int sizeof_token = 500;
	int tindex = 0;
	while(output != end)
	{
		if(tindex >= sizeof_token)
		{
			sizeof_token += 500;
			token = checked_realloc(token, sizeof_token+1);
		}
		token[tindex] = stream[output];
		output++;
		tindex++;
	}
	if(norm_char(stream[output]))
	{
		token[tindex] = stream[output];
	}
	outputw = token;
 }
 //handle simple command
 if(sindex == 0)
 {
	char** word = (char**)checked_malloc(50*sizeof(char*));
	int sizeof_word = 50;
	int windex = 0;
	char* token = (char*) checked_malloc(500*sizeof(char));
	int sizeof_token = 500;
	int tindex = 0;
	cmds->cmd->type = SIMPLE_COMMAND;
	while(sindex < end && stream[sindex] != '<' && stream[sindex] != '>')
	{
		if(windex >= sizeof_word)
		{
			sizeof_word += 50;
			word = checked_realloc(word, sizeof_word+1);
		}
		if(tindex >= sizeof_token)
		{
			sizeof_token += 500;
			token = checked_realloc(token, sizeof_token+1);
		}
		if(norm_char(stream[sindex]))
		{
			token[tindex] = stream[sindex];
			sindex++;
			tindex++;
		}
		else if(stream[sindex] == ' ')
		{
			word[windex] = token;
			cmds->warray->wordStream[cmds->warray->nword] = token;
			cmds->warray->nword++;
			sindex++;
			windex++;
			token = (char*) checked_malloc(500*sizeof(char));
			tindex = 0;
		}
	}
	if(norm_char(stream[sindex]))
	{
		token[tindex] = stream[sindex];
	}
	word[windex] = token;
	windex++;
	cmds->warray->wordStream[cmds->warray->nword] = token;
	cmds->warray->nword++;
	word[windex] = NULL;
	if(outputw)
	{
		cmds->warray->outputSet[cmds->warray->nout] = outputw;
		cmds->warray->nout++;
	}
	if(inputw)
	{
		cmds->warray->inputSet[cmds->warray->nin] = inputw;
		cmds->warray->nin++;
	}
	cmds->cmd->input = inputw;
	cmds->cmd->output = outputw;
	cmds->cmd->u.word = word;
 }
 //handle subshell
 else if(stream[sindex] == ')')
 {
	sindex--;
	int shellend = sindex;
	int subshells = 1;
	while(sindex != 0 && subshells != 0)
	{
		if(stream[sindex] == ')')
		{
			subshells++;
		}
		if(stream[sindex] == '(')
		{
			subshells--;
		}
		sindex--;
	}
	char* sub = (char*) checked_malloc(end*sizeof(char)+1);
	if(sindex == 0)
	{
		if(outputw)
		{
			cmds->warray->outputSet[cmds->warray->nout] = outputw;
			cmds->warray->nout++;
		}
		if(inputw)
		{
			cmds->warray->inputSet[cmds->warray->nin] = inputw;
			cmds->warray->nin++;
		}
		cmds->cmd->type = SUBSHELL_COMMAND;
		cmds->cmd->input = inputw;
		cmds->cmd->output = outputw;
		stream++;
		strncpy(sub, stream, (size_t)(shellend-sindex));
		cmds->cmd->u.subshell_command = build_command(sub, cmds->warray);
	}
	else
	{
		if(outputw)
		{
			cmds->warray->outputSet[cmds->warray->nout] = outputw;
			cmds->warray->nout++;
		}
		if(inputw)
		{
			cmds->warray->inputSet[cmds->warray->nin] = inputw;
			cmds->warray->nin++;
		}
		hassubshell = 1;
		subshell_cmd->type = SUBSHELL_COMMAND;
		subshell_cmd->input = inputw;
		subshell_cmd->output = outputw;
		sindex += 2;
		char* temp = stream;
		temp += sindex;
		strncpy(sub, temp, (size_t)(shellend-sindex));
		subshell_cmd->u.subshell_command = build_command(sub, cmds->warray);
		sindex -=3;
	}	
 }
 
 //handle sequence commands
 int checker = sindex;
 int ignore = 0;
 while(checker != 0 && (stream[checker] != ';' || ignore != 0))
 {
	if(stream[checker] != ')')
	{
		ignore++;
	}
	if(stream[checker] != '(')
	{
		ignore--;
	}
	if(stream[checker] != '&' || stream[checker] != '|' || stream[checker] != '?')
	{
		complexcommand = 1;
	}
	checker--;
 }
 if(checker != 0)
 {
	char* temp = stream;
	strcpy(temp, stream);
	char* front = (char*) checked_malloc(strlen(stream));
	char* back = (char*) checked_malloc(strlen(stream));
	strncpy(front, temp, (size_t)(checker-1));
	if(hassubshell == 0 || complexcommand != 0)
	{
		temp = stream;
		strcpy(temp, stream);
		temp += (checker + 2);
		strncpy(back, temp, ((size_t)(end-(checker + 1))));
		cmds->cmd->type = SEQUENCE_COMMAND;
		cmds->cmd->u.command[0] = build_command(front, cmds->warray);
		cmds->cmd->u.command[1] = build_command(back, cmds->warray);
		return cmds;
	}
	else
	{
		cmds->cmd->type = SEQUENCE_COMMAND;
		cmds->cmd->u.command[0] = build_command(front, cmds->warray);
		cmds->cmd->u.command[1] = subshell_cmd;
		return cmds;
	}
 }

 // handle and and or commands
 checker = sindex;
 ignore = 0;
 while(checker != 0 && ((stream[checker] != '&' && stream[checker] != '?') || ignore != 0))
 {
	if(stream[checker] != ')')
	{
		ignore++;
	}
	if(stream[checker] != '(')
	{
		ignore--;
	}
	if(stream[checker] != '|')
	{
		complexcommand = 1;
	}

	checker--;
 }
 if(checker != 0)
 {
	if(stream[checker] == '&')
	{
		cmds->cmd->type = AND_COMMAND;
	}
	else
	{
		cmds->cmd->type = OR_COMMAND;
	}
	char* temp = stream;
	strcpy(temp, stream);
	char* front = (char*) checked_malloc(strlen(stream));
	char* back = (char*) checked_malloc(strlen(stream));
	strncpy(front, temp, (size_t)(checker-1));
	if(hassubshell == 0 || complexcommand != 0)
	{
		temp = stream;
		strcpy(temp, stream);
		temp += (checker + 2);
		strncpy(back, temp, ((size_t)(end-(checker + 1))));
		cmds->cmd->u.command[0] = build_command(front, cmds->warray);
		cmds->cmd->u.command[1] = build_command(back, cmds->warray);
		return cmds;
	}
	else
	{
		cmds->cmd->u.command[0] = build_command(front, cmds->warray);
		cmds->cmd->u.command[1] = subshell_cmd;
		return cmds;
	}
 }

  // handle pipe commands
 checker = sindex;
 ignore = 0;
 while(checker != 0 && (stream[checker] != '|' || ignore != 0))
 {
	if(stream[checker] != ')')
	{
		ignore++;
	}
	if(stream[checker] != '(')
	{
		ignore--;
	}
	checker--;
 }
 if(checker != 0)
 {
	cmds->cmd->type = PIPE_COMMAND;
	char* temp = stream;
	strcpy(temp, stream);
	char* front = (char*) checked_malloc(strlen(stream));
	char* back = (char*) checked_malloc(strlen(stream));
	strncpy(front, temp, (size_t)(checker-1));
	if(hassubshell == 0)
	{
		temp = stream;
		strcpy(temp, stream);
		temp += (sindex + 2);
		strncpy(back, temp, ((size_t)(end-(checker + 1))));
		cmds->cmd->u.command[0] = build_command(front, cmds->warray);
		cmds->cmd->u.command[1] = build_command(back, cmds->warray);
		return cmds;
	}
	else
	{
		cmds->cmd->u.command[0] = build_command(front, cmds->warray);
		cmds->cmd->u.command[1] = subshell_cmd;
		return cmds;
	}
 }

 return cmds;
}

command_stream_t order_commands(struct command_stream* cmdstack, char* stream)
{
 struct cstack* cmds = (struct cstack*) checked_malloc(sizeof(struct cstack));
 cmds->cmd = (struct command*)checked_malloc(sizeof(struct command));
 cmds->cmd->u.word = NULL;
 cmds->prev = NULL;
 cmds->next = NULL;
 cmds->count = 0;
 
 while(cmds->cmd != NULL)
 {
	
	cmds = create_command(cmds, stream);
	if(cmds->cmd != NULL)
	{
		int tcount = cmds->count;
		struct cstack* temp = cmds;
		cmds = cmds->prev;
		cmds = (struct cstack*) checked_malloc(sizeof(struct cstack));
		cmds->count = tcount;
		cmds->cmd = (struct command*)checked_malloc(sizeof(struct command));
		cmds->cmd->u.word = NULL;
		cmds->next = temp;
		temp->prev = cmds;
		cmds->prev = NULL;
	}
 }
 cmds = cmds->next;
 cmds->prev = NULL;
 while(cmds->next != NULL)
 {
	cmds = cmds->next;
 }
 cmdstack->commands = cmds;
 return cmdstack;
}


command_stream_t make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
 //initiate relevant variables

 struct command_stream* comPtr = (struct command_stream*) checked_malloc(sizeof(struct command_stream));
 char* stream = (char*) checked_malloc(500*sizeof(char));
 int sizeof_stream = 500;
 char elem = get_next_byte(get_next_byte_argument);
 int index = 0;
 int paren_count = 0;
 int incommand = 0;
 int linenumber = 1;
 //skip any preceding whitespace or newlines
 while(elem == ' ' || elem == '\t' || elem == '\n')
 {
	if(elem == '\n')
	{
		linenumber++;
	}
	elem = get_next_byte(get_next_byte_argument);
 }
 
 //skip any preceding comments
 while(elem == '#')
 {
	while(elem != '\n')
	{
		elem = get_next_byte(get_next_byte_argument);
	}
	elem = get_next_byte(get_next_byte_argument);
	while(elem == ' ' || elem == '\t' || elem == '\n')
	{
		if(elem == '\n')
		{
			linenumber++;
		}
		elem = get_next_byte(get_next_byte_argument);
	}	
 }

 //return NULL if no data
 if(elem == EOF)
 {
	return NULL;
 }

 //error if starting character is invalid
 if(elem == '|' || elem == '&' || elem == ')' || elem == '>' || elem == '<' || elem == ';')	
 {
	fprintf(stderr, "%d: invalid starting character\n", linenumber);
	exit(EXIT_FAILURE);
 }

 //scan stream until end of file
 while(elem != EOF)
 {
	//grow stream if needed
	if(index >= sizeof_stream)
		{
			sizeof_stream += 500;
			stream = checked_realloc(stream, sizeof_stream+1);
		}

       if (norm_char(elem))
	{
		stream[index] = elem;
		incommand = 1;
		index++;
		elem = get_next_byte(get_next_byte_argument);
	}

	else if(elem == ' ' || elem == '\t')
	{
		while(elem == ' ' || elem == '\t')
		{
			elem = get_next_byte(get_next_byte_argument);
		}
		if(elem == '<' || elem == '>')
		{}
		else
		{
		stream[index] = ' ';
		index++;
		}
	}

	else if(elem == '\n')
	{
		stream[index] = elem;
		index++;
		incommand = 0;
		while(elem == ' ' || elem == '\t' || elem == '\n')
		{
			if(elem == '\n')
			{
				linenumber++;
			}
			elem = get_next_byte(get_next_byte_argument);
		}
		if(special_char(elem, 1))
		{
			fprintf(stderr, "%d: invalid starting character\n", linenumber);
			exit(EXIT_FAILURE);
		}
	}
	
	else if(elem == '<')
	{
		
		stream[index] = elem;
		index++;
		elem = get_next_byte(get_next_byte_argument);
		while(elem == ' ' || elem == '\t')
		{
			elem = get_next_byte(get_next_byte_argument);
		}
		if(special_char(elem, 0) || elem == '\n'  || elem == EOF)
		{
			fprintf(stderr, "%d: invalid input\n", linenumber);
			exit(EXIT_FAILURE);
		}
		while(norm_char(elem))
		{
			stream[index] = elem;
			index++;
			elem = get_next_byte(get_next_byte_argument);
		}
		while(elem == ' ' || elem == '\t')
		{
			elem = get_next_byte(get_next_byte_argument);
		}
		if(elem == '>')
		{
			stream[index] = elem;
			index++;
			elem = get_next_byte(get_next_byte_argument);
			while(elem == ' ' || elem == '\t')
			{
				elem = get_next_byte(get_next_byte_argument);
			}
			if(special_char(elem, 0) || elem == '\n' || elem == EOF)
			{
				fprintf(stderr, "%d: invalid output\n", linenumber);
				exit(EXIT_FAILURE);
			}
			while(norm_char(elem))
			{
				stream[index] = elem;
				index++;
				elem = get_next_byte(get_next_byte_argument);
			}
			while(elem == ' ' || elem == '\t')
			{
				elem = get_next_byte(get_next_byte_argument);
			}
			if(elem != '\n')
			{
				stream[index] = ' ';
				index++;
			}
			if(elem == '>' || elem == '<')
			{
				fprintf(stderr, "%d: invalid character\n", linenumber);
				exit(EXIT_FAILURE);
			}
		}
		else if(elem == '<')
		{
			fprintf(stderr, "%d: invalid character\n", linenumber);
			exit(EXIT_FAILURE);
		}
		else
		{
			if(elem != '\n')
			{
				stream[index] = ' ';
				index++;
			}
		}
	}
	else if(elem == '>')
	{
		stream[index] = elem;
		index++;
		elem = get_next_byte(get_next_byte_argument);
		while(elem == ' ' || elem == '\t')
		{
			elem = get_next_byte(get_next_byte_argument);
		}
		if(special_char(elem, 0) || elem == '\n'  || elem == EOF)
		{
			fprintf(stderr, "%d: invalid output\n", linenumber);
			exit(EXIT_FAILURE);
		}
		while(norm_char(elem))
		{
			stream[index] = elem;
			index++;
			elem = get_next_byte(get_next_byte_argument);
		}
		while(elem == ' ' || elem == '\t')
		{
			elem = get_next_byte(get_next_byte_argument);
		}
		if(elem != '\n')
		{
			stream[index] = ' ';
			index++;
		}
		if(elem == '>' || elem == '<')
		{
			fprintf(stderr, "%d: invalid character\n", linenumber);
			exit(EXIT_FAILURE);
		}
	}
	else if(elem == ';')
	{
		if(index >= 1 && stream[index-1] != ' ')
		{
			stream[index] = ' ';
			index++;
		}
		stream[index] = elem;
		index++;
		elem = get_next_byte(get_next_byte_argument);
		while(elem == ' ' || elem == '\t' || elem == '\n')
		{
			elem = get_next_byte(get_next_byte_argument);
		}
		stream[index] = ' ';
		index++;
		if(elem == EOF || elem == ')')
		{
			index--;
		}
		if(elem ==';')
		{
			fprintf(stderr, "%d: invalid character\n", linenumber);
			exit(EXIT_FAILURE);
		}

	}
	else if(elem == '(')
	{
	if(index >= 2 && (!special_char(stream[index-2], 0) && stream[index-2] != '?') && 
	   (stream[index-1] != '\n' && stream[index-1] != ';' && stream[index-1] != '('))
		{
			fprintf(stderr, "%d: invalid character\n", linenumber);
			exit(EXIT_FAILURE);
		}
		stream[index] = elem;
		index++;
		paren_count++;
		incommand = 1;
		elem = get_next_byte(get_next_byte_argument);
		while(elem == ' ' || elem == '\t' || elem == '\n')
		{
			elem = get_next_byte(get_next_byte_argument);
		}
		if(special_char(elem, 1))
		{
			fprintf(stderr, "%d: invalid character\n", linenumber);
			exit(EXIT_FAILURE);
		}
	}
	else if(elem == ')')
	{
		if(index >= 1 && stream[index-1] == ' ')
		{
			stream[index-1] = elem;
		}
		else
		{
			stream[index] = elem;
			index++;
		}
		elem = get_next_byte(get_next_byte_argument);
		paren_count--;
		while(elem == ' ' || elem == '\t')
		{
			elem = get_next_byte(get_next_byte_argument);
		}
		if(norm_char(elem))
		{
			fprintf(stderr, "%d: invalid character\n", linenumber);
			exit(EXIT_FAILURE);
		}
	}
	else if(elem == '&')
	{
		if(index >= 1 && stream[index-1] != ' ')
		{
			stream[index] = ' ';
			index++;
		}
		stream[index] = elem;
		index++;
		elem = get_next_byte(get_next_byte_argument);
		if(elem == '&')
		{
			elem = get_next_byte(get_next_byte_argument);
			while(elem == ' ' || elem == '\t' || elem == '\n')
			{
				if(elem == '\n')
				{
					linenumber++;
				}
				elem = get_next_byte(get_next_byte_argument);
			}
			stream[index] = ' ';
			index++;
			if(special_char(elem, 1)  || elem == EOF)
			{
				fprintf(stderr, "%d: invalid character\n", linenumber);
				exit(EXIT_FAILURE);
			}
		}
		else
		{
			fprintf(stderr, "%d: invalid expression\n", linenumber);
			exit(EXIT_FAILURE);
		}

	}
	else if(elem == '|')
	{
		if(index >= 1 && stream[index-1] != ' ')
		{
			stream[index] = ' ';
			index++;
		}
		elem = get_next_byte(get_next_byte_argument);
		if(elem == '|')
		{
			//use ? as || 
			stream[index] = '?';
			index++;
			stream[index] = ' ';
			index++;
			elem = get_next_byte(get_next_byte_argument);
			
			while(elem == ' ' || elem == '\t' || elem == '\n')
			{
				if(elem == '\n')
				{
					linenumber++;
				}
				elem = get_next_byte(get_next_byte_argument);
			}
			
			if(special_char(elem, 1) || elem == EOF)
			{
				fprintf(stderr, "%d: invalid character\n", linenumber);
				exit(EXIT_FAILURE);
			}
		}
		else
		{
			stream[index] = '|';
			index++;
			stream[index] = ' ';
			index++;
			while(elem == ' ' || elem == '\t' || elem == '\n')
			{
				if(elem == '\n')
				{
					linenumber++;
				}
				elem = get_next_byte(get_next_byte_argument);
			}
			if(special_char(elem, 1)  || elem == EOF)
			{
				fprintf(stderr, "%d: invalid character\n", linenumber);
				exit(EXIT_FAILURE);
			}
		}

	}
	else if(elem == '#')
	{
		if(incommand == 0)
		{
			while(elem != '\n')
			{
				elem = get_next_byte(get_next_byte_argument);
			}
			while(elem == ' ' || elem == '\t' || elem == '\n')
			{
				if(elem == '\n')
				{
					linenumber++;
				}
				elem = get_next_byte(get_next_byte_argument);
			}
		}
		else
		{
			fprintf(stderr, "%d: invalid comment\n", linenumber);
			exit(EXIT_FAILURE);
		}
	}
	//exit if invalid character	
	else
	{
		fprintf(stderr, "%d: invalid character\n", linenumber);
		exit(EXIT_FAILURE);
	}
 }
 
 stream[index] = EOF;
 if(paren_count != 0)
 {
	fprintf(stderr, "%d: invalid parentheses count\n", linenumber);
	exit(EXIT_FAILURE);
 }
 comPtr = order_commands(comPtr, stream);

 //int cnt =1;
 int m,n,nwords,nins,nouts,ccount,cwords,cins,couts,nbe,naf,ncount;
 struct cstack* k = comPtr->commands;
 struct cstack* j;
 int isdone = 0;
 ccount = 0;
 nbe = 0;
 naf = 0;

 //establishing dependency relationships among commands
 while (k != NULL)
 {
	cwords = k->warray->nword;
	cins = k->warray->nin;
	couts = k->warray->nout;
	//allocating sufficient amount of data to avoid realloc
	k->cmd->before = (struct command**)malloc(50*sizeof(struct command*));
	k->cmd->after  = (struct command**)malloc(50*sizeof(struct command*));

	j = comPtr->commands; //reset ptr
	ncount = 0;
	while (j != NULL)
	{
		isdone = 0;
		if (ccount == ncount) //avoid comparing the same command
		{
			ncount++;
			struct cstack* tmp = j;
			j = tmp->prev;
			ncount++;
			continue;
		}
		nwords = j->warray->nword;
		nins = j->warray->nin;
		nouts = j->warray->nout;
		if (ccount == 0) //for the first command, not necessary to worry about commands appearing before
		{
			for (m = 0; m < couts; m++)
			{ 
				for (n = 0; n < nouts; n++)
				{
					
					if (!strcmp(k->warray->outputSet[m],j->warray->outputSet[n]))
					{
						//still need to check for Write Write dependency
						//ex:
						//#1 sort < maxwell.txt > james.txt && grep -c h james.txt
						//#2 sort otto.txt > james.txt 	
						//if #2 is executed first, 1's grep -c h might output something different
						if (ccount < ncount)
						{
							k->cmd->after[naf] = j->cmd;
							naf++;
						}
						//same logic as above
						else
						{
							k->cmd->before[nbe] = j->cmd;
							nbe++;
						}
						isdone = 1;
						break; 
					}

				}
				if (isdone)
				{break;}
				
				//compar
				//e current command's outputs against every word and input of other commands	
				for (n = 0; n < nwords; n++)
				{
					if (!strcmp(k->warray->outputSet[m],j->warray->wordStream[n]))
					{	
						k->cmd->after[naf] = j->cmd;
						naf++;
						isdone = 1;
						break; 
					}

				}
				//if you find at least one dependency, no need to check for others. move on to the next command for comparison
				if (isdone) 
				{break;}
				
				for (n = 0; n < nins; n++)
				{
					if (!strcmp(k->warray->outputSet[m],j->warray->inputSet[n]))
					{	
						k->cmd->after[naf] = j->cmd;
						naf++;
						isdone = 1;
						break; 
					}
				}
				//if you find at least one dependency, no need to check for others. move on to the next command for comparison
				if (isdone)
				{break;}

			}

			if (isdone)
			{ 
				struct cstack* tmp = j;
				j = tmp->prev;
				ncount++;
				continue;
			}

			for (m = 0; m < cins; m++)
			{
				
				for (n = 0; n < nouts; n++)
				{
					//if current command writes to something that is accessed as an input in later commands
					if (!strcmp(k->warray->inputSet[m],j->warray->outputSet[n]))
					{	
						//ex:
						//#1 sort < james.txt > cordon.txt && grep -c h
						//#2 echo green > james.txt
						//if james is overwritten first with green, grep -c h might produce a different value
						if (ccount < ncount)
						{
							k->cmd->after[naf] = j->cmd;
							naf++;
						}
						//ex:
						//#1 echo green > james.txt
						//#2 sort < james.txt > cordon.txt && grep -c h	
						else
						{
							k->cmd->before[nbe] = j->cmd;
							nbe++;
						}
						isdone = 1;
						break; 
					}

				}
				// there is no dependency between in-in and in-word
				//ex: 
				// #1 sort james.txt 
				// #2 grep -c james.txt 
				// #3 sort < james.txt > cordon.txt
				if (isdone)
				{break;}

			}

			if (isdone)
			{ 
				struct cstack* tmp = j;
				j = tmp->prev;
				ncount++;
				continue;
			}

			for (m = 0; m < cwords; m++)
			{
				
				for (n = 0; n < nouts; n++)
				{
					//if current command writes to something that is accessed as an input in later commands
					if (!strcmp(k->warray->wordStream[m],j->warray->outputSet[n]))
					{
						//ex:
						//#1 grep -c h james.txt
						//#2 sort < cordon.txt > james.txt
						//still R W dependency		
						if (ccount < ncount)
						{
							k->cmd->after[naf] = j->cmd;
							naf++;
						}
						//ex:
						//#1 sort < cordon.txt > james.txt
						//#2 grep -c h james.txt
						else
						{
							k->cmd->before[nbe] = j->cmd;
							nbe++;
						}
						isdone = 1;
						break; 
					}

				}

				// there is no dependency between in-in and in-word
				//ex: 
				// #1 sort james.txt 
				// #2 grep -c james.txt 
				if (isdone)
				{break;}
			}
		}

		else //for other commands, need to check both commands appearing before and after
		{

			for (m = 0; m < couts; m++)
			{
				
				for (n = 0; n < nouts; n++)
				{
					if (!strcmp(k->warray->outputSet[m],j->warray->outputSet[n]))
					{
						//still need to check for Write Write dependency
						//ex:
						//#1 sort < maxwell.txt > james.txt && grep -c h james.txt
						//#2 sort otto.txt > james.txt 	
						//if #2 is executed first, 1's grep -c h might output something different
						if (ccount < ncount)
						{
							k->cmd->after[naf] = j->cmd;
							naf++;
						}
						//same logic as above
						else
						{
							k->cmd->before[nbe] = j->cmd;
							nbe++;
						}
						isdone = 1;
						break; 
					}

				}

				if (isdone)
				{break;}
				
				for (n = 0; n < nwords; n++)
				{
					//if current command writes to something that is accessed as an input in later commands
					if (!strcmp(k->warray->outputSet[m],j->warray->wordStream[n]))
					{	
						//#ex:
						//sort max.txt > james.txt
						//cat james.txt && grep -c -h o
						if (ccount < ncount)
						{
							k->cmd->after[naf] = j->cmd;
							naf++;
						}
						//#ex:
						//cat james.txt && grep -c -h o
						//sort max.txt > james.txt
						else
						{
							k->cmd->before[nbe] = j->cmd;
							nbe++;
						}
						isdone = 1;
						break; 
					}

				}
				if (isdone)
				{break;}

				for (n = 0; n < nins; n++)
				{
					if (!strcmp(k->warray->outputSet[m],j->warray->inputSet[n]))
					{	
						//#1 echo free > james.txt
						//#2 sort < james.txt > darton.txt
						//read must wait for write
						if (ccount < ncount)
						{
							k->cmd->after[naf] =  j->cmd;
							naf++;
						}
						//#1 sort < james.txt > darton.txt && cat darton.txt
						//#2 (echo free && echo slave) > james.txt
						//still read write dependency
						//if 2 is executed first, darton might have a different data
						else
						{
							k->cmd->before[nbe] = j->cmd;
							nbe++;
						}
						isdone = 1;
						break; 
					}
				}
				if (isdone)
				{break;}
			}

			if (isdone)
			{ 
				struct cstack* tmp = j;
				j = tmp->prev;
				ncount++;
				continue;
			}

			for (m = 0; m < cins; m++)
			{
				
				for (n = 0; n < nouts; n++)
				{
					//if current command writes to something that is accessed as an input in later commands
					if (!strcmp(k->warray->inputSet[m],j->warray->outputSet[n]))
					{	
						//ex:
						//#1 sort < james.txt > cordon.txt && grep -c h
						//#2 echo green > james.txt
						//if james is overwritten first with green, grep -c h might produce a different value
						if (ccount < ncount)
						{
							k->cmd->after[naf] = j->cmd;
							naf++;
						}
						//ex:
						//#1 echo green > james.txt
						//#2 sort < james.txt > cordon.txt && grep -c h	
						else
						{
							k->cmd->before[nbe] = j->cmd;
							nbe++;
						}
						isdone = 1;
						break; 
					}

				}
				// there is no dependency between in-in and in-word
				//ex: 
				// #1 sort james.txt 
				// #2 grep -c james.txt 
				// #3 sort < james.txt > cordon.txt
				if (isdone)
				{break;}

			}

			if (isdone)
			{ 
				struct cstack* tmp = j;
				j = tmp->prev;
				ncount++;
				continue;
			}

			for (m = 0; m < cwords; m++)
			{
				
				for (n = 0; n < nouts; n++)
				{
					//if current command writes to something that is accessed as an input in later commands
					if (!strcmp(k->warray->wordStream[m],j->warray->outputSet[n]))
					{
						//ex:
						//#1 grep -c h james.txt
						//#2 sort < cordon.txt > james.txt
						//still R W dependency		
						if (ccount < ncount)
						{
							k->cmd->after[naf] = j->cmd;
							naf++;
						}
						//ex:
						//#1 sort < cordon.txt > james.txt
						//#2 grep -c h james.txt
						else
						{
							k->cmd->before[nbe] = j->cmd;
							nbe++;
						}
						isdone = 1;
						break; 
					}

				}

				// there is no dependency between in-in and in-word
				//ex: 
				// #1 sort james.txt 
				// #2 grep -c james.txt 
				if (isdone)
				{break;}
			}



		}		
		struct cstack* tmp = j;
		j = tmp->prev;
		ncount++;

	}
 	k->cmd->bsize = nbe;
	k->cmd->asize  = naf;
	//reset counter variables
	nbe = 0; 
	naf = 0; 			
	struct cstack* c = k;
	k = c->prev;
	ccount++;
	
 }

 //building dependency tree
 /*
 struct cstack* iter = comPtr->commands;
 while (iter != NULL)
 {
	if (iter->cmd->bsize == 0 && iter->cmd->asize == 0)
	{
		enque(&comPtr->independent,iter->cmd);
	}
	

	else
	{ 
		enque(&comPtr->dependent,iter->cmd);
	}	

	struct cstack* c = iter;
	iter = c->prev;
 }*/

 /* 
 struct cstack* jdog = comPtr->commands;
 
 
 while (comPtr->commands != NULL)
 {	
	printf("before: %d ",comPtr->commands->cmd->bsize);
	printf("after: %d ",comPtr->commands->cmd->asize);
	printf("\n");

	struct cstack*c  = comPtr->commands;
	comPtr->commands = c->prev;
 }

 
 comPtr->commands = jdog;
 */

 return comPtr;
}

command_t
read_command_stream (command_stream_t s)
{

 if( s == NULL)
 {
	return NULL;
 }
 if(s->commands != NULL)
 {
 struct cstack* c = s->commands;
 s->commands = c->prev;
 return c->cmd;
 }
 return NULL;
} 
