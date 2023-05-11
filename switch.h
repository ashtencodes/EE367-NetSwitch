enum ValidEntry {
	NotValid,
	Valid
};


enum localPortTreeValue
{
	NO,
	YES
};

struct switch_port_forwarding {
	enum ValidEntry valid;
	char dst;
	int port;
	enum localPortTreeValue localPortTree;
};


struct switch_job_queue {
	struct switch_job *head;
	struct switch_job *tail;
	int occ;
};

void switch_main(int switch_id);