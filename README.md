# Meme Academy

An OpenJK build for the JKA speedrunning community that adds support for
in-game Twitch chat.

<a href="http://www.youtube.com/watch?feature=player_embedded&v=g9HrPJq-kow" target="_blank">
    <img src="http://img.youtube.com/vi/g9HrPJq-kow/0.jpg" border="10"/>
</a>

## Disclaimer

This build will allow users in your Twitch chat to execute commands against the
JKA client console.  This is potentially dangerous and you should first
understand the risks before using.

## Connecting to Twitch

Connection information for the IRC server is stored and read as cvars.  The
cvars for connecting are as follows:

```
  cl_ircHost
  cl_ircPort
  cl_ircUsername
  cl_ircPassword
```

If custom cvars are not provided, the client will connect to
"irc.chat.twitch.tv" as an anonymous user.

If the connection fails, the client must be restarted.

## Twitch User Commands

Once connected to a channel, users in Twitch chat can can invoke console
commands with the `!` prefix:
```
!r_showtris 1
!saber sith_sword
```

All commands are in the exact format as they are accepted in the game console.

The following commands are always blocked:
```
say
join
kill
devmap
cl_irc*
```

Any variables that are changed will be restored to the original value after 20
seconds.

## Custom Commands

### `say`

Sends a message to the current channel:
```
say Hey, what's going on?
```

### `join`

Joins a channel on the IRC server:
```
join #your_solution
```

### `block`/`unblock`

Blocks/unblocks a command from being executed by chat:
```
block spawn
unblock spawn
```

## Custom Variables

### `cl_ircShowChat`

Draws Twitch chat within the game client:
```
cl_ircShowChat 1
cl_ircShowChat 0
```

## Credits

* [OpenJK](https://github.com/JACoders/OpenJK) for keeping the OSS release of
  this game alive
* [YS](https://github.com/ryanjphillips) for the teaser video, testing, and live-streaming the debut
* [Sphere](https://github.com/kugelrund) for [Speed-Academy](https://github.com/kugelrund/Speed-Academy)
* The JKA Speedrunning community for being awesome
