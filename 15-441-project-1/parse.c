#include "parse.h"

extern void set_parsing_options(char *buf, size_t siz, Request *parsing_request);
extern int yyparse();
/**
* Given a char buffer returns the parsed request headers
*/
Request * parse(char *buffer, int size, int socketFd) {
    //Differant states in the state machine
    printf("size=%d\n", size);
	enum {
		STATE_START = 0, STATE_CR, STATE_CRLF, STATE_CRLFCR, STATE_CRLFCRLF
	};

	int i = 0, state;
	size_t offset = 0;
	char ch;
	char buf[8192];
	memset(buf, 0, 8192);

	state = STATE_START;
	while (state != STATE_CRLFCRLF) {
		char expected = 0;

		if (i == size)
			break;

		ch = buffer[i++];
		buf[offset++] = ch;

		switch (state) {
		case STATE_START:
		case STATE_CRLF:
		    // 回车符号
			expected = '\r';
			break;
		case STATE_CR:
		case STATE_CRLFCR:
		    // 换行符
			expected = '\n';
			break;
		default:
			state = STATE_START;
			continue;
		}

		if (ch == expected)
			state++;
		else
			state = STATE_START;

	}
	printf("state = %d\n", state);
	printf("i = %d\n", i);
    //Valid End State
    Request *request = NULL;
	 if (state == STATE_CRLFCRLF) {
	    printf("Start to parse buffer\n");
		request = (Request *) malloc(sizeof(Request));
		request->header_count=0;
        // set the max header number is 10
        request->headers = (Request_header *) malloc(sizeof(Request_header)*10);
		// set the max request body is 10KB
        request->body = (Request_body *)malloc(sizeof(Request_body)*1024*10);
        offset = 0;
        int j = i;
        while(j < size) {
            ch = buffer[j++];
            request->body[offset++] = ch;
        }
        // TODO Bus error 10
        set_parsing_options(buf, i, request);
		if (yyparse() == SUCCESS) {
            return request;
		}
	}
    printf("Parsing Failed\n");
    return request;
}
