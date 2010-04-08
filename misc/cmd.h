#define CMD_FN(name) int cmd_ ## name(char *cmd)

typedef struct {
	const char *name;
	int (*cmd)(char*);
} CMD_CALL;

int run_command(char *cmd);
int cmd_key_set(char *key, int state);

CMD_FN(send_file);
CMD_FN(activate_slot);
CMD_FN(run);
CMD_FN(step);
CMD_FN(screen_bw);
CMD_FN(screen_gs);
CMD_FN(key_down);
CMD_FN(key_up);
CMD_FN(reset_calc);
CMD_FN(dump_state);
CMD_FN(set_code_bp);
CMD_FN(remove_code_bp);
