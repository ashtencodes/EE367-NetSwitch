enum ValidEntry {
	NotValid,
	Valid
};

enum switch_job_type {
	JOB_CONTROL_SEND,
	JOB_CONTROL_RECV
};

struct switch_port_forwarding {
	enum ValidEntry valid;
	char dst;
	int port;
	int isChild;
	int isParent;
};

void switch_main(int switch_id);