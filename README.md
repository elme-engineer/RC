# RC PROJECT 2023/2024

## COMPLETED FUNCTIONS ✅

- `void error();`
- `void read_config_file()`
- `void save_config_file()`
- `int validate_user()`
- `void add_user()`
- `void send_msg_udp()`
- `free_node()`
- `void free_list()`
- `pUser get_user()`
- `int del_user(char *username);`


## INCOMPLET FUNCTIONS ❌

- `void *udp()`
- `void *tcp()`
- `void process_client()`


## OTHER STUFF TO DO ❕

- Handle client request with the server (tcp)
- Finish admin actions (udp)

    #### Student And Professor Actions

    - List classes: `LIST_CLASSES`
    - List subscribed classes `LIST_SUBSCRIBED`
    - Subscribe class: `SUBSCRIBE_CLASS <nome>` (MULTICAST)

    #### Professor ONLY Actions

    - Create class: `CREATE_CLASS <name> <size>`
    - Send texts to other clients: `SEND {name} {text that server will send to subscribers}` (MULTICAST)