
#include "Arduino.h"

#include "Readline.h"
#include "editline.h"
#include "stdio.h"

static Stream *rl_stream = NULL;

static
uint8_t hex2bin(uint8_t in)
{
  if ((in >= '0') && (in <= '9')) {
    return (in - '0');
  }
  if ((in >= 'A') && (in <= 'F')) {
    return (10 + (in - 'A'));
  }
  if ((in >= 'a') && (in <= 'f')) {
    return (10 + (in - 'a'));
  }
  return 0;
}

static
void unhexdump(uint8_t *buf, uint32_t len)
{
  uint32_t i;
  uint8_t d;
  for (i = 0; i < len/2; i++) {
    d = (hex2bin(buf[i*2]) << 4) | hex2bin(buf[i*2+1]);
    buf[i] = d;
  }
}

// returns string length or length of binary data
// blank line terminates input
// crlf preserved for string output, stripped from binary input
size_t Readline_read_buf(uint8_t *data, size_t len, bool to_bin)
{
  bool crlf = false;
  char ch;
  size_t idx = 0;

  do {
    ch = ard_getc();
    if (ch == '\r') {
      if (crlf)
        break;
      ard_putc('\r');
      ard_putc('\n');
      crlf = true;
    } else {
      ard_putc(ch);
      crlf = false;
    }
    if (!(to_bin && crlf))
      data[idx++] = ch;
  } while (idx < len-1);

  ard_putc('\r');
  ard_putc('\n');

  if (!to_bin) {
    data[idx] = '\0';
    return idx;
  }

  // put 0 in MSB if not aligned
  if (idx % 2 == 1) {
    size_t i;
    for (i = idx; i > 0; i--) {
      data[i] = data[i-1];
    }
    data[0] = '0';
    idx++;
  }

  unhexdump(data, idx);
  return (idx/2);
}

size_t Readline_read_str_data(char *buf, size_t len)
{
  char *tmp;
  size_t idx = 0;
  Readline_read_str_data(&tmp, buf, len, &idx);
  return idx;
}

// use a pre-allocated buffer to fill in a string
void Readline_read_str_data(char **data, char *buf, size_t len, size_t *idx)
{
  char ch;
  size_t l = 0;
  *data = &(buf[*idx]);
  do {
    ch = ard_getc();
    if (ch == '\r') {
      if (l == 0) {
        *data = NULL;
      } else {
        buf[(*idx)++] = '\0';
        ard_putc('\r');
        ard_putc('\n');
      }
      return;
    }
    ard_putc(ch);
    buf[(*idx)++] = ch;
    l++;
  } while (*idx < len-1);
  buf[(*idx)++] = '\0';
}


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

void (*Readline_idle)() = NULL;

extern "C"
char ard_getc()
{
  if (rl_stream != NULL) {
    while (!rl_stream->available()) {
      if (Readline_idle != NULL) {
        (*Readline_idle)();
      } else {
        delay(10);
      }
    }
    return rl_stream->read();
  } else {
    while (!Serial.available()) {
      if (Readline_idle != NULL) {
        (*Readline_idle)();
      } else {
        delay(10);
      }
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

void Readline_print_command_list(Readline_cmd_list *command_list)
{
  int i, l;
  Serial.println("Select a command:");
  for (i = 0; command_list[i].cmd_ch != NULL; i++) {
    Serial.print("  ");
    Serial.print(command_list[i].cmd_ch);
    l = 28 - strlen(command_list[i].cmd_ch);
    Serial.print(" ");
    while (l > 0) {
      Serial.print(" ");
      l--;
    }
    Serial.println(command_list[i].cmd_desc);
  }
}

// a is prototype, b is command
static
int match_command(const char *a, const char *b)
{
  int ai, bi;
  ai = bi = 0;
  while ( // skip non alphanum
    !((b[bi] >= 'a') && (b[bi] <= 'z')) &&
    !((b[bi] >= 'A') && (b[bi] <= 'Z')) &&
    !((b[bi] >= '0') && (b[bi] <= '9')))
  {
    bi++;
  }

  while (1) {
    if ((a[ai] == ' ') && (b[bi] == '\0'))
      break;
    if (a[ai] != b[bi])
      return 0;
    if (a[ai] == '\0')
      return 0;
    if (a[ai] == ' ')
      break;
    ai++;
    bi++;
  }
  return (bi+1);
}

void Readline_parse_command(String &cmd, Readline_cmd_list *command_list)
{
  int i;
  int offs;
  uint32_t st, en, an;
  uint32_t args[5]; // check max args

  for (i = 0; command_list[i].cmd_ch != NULL; i++) {
    offs = match_command(command_list[i].cmd_ch, cmd.c_str());
    if (offs > 0) {
      st = offs;
      en = st;
      an = 0;
      while ((st < cmd.length()) && (an < 5)) {
        while (st < cmd.length()) {
          if (cmd[st] != ' ') break;
          st++;
        }
        en = st;
        while (en < cmd.length()) {
          if (cmd[en] == ' ') break;
          en++;
        }
        if (en == st)
          break;
        args[an] = (uint32_t)cmd.substring(st,en+1).toInt();
        an++;
        st = en+1;
      }
//      Serial.print("Running ");
//      Serial.println(command_list[i].cmd_desc);
      command_list[i].cmd(args, an);
      return;
    }
  }
}



