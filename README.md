````
*This project has been created as part of the 42 curriculum by mhoushma, oshcheho*

# Description

ircserv is a C++ IRC server project. It handles client connections, authentication, channel management, and core IRC commands such as JOIN, PART, PRIVMSG, QUIT, KICK, INVITE, TOPIC, and MODE. A bonus trivia bot is also available with `--bonus`.

# Instructions

## Compilation

```bash
make
````

## Execution

```bash
./ircserv <port> <password>
./ircserv <port> <password> --bonus
```

The server runs on the given port and requires the matching password to register.

## Commands usage

After connecting with an IRC client, send the usual IRC lines:

```text
PASS <password>
NICK <nickname>
USER <username> 0 * :<realname>
JOIN #channel
PRIVMSG #channel :hello
PART #channel
QUIT :bye
```

Channel names must start with `#` or `&`.

## Bonus bot usage

When the server is started with `--bonus`, the trivia bot creates the `#trivia` channel.
To start a game, join `#trivia` and send:

```text
PRIVMSG #trivia :start
```

Then answer with `1`, `2`, `3`, or `4`. After 10 questions, the bot shows the score and waits for `start` again.

# Resources

* RFC 1459 and RFC 2812 IRC documentation
* irssi and WeeChat user documentation
* IRC command references and protocol examples
* 42 project subject and evaluation guidelines

## AI usage

AI was used to help structure comments, implement the trivia bot questions, and create testing scripts.

```
```
