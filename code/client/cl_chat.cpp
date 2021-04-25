#include "client.h"

const char *irc_chat_buffer[10] = { NULL };

void CL_DrawChat( void )
{
  cvar_t *cl_showChat = Cvar_Get("cl_showChat", "1", CVAR_ARCHIVE_ND);

  if (!cl_showChat->integer) {
    return;
  }

  int x = 300;
  int y = 800;
  int line_length = 50;
  int max_y_pos = y - 5 * SMALLCHAR_HEIGHT;

  /* float color[4] = { 1.0, 1.0, 1.0, 1.0 }; */
  int total_lines;

  for (int i = 10; i > 0; i--) {
    if (irc_chat_buffer[i] == NULL) {
      continue;
    }

    if (y < max_y_pos) {
      return;
    }

    total_lines = strlen(irc_chat_buffer[i]) / line_length + 1;

    SCR_DrawWordWrappedString(x, y - total_lines * SMALLCHAR_HEIGHT, irc_chat_buffer[i], line_length);

    y -= SMALLCHAR_HEIGHT * total_lines;
  }
}

void CL_AddChatMessage( const char *new_message ) {
  char *message = (char*) malloc(sizeof(char) * strlen(new_message) + 1);

  strcpy(message, new_message);

  for (int i = 1; i < 10; i++) {
    irc_chat_buffer[i - 1] = irc_chat_buffer[i];
  }

  irc_chat_buffer[9] = message;
}
