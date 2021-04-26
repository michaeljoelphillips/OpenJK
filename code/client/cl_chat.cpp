#include "client.h"

#define IRC_CHAT_BUFFER_SIZE 10

typedef struct {
  const char *user;
  const char *message;
} irc_message_t;

const irc_message_t *irc_chat_buffer[IRC_CHAT_BUFFER_SIZE] = { NULL };

/*
======================
 Draw IRC on the screen

 Writes the contents of `irc_chat_buffer` to a section of the screen in
 descending order.  (Newest messages on the bottom)

 Chat messages will not be drawn if:

   * `cl_showChat` is disabled (0)
   * The game client is not connected to the server

======================
*/
void CL_DrawChat( void )
{
  if (cls.ircStarted == qfalse) {
    return;
  }

  cvar_t *cl_showChat = Cvar_Get("cl_showChat", "1", CVAR_ARCHIVE_ND);

  // Don't render chat if the client is not an active state (cgame has
  // started), or if chat is disabled.
  if (!cl_showChat->integer || cls.state != CA_ACTIVE) {
    return;
  }

  int x = cls.glconfig.vidWidth * .4;
  int y = cls.glconfig.vidHeight;
  int line_length = 50;
  int max_y_pos = y - 5 * SMALLCHAR_HEIGHT;
  float color[4] = { 1.0, 1.0, 1.0, 1.0 };

  int total_lines;
  int y_line_wrap_start_position;
  const irc_message_t *message;

  for (int i = IRC_CHAT_BUFFER_SIZE; i > 0; i--) {
    if (irc_chat_buffer[i] == NULL) {
      continue;
    }

    if (y < max_y_pos) {
      return;
    }

    message = irc_chat_buffer[i];
    total_lines = strlen(message->message) / line_length + 1;
    y_line_wrap_start_position = y - total_lines * SMALLCHAR_HEIGHT;

    // Convert ascii char values to floats for username colors.
    color[0] = 0.1 * (float) message->user[0];
    color[1] = 0.1 * (float) message->user[1];
    color[2] = 0.1 * (float) message->user[2];

    // Draw the username to the left of the main chat region:
    //
    //   your_solution yoursoFiloni2
    //  drmeowingtonmd OMEGALUL
    //
    SCR_DrawSmallStringExt(x - (strlen(message->user) * SMALLCHAR_WIDTH + SMALLCHAR_WIDTH), y_line_wrap_start_position, message->user, color, qfalse, qfalse);
    SCR_DrawWordWrappedString(x, y_line_wrap_start_position, message->message, line_length);

    y -= SMALLCHAR_HEIGHT * total_lines;
  }
}

void CL_AddChatMessage(const char *user, const char *text) {
  irc_message_t *new_message = (irc_message_t*) malloc(sizeof(irc_message_t));
  const char *message = (const char*) malloc(sizeof(char) * strlen(text) + 1);
  const char *username = (const char*) malloc(sizeof(char) * strlen(user) + 1);

  strcpy((char*) username, user);
  strcpy((char*) message, text);

  new_message->user = username;
  new_message->message = message;

  free((void*) irc_chat_buffer[0]);

  for (int i = 1; i < IRC_CHAT_BUFFER_SIZE; i++) {
    irc_chat_buffer[i - 1] = irc_chat_buffer[i];
  }

  irc_chat_buffer[IRC_CHAT_BUFFER_SIZE - 1] = new_message;
}
