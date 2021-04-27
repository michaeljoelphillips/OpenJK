#include <stdio.h>
#include <time.h>

#include "client.h"

typedef struct {
  const char* command;
  time_t scheduled_execution_time;
} restore_command_t;

int restore_queue_index = 0;
const restore_command_t *restore_queue[256] = { NULL };

void queueRestoreCommand(const char*, const char*);
int applyCvar(const char *message);

/*
======================
 Handle an incoming IRC message

 If the message is prefixed with a `!` character, treat it like a console
 command.
======================
*/
void CL_HandleIRCMessage (const char *user, const char *message) {
  if (strncmp(message, "!", 1)) {
    return;
  }

  if (!applyCvar(++message)) {
    return;
  }

  /* if (strcmp(message, "kill")) {return;} */
  /* if (strcmp(message, "blind")) {return;} */
  /* if (strcmp(message, "damage")) {return;} */

  Cmd_ExecuteString(message);
}

void CL_TickRestoreCommandQueue (void) {
  time_t current_time = time(NULL);
  const restore_command_t *current_command;

  for (int i = 0; i < 256; i++) {
    current_command = restore_queue[i];

    if (current_command == NULL) {
      continue;
    }

    // Commands are added to the queue in order, if we encounter one that has
    // an execution time greater than the current time, we can bail on the rest
    // of the routine.
    if (current_command->scheduled_execution_time > current_time) {
      break;
    }

    Com_Printf("Restoring command: %s", current_command->command);

    Cmd_ExecuteString(current_command->command);

    restore_queue[i] = NULL;
    free((void*) current_command);
  }
}

/*
======================
 Execute an incoming IRC message as a console cvar modifier

 If the command is a valid cvar modifier, the state of the cvar prior to the
 execution of the command will be added to the restore queue.

 If the cvar cannot be found, 1 is returned.
======================
*/
int applyCvar(const char *message) {
  char *cvar_value;
  char *cvar_name = (char*) malloc(sizeof(char) * strlen(message + 1));

  // Copy the contents of message to cvar_name and tokenize it by a whitespace
  // to obtain the value of the name.
  strcpy(cvar_name, message);
  cvar_name = strtok((char*) cvar_name, " ");

  cvar_t *cvar = Cvar_FindVar(cvar_name);

  if (cvar == NULL) {
    return 1;
  }

  cvar_value = (char*) malloc(sizeof(char) * strlen(cvar->string));
  strcpy(cvar_value, cvar->string);

  Com_Printf("CVAR was found! %s \n", message);

  Cmd_ExecuteString(message);
  queueRestoreCommand(cvar_name, cvar_value);

  return 0;
}

void queueRestoreCommand(const char *cvar_name, const char *cvar_value) {
  restore_command_t *restore_command = (restore_command_t*) malloc(sizeof(restore_command_t));

  char *command = (char*) malloc(sizeof(char) * strlen(cvar_name) + sizeof(char) * strlen(cvar_value) + 1);
  sprintf(command, "%s %s", cvar_name, cvar_value);

  restore_command->command = command;
  restore_command->scheduled_execution_time = time(NULL) + 30;

  restore_queue[restore_queue_index] = restore_command;

  if (restore_queue_index < 256) {
    restore_queue_index++;
  } else {
    restore_queue_index = 0;
  }
}
