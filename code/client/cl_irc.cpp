#include <stdio.h>
#include <string.h>
#include <libircclient.h>
#include <libirc_rfcnumeric.h>

#ifdef USE_INTERNAL_IRCCLIENT
#include "ircclient/libircclient.h"
#else
#include <libircclient.h>
#endif

#include "client.h"
#include "qcommon/qcommon.h"

#define IRC_COMMAND_CHAR = "!";

int channel_joined = 0;

// @todo: Buffer overflow?
char *current_channel = (char*) malloc(40);
irc_session_t *session;
irc_callbacks_t callbacks;

void connect(irc_session_t *session, const char *event, const char *origin, const char **params, unsigned int count)
{
  Com_Printf("Connected to the IRC Server");
}

void numeric(irc_session_s *session, unsigned int event, const char * origin, const char ** params, unsigned int count)
{
  Com_Printf("Captured Event: %d %s\n", event, origin);
}

// @todo: Parse the user string and display only the nick
void channel(irc_session_t *session, const char *event, const char *origin, const char **params, unsigned int count)
{
  const char *user = origin;
  const char *message = params[1];

  if (strncmp(message, "!", 1) == 0) {
    Cmd_ExecuteString(++message);
  }

  Com_Printf("%s: %s\n", user, message);
}

void join(irc_session_t *session, const char *event, const char *origin, const char **params, unsigned int count)
{
  if (channel_joined) {
    Com_Printf("Disconnecting from %s\n", current_channel);

    if (irc_cmd_part(session, current_channel)) {
      Com_Printf("Failed to leave channel %s\n", current_channel);

      return;
    }
  }

  channel_joined = 1;
  strcpy(current_channel, params[0]);

  Com_Printf("Joined channel %s\n", params[0]);
}

void part(irc_session_t *session, const char *event, const char *origin, const char **params, unsigned int count)
{
  Com_Printf("Parted ways with channel %s\n", params[0]);
}

void CL_InitIRC( void )
{
  int connect_failed;

  memset(&callbacks, 0, sizeof(irc_callbacks_t));

  callbacks.event_connect = connect;
  callbacks.event_numeric = numeric;
  callbacks.event_channel = channel;
  callbacks.event_join    = join;
  callbacks.event_part    = part;

  session = irc_create_session(&callbacks);

  if (!session) {
    Com_Printf("Failed to construct the session");
  }

  connect_failed = irc_connect(session, "irc.chat.twitch.tv", 6667, "oauth:", "", "", NULL);

  Com_Printf("Connection Status: %d", connect_failed);

  if (connect_failed) {
    Com_Printf("Connection failed: %s\n", irc_strerror(irc_errno(session)));

    irc_destroy_session(session);
  }

  if (!irc_is_connected(session)) {
    Com_Printf("The connection was not successful");
  }

  Com_Printf("Connection Status: %d", connect_failed);
}

void CL_IRCRecv( void )
{
  if (!irc_is_connected(session)) {
    return;
  }

  struct timeval tv;
  fd_set in_set, out_set;
  int maxfd = 0;

  tv.tv_usec = 2500;
  tv.tv_sec = 0;

  // Init sets
  FD_ZERO (&in_set);
  FD_ZERO (&out_set);

  irc_add_select_descriptors (session, &in_set, &out_set, &maxfd);

  if ( select(maxfd + 1, &in_set, &out_set, 0, &tv) < 0 )
  {
    return;
  }

  irc_process_select_descriptors(session, &in_set, &out_set);
}

void CL_IRCSay( void )
{
  char *message;

  if (Cmd_Argc() < 1) {
    return;
  }

  message = Cmd_ArgsFrom(1);

  irc_cmd_msg(session, current_channel, message);
}

void CL_IRCJoin( void )
{
  char *channel;

  if (Cmd_Argc() < 1) {
    return;
  }

  if (!irc_is_connected(session)) {
    Com_Printf("Not connected to the IRC server");

    return;
  }

  channel = Cmd_ArgsFrom(1);
  irc_cmd_join(session, channel, NULL);
}
