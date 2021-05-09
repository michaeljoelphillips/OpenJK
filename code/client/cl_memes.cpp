#include <stdio.h>
#include <time.h>

#include "client.h"

#define RESTORE_QUEUE_SIZE 256
#define RESTORE_COMMAND_TIMER 20

#define DEFAULT_BLOCKED_IRC_COMMANDS "say,join,kill,devmap,cl_irc"

/*
======================
 Represents a command that should be executed at `scheduled_execution_time` for
 the purposes of returning the game state back to normal after having recieved
 a command over IRC.

 Example:

  Given the cvar `cg_fov` and its value `112`
  When the IRC command `!cg_fov 999` is received
  Then `cg_fov` is set to `999`
  And a `cg_fov 112` is added to the restore command queue
======================
*/
typedef struct {
  const char* command;
  time_t scheduled_execution_time;
} restore_command_t;


int restore_queue_index = 0;

restore_command_t *restore_command_queue[RESTORE_QUEUE_SIZE] = { NULL };

int isCommandBlocked(const char*);
int applyCvar(const char*, const char*);
void queueRestoreCommand(const char*, const char*);

/*
======================
 Handle an incoming IRC message

 If the message is prefixed with a `!` character, execute it as a console
 command.
======================
*/
void CL_HandleIRCMessage (const char *user, const char *message) {
  char *command_name;

  if (strncmp(message, "!", 1)) {
    return;
  }

  // Copy the contents of `message` to `command_name` and tokenize it by a
  // whitespace to obtain the command name.
  command_name = (char*) malloc(sizeof(char) * strlen(message + 1));
  strcpy(command_name, ++message);
  command_name = strtok(command_name, " ");

  if (isCommandBlocked(command_name)) {
    return;
  }

  applyCvar(command_name, message);

  Cmd_ExecuteString(message);

  free(command_name);
}

int isCommandBlocked(const char* command_name) {
  bool is_blocked = false;
  char *blocked_commands;
  cvar_t *cl_ircBlockedCommands;

  cl_ircBlockedCommands = Cvar_Get("cl_ircBlockedCommands", DEFAULT_BLOCKED_IRC_COMMANDS, CVAR_ARCHIVE_ND);
  blocked_commands = (char*) malloc(sizeof(char) * strlen(cl_ircBlockedCommands->string) + 1);
  strcpy(blocked_commands, cl_ircBlockedCommands->string);

  char *token = strtok(blocked_commands, ",");

  while (token != NULL) {
    if (strncmp(command_name, token, strlen(token)) == 0) {
      is_blocked = true;

      break;
    }

    token = strtok(NULL, ",");
  }

  free(blocked_commands);

  return is_blocked;
}

/*
======================
 Execute an incoming IRC message as a console cvar modifier

 If the command is a valid cvar modifier, the state of the cvar prior to the
 execution of the command will be added to the restore queue.

 `1` is returned if the cvar cannot be found.
======================
*/
int applyCvar(const char *command_name, const char *message) {
  cvar_t *cvar;
  char *cvar_value;

  cvar = Cvar_FindVar(command_name);

  if (cvar == NULL) {
    return 1;
  }

  cvar_value = (char*) malloc(sizeof(char) * strlen(cvar->string));
  strcpy(cvar_value, cvar->string);

#ifdef _DEBUG
  Com_Printf("Found cvar, executing command: %s\n", message);
#endif

  queueRestoreCommand(command_name, cvar_value);

  free(cvar_value);

  return 0;
}

void queueRestoreCommand(const char *cvar_name, const char *cvar_value) {
  for (int i = 0; i < RESTORE_QUEUE_SIZE; i++) {
    if (restore_command_queue[i] == NULL) {
      continue;
    }

    if (strncmp(cvar_name, restore_command_queue[i]->command, strlen(cvar_name)) == 0) {
      restore_command_queue[i]->scheduled_execution_time = time(NULL) + RESTORE_COMMAND_TIMER;

      return;
    }
  }

  restore_command_t *restore_command = (restore_command_t*) malloc(sizeof(restore_command_t));
  char *command = (char*) malloc(sizeof(char) * strlen(cvar_name) + sizeof(char) * strlen(cvar_value) + 1);

  sprintf(command, "%s %s", cvar_name, cvar_value);

  restore_command->command = command;
  restore_command->scheduled_execution_time = time(NULL) + RESTORE_COMMAND_TIMER;

  restore_command_queue[restore_queue_index] = restore_command;

  if (restore_queue_index < RESTORE_QUEUE_SIZE) {
    restore_queue_index++;
  } else {
    restore_queue_index = 0;
  }
}

/*
======================
 Process schedule restore commands on the restore command queue

 Commands with a `scheduled_execution_time` greater than the current time will
 be executed.
======================
*/
void CL_ProcessRestoreCommandQueue (void) {
  time_t current_time = time(NULL);
  const restore_command_t *current_command;

  for (int i = 0; i < RESTORE_QUEUE_SIZE; i++) {
    current_command = restore_command_queue[i];

    if (current_command == NULL) {
      continue;
    }

    // Commands are added to the queue in order, if we encounter one that has
    // an execution time greater than the current time, we can bail on the rest
    // of the routine.
    if (current_command->scheduled_execution_time > current_time) {
      break;
    }

#ifdef _DEBUG
    Com_Printf("Restoring game state: %s\n", current_command->command);
#endif

    Cmd_ExecuteString(current_command->command);

    restore_command_queue[i] = NULL;
    free((void*) current_command);
  }
}
