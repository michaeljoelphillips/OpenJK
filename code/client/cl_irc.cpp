#include <stdio.h>
#include <string.h>

#ifdef USE_INTERNAL_IRCCLIENT
#include "ircclient/libircclient.h"
#else
#include <libircclient.h>
#endif

#include "client.h"
#include "qcommon/qcommon.h"

int channel_joined = 0;
irc_session_t *session;

char *current_channel = (char*) malloc(40 * sizeof(char));

void connect(irc_session_t *session, const char *event, const char *origin, const char **params, unsigned int count);
void numeric(irc_session_s *session, unsigned int event, const char * origin, const char ** params, unsigned int count);
void join(irc_session_t *session, const char *event, const char *origin, const char **params, unsigned int count);
void part(irc_session_t *session, const char *event, const char *origin, const char **params, unsigned int count);
void channel(irc_session_t *session, const char *event, const char *origin, const char **params, unsigned int count);

/*
======================
 Initialize the IRC Client

 Connection information for the IRC server is stored and read as cvars.  The
 cvars for connecting are as follows:

   cl_ircHost
   cl_ircPort
   cl_ircUsername
   cl_ircPassword

 If custom cvars are not provided, the client will connect to
 "irc.chat.twitch.tv" as an anonymous user.

 If the connection fails, the client must be restarted.
======================
*/
void CL_InitIRC( void )
{
  cvar_t *cl_ircHost = Cvar_Get("cl_ircHost", "irc.chat.twitch.tv", CVAR_ARCHIVE_ND);
  cvar_t *cl_ircPort = Cvar_Get("cl_ircPort", "6667", CVAR_ARCHIVE_ND);
  cvar_t *cl_ircUsername = Cvar_Get("cl_ircUsername", "justinfan14970", CVAR_ARCHIVE_ND);
  cvar_t *cl_ircPassword = Cvar_Get("cl_ircPassword", "kappa", CVAR_ARCHIVE_ND);

  irc_callbacks_t callbacks = {
    .event_connect = connect,
    .event_join    = join,
    .event_part    = part,
    .event_channel = channel,
    .event_numeric = numeric,
  };

  session = irc_create_session(&callbacks);

  if (!session) {
    Com_Printf("Failed to create the IRC session\n");

    return;
  }

  if (irc_connect(session, cl_ircHost->string, cl_ircPort->integer, cl_ircPassword->string, cl_ircUsername->string, cl_ircUsername->string, NULL)) {
    Com_Printf("Failed to connect to IRC server %s:%d\n", cl_ircHost->string, cl_ircPort->integer);

    irc_destroy_session(session);

    return;
  }
}

void connect(irc_session_t *session, const char *event, const char *origin, const char **params, unsigned int count) {
#ifdef _DEBUG
  Com_Printf("Connect to IRC Server\n");
#endif
}

void numeric(irc_session_s *session, unsigned int event, const char * origin, const char ** params, unsigned int count) {
#ifdef _DEBUG
  Com_Printf("Received IRC numeric event\n");
#endif
}

/*
======================
 The channel event callback

 Received when a message is sent to the connected IRC channel
======================
*/
void channel(irc_session_t *session, const char *event, const char *origin, const char **params, unsigned int count)
{
  const char *user = origin;
  const char *message = params[1];

  if (strncmp(message, "!", 1) == 0) {
    Cmd_ExecuteString(++message);
  }

  Com_Printf("%s: %s\n", user, --message);
}

/*
======================
 The join event callback

 Received when the current user joins an IRC channel
======================
*/
void join(irc_session_t *session, const char *event, const char *origin, const char **params, unsigned int count)
{
  if (channel_joined) {
    Com_Printf("Leaving %s\n", current_channel);

    if (irc_cmd_part(session, current_channel)) {
      Com_Printf("Failed to leave channel %s\n", current_channel);

      return;
    }
  }

  channel_joined = 1;
  strcpy(current_channel, params[0]);

  Com_Printf("Joined channel %s\n", params[0]);
}


/*
======================
 The part event callback

 Received when the current user parts from (leaves) an IRC channel
======================
*/
void part(irc_session_t *session, const char *event, const char *origin, const char **params, unsigned int count)
{
  channel_joined = 0;
  memset(current_channel, 0, 40 * sizeof(char));
}

/*
======================
 Receive events from the IRC server

 This function checks for incoming async IRC events via select().  It must be
 run on the game loop in order for IRC messages to be received.

 Refer to libircclient
======================
*/
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

/*
======================
Send messages to the connection IRC channel from the console:

 say PogChamp

======================
*/
void CL_IRCSay( void )
{
  if (Cmd_Argc() < 1) {
    return;
  }

  irc_cmd_msg(session, current_channel, Cmd_ArgsFrom(1));
}

/*
======================
Join an IRC channel from the console:

 join #your_solution

======================
*/
void CL_IRCJoin( void )
{
  if (Cmd_Argc() < 1) {
    return;
  }

  if (!irc_is_connected(session)) {
    Com_Printf("Not connected to the IRC server");

    return;
  }

  irc_cmd_join(session, Cmd_ArgsFrom(1), NULL);
}
