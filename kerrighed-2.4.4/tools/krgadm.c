 /*
 *  Copyright (C) 2006-2007, Pascal Gallard, Kerlabs.
 *  Copyright (c) 2008 Jean Parpaillon <jean.parpaillon@kerlabs.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>

#include <kerrighed.h>

enum {
	NONE,
	STATUS,
	START,
	WAIT_START,
	POWEROFF,
	REBOOT,
};

#define NODE_SEP ','
#define NODE_RANGE_SEP '-'
#define POLL_NODES 1
#define NODES_OPTION "nodes", required_argument, NULL, 'n'
#define AUTO_OPTION "auto", required_argument,NULL, 'a'

static struct option start_options[] = {
  {NODES_OPTION},
  {AUTO_OPTION},
  {NULL, 0, NULL, 0}
};

void help(char * program_name)
{
	printf("\
Usage: %s -h,--help\n\
  or:  %s cluster {status|wait_start|poweroff|reboot}\n\
  or:  %s cluster start  [-n|--nodes] [-a|--auto]\n\
  or:  %s nodes\n",
	       program_name, program_name, program_name, program_name);
	printf("\n\
Mode:\n\
  cluster           clusters management\n\
  nodes             print nodes status\n\
\n\
Cluster start options:\n\
  -n, --nodes       list of nodes to wait for before starting\n\
                    ie: 2,3,6-10,42-45\n\
  -a, --auto        number of nodes to wait for before starting\n\
\n\
Node Status:\n\
  present           available for integrating the cluster\n\
  online            participating in the cluster\n\
\n");
}

int parse_node(char *ch, struct krg_node_set *node_set)
{
	int node;
	char* endptr;
	int r = 1;

	node = strtol(ch, &endptr, 10);
	if (endptr == ch || *endptr) {
		r = -1;
		errno = EINVAL;
		perror("node");
	} else
		krg_node_set_add(node_set, node);

	return r;
}

int parse_nodes_interval(char *ch, struct krg_node_set *node_set)
{
	int node1, node2, node;
	char* index;
	char* endptr;
	int r = 1;

	index = strchr(ch, NODE_RANGE_SEP);
	node1 = strtol(ch, &endptr, 10);
	if (endptr == ch || NODE_RANGE_SEP != *endptr) {
		r = -1;
		errno = EINVAL;
		perror("first node range");
		goto exit;
	}
	node2 = strtol(index+1, &endptr, 10);
	if (endptr == index+1 || *endptr) {
		r = -1;
		errno = EINVAL;
		perror("end node range");
		goto exit;
	}

	for(node=node1; node<=node2; node++)
		krg_node_set_add(node_set, node);

 exit:
	return r;
}

int parse_nodes(char *ch, struct krg_node_set *node_set)
{
	char *ch_item;
	int r = 1;

	ch_item = strrchr(ch, NODE_SEP);

	if(ch_item){
		*ch_item = 0;
		if ((r=parse_nodes(ch, node_set)) == -1)
			goto exit;
		if ((r=parse_nodes(ch_item+1, node_set)) == -1)
			goto exit;
		goto exit;
	}

	ch_item = strrchr(ch, NODE_RANGE_SEP);
	if(ch_item) {
		if ((r=parse_nodes_interval(ch, node_set)) == -1)
			goto exit;
	} else
		if ((r=parse_node(ch, node_set)) == -1)
			goto exit;

 exit:
	return r;
}

/*
 * Return number of nodes in the cluster if up, 0 if not, -1 on failure
 */
int cluster_status()
{
	struct krg_clusters* cluster_status;
	int i = 0;

	cluster_status = krg_cluster_status();
	if(!cluster_status){
		i = -1;
		goto exit;
	}

	if (krg_clusters_is_up(cluster_status, 0)){
		i = krg_nodes_num_online(krg_nodes_status());
	}

 exit:
	krg_clusters_destroy(cluster_status);
	return i;
}

/*
 * When returning on success, node_set contains nodes to start.
 *
 * Return 1 when i nodes are present, -1 on failure.
 */
int wait_for_nodes_count(int i, struct krg_node_set* node_set)
{
	struct krg_nodes* status;
	int cur, r = 1;
	int nodes_count;

	printf("Waiting for %d nodes to join... ", i);
	fflush(stdout);

	if (cluster_status() > 0) {
		r = -1;
		errno = EALREADY;
		goto exit;
	}

	if (i < 1 || i > krg_get_max_nodes()) {
		r = -1;
		errno = ERANGE;
		goto exit;
	}

	do {
		status = krg_nodes_status();
		if (!status) {
			r = -1;
			goto exit;
		}

		nodes_count = krg_nodes_num_present(status);
		printf("%4d/%-4d", nodes_count, i);
		fflush(stdout);

		if (nodes_count < i) {
			sleep(POLL_NODES);
			printf("\b\b\b\b\b\b\b\b\b");
			krg_nodes_destroy(status);
		}

		fflush(stdout);
	} while (nodes_count < i);

	nodes_count = 0;
	cur = -1;
	do {
		cur = krg_nodes_next_present(status, cur);
		krg_node_set_add(node_set, cur);
		nodes_count++;
	} while (nodes_count < i);

	krg_nodes_destroy(status);

 exit:
	if (r==1)
		printf(" done\n");
	else
		printf(" fail (%s)\n", strerror(errno));

	return r;
}

/*
 * Return 1 when nodes in node_set are present, -1 on failure.
 */
int wait_for_nodes(struct krg_node_set* node_set)
{
	struct krg_nodes* status;
	int bcl, r = 1, done, node_count;

	printf("Waiting for nodes to join... ");
	fflush(stdout);

	if (cluster_status() > 0) {
		r = -1;
		errno = EALREADY;
		goto exit;
	}

	node_count = krg_node_set_weight(node_set);
	do {
		done = 1;
		status = krg_nodes_status();
		if (!status) {
			r = -1;
			goto exit;
		}
		for (bcl = 0; bcl < krg_get_max_nodes(); bcl++) {
			if (krg_node_set_contains(node_set, bcl)) {
				printf("%4d:", bcl);
				if (krg_nodes_is_present(status, bcl))
					printf("1");
				else {
					done = 0;
					printf("0");
				}
			}
			fflush(stdout);
		}
		krg_nodes_destroy(status);

		if (!done) {
			sleep(POLL_NODES);

			for (bcl = 0; bcl < node_count; bcl++)
				printf("\b\b\b\b\b\b");
			fflush(stdout);
		}

	} while (!done);

 exit:
	if (r==1)
		printf(" done\n");
	else
		printf(" fail (%s)\n", strerror(errno));

	return r;
}

/*
 * Return 0 on success, -1 on failure
 */
int cluster_start(int argc, char* argv[], char* program_name)
{
	struct krg_node_set* node_set;
	int c, option_index;
	int r = 0;
	int wait = -1;
	char* nodes;
	char* endptr;

	node_set = krg_node_set_init();

	while ((c=getopt_long(argc, argv, "n:a:",
												start_options, &option_index)) != -1) {
		switch (c) {
		case 'n':
			wait = 0;
			nodes = malloc(sizeof(char)*(strlen(optarg)+1));
			nodes = strncpy(nodes, optarg, strlen(optarg)+1);
			if ((r=parse_nodes(optarg, node_set)) == -1)
				goto exit;
			break;
		case 'a':
			wait = strtol(optarg, &endptr, 10);
			if (endptr == optarg || *endptr) {
				r = -1;
				errno = EINVAL;
				perror("nodes number");
				goto exit;
			}
			break;
		default:
			help(program_name);
			goto exit;
		}
	}

	switch (wait) {
	case -1:
		{
			struct krg_nodes* status;

			status = krg_nodes_status();
			node_set = krg_nodes_get_present(status);
			krg_nodes_destroy(status);
		}
		break;
	case 0:
		r = wait_for_nodes(node_set);
		if (r == -1)
			goto exit;
		break;
	default:
		r = wait_for_nodes_count(wait, node_set);
		if (r == -1) {
			goto exit;
		}
	}

	printf("Starting cluster... ");
	fflush(stdout);
	r = krg_cluster_start(node_set);
	if (r == -1)
		printf("fail (%s)\n", strerror(errno));
	else
		printf("done\n");

 exit:
	return r;
}

