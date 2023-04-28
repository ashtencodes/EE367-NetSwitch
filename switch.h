enum ValidEntry {
	NotValid,
	Valid
};

enum switch_job_type {
	JOB_CONTROL_SEND,
	JOB_CONTROL_RECV
};

enum localPortTreeValue
{
	NONE,
	HOST_EDGE,
	BAD_EDGE,
	PARENT,
	CHILD
};

struct switch_port_forwarding {
	enum ValidEntry valid;
	char dst;
	int port;
	enum localPortTreeValue type;
};

struct switch_job {
	enum switch_job_type type;
	struct packet *packet;
	int in_port_index;
	int out_port_index;
	char fname_download[100];
	char fname_upload[100];
	int ping_timer;
	int file_upload_dst;
   	int file_download_dst;
	struct switch_job *next;
};

struct switch_job_queue {
	struct switch_job *head;
	struct switch_job *tail;
	int occ;
};

void switch_main(int switch_id);