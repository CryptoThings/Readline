
#ifndef __READLINE_H__
#define __READLINE_H__

String Readline(const char *prompt, Stream *stream = NULL);

size_t Readline_read_buf(uint8_t *data, size_t len, bool to_bin = false);

size_t Readline_read_str_data(char *buf, size_t len);
void Readline_read_str_data(char **data, char *buf, size_t len, size_t *idx);

typedef struct _cmd_list {
  const char *cmd_ch;
  const char *cmd_desc;
  void (*cmd)(uint32_t *args, uint32_t num_args);
} Readline_cmd_list;

extern Readline_cmd_list command_list[];

void Readline_print_command_list(Readline_cmd_list *command_list);
void Readline_parse_command(String &cmd, Readline_cmd_list *command_list);

extern void (*Readline_idle)();

#endif