/*
 * Return EXIT_SUCCESS on success, EXIT_FAILURE on failure.
 */
int cluster(int argc, char* argv[], char* program_name)
{
	int action = NONE;
	int ret = EXIT_SUCCESS;
	int r;

	if(argc==0 || !strcmp(*argv, "status"))
		action = STATUS;
	else if(!strcmp(*argv, "start"))
		action = START;
	else if(!strcmp(*argv, "wait_start"))
		action = WAIT_START;
	else if(!strcmp(*argv, "poweroff"))
		action = POWEROFF;
	else if(!strcmp(*argv, "reboot"))
		action = REBOOT;

	switch (action) {
	case STATUS:
		printf("status: ");
		r = cluster_status();
			switch(r){
			case -1:
				printf("error\n");
				ret = EXIT_FAILURE;
				break;
			case 0:
				printf("down\n");
				break;
			default:
				printf("up on %d nodes\n", r);
			}
			break;
	case START:
		r = cluster_start(argc, argv, program_name);
		if (r == -1)
			ret = EXIT_FAILURE;
		break;
	case WAIT_START:
		printf("Waiting for cluster to start... ");
		fflush(stdout);
		r = krg_cluster_wait_for_start();
		if (r == -1) {
			printf("fail (%s)\n", strerror(errno));
			ret = EXIT_FAILURE;
		} else
			printf("done\n");
		break;
	case POWEROFF:
		printf("Shutting down cluster... ");
		fflush(stdout);
		if (cluster_status()==0) {
			printf("fail (cluster not running)\n");
			ret = EXIT_FAILURE;
		} else {
			r = krg_cluster_shutdown(0);
			if (r == -1) {
				printf("fail (%s)\n", strerror(errno));
				ret = EXIT_FAILURE;
			} else
				printf("done\n");
		}
		break;
	case REBOOT:
		printf("Rebooting cluster... ");
		fflush(stdout);
		if (cluster_status() == 0) {
			printf("fail (cluster not running)\n");
			ret = EXIT_FAILURE;
		} else {
			r = krg_cluster_reboot(0);
			if (r == -1) {
				printf("fail (%s)\n", strerror(errno));
				ret = EXIT_FAILURE;
			} else
				printf("done\n");
		}
	break;
	default:
		help(program_name);
	}

	return ret;
}

/*
 * Return EXIT_SUCCESS on success, EXIT_FAILURE on error
 */
int nodes(int argc, char* argv[], char* program_name)
{
	int ret = EXIT_SUCCESS;

	if(argc==0 || !strcmp(*argv, "status")){
		struct krg_nodes* status;
		int bcl;

		status = krg_nodes_status();

		if(status){
			for(bcl=0;bcl<krg_get_max_nodes();bcl++)
				if(status->nodes[bcl]!=INVALID)
					printf("%d:%s\n", bcl, krg_status_str(status->nodes[bcl]));
		}else{
			printf("Are you sure to run Kerrighed ?\n");
			ret = EXIT_FAILURE;
		}
	}else
		help(program_name);

	return ret;
}

int main(int argc, char* argv[])
{
	char **arg;
	char* program_name;
	int count;
	int ret = EXIT_SUCCESS;

	arg = argv;
	count = argc;
	program_name = argv[0];

	if(argc==1){
		help(program_name);
		exit(EXIT_SUCCESS);
	}

	opterr = 0;

	count--;
	arg++;

	if (!strcmp(argv[1], "cluster"))
		ret = cluster(count-1, arg+1, program_name);
	else if (!strcmp(argv[1], "nodes"))
		ret = nodes(count-1, arg+1, program_name);
	else
		help(program_name);

	return ret;
}
