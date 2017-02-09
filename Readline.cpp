
#include "Arduino.h"

#include "Readline.h"
#include "editline.h"
#include "stdio.h"

static Stream *rl_stream = NULL;

String Readline(const char *prompt, Stream *stream)
{
  char *line;
  String ret;

  rl_stream = stream;

  line = readline(prompt);
  add_history(line);
  ret = line;
//  free(line);

  return ret;
}

extern "C"
char ard_getc()
{
  if (rl_stream != NULL) {
    while (!rl_stream->available()) {
      delay(10);
    }
    return rl_stream->read();
  } else {
    while (!Serial.available()) {
      delay(10);
    }
    return Serial.read();
  }
}

extern "C"
void ard_putc(char ch)
{
  if (rl_stream != NULL) {
    rl_stream->print(ch);
  } else {
    Serial.print(ch);
  }
}


