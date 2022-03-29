/* Cluster configuration related interface functions.
 * @file libhotplug.c
 *
 * Copyright (C) 2006-2007, Kerlabs
 *
 * Authors:
 *    Pascal Gallard, Kerlabs.
 *    Jean Parpaillon, <jean.parpaillon@kerlabs.com>
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>

#include <kerrighed/types.h>
#include <kerrighed/krgnodemask.h>
#include <kerrighed_tools.h>
#include <hotplug.h>

static int kerrighed_max_nodes = -1;
static int kerrighed_max_clusters = -1;

char* krg_status_str(int s)
{
	char* str = malloc(sizeof(char)*KRG_STATUS_LENGTH);

	switch (s) {
	case POSSIBLE:
		snprintf(str, KRG_STATUS_LENGTH, "possible");
		break;
	case PRESENT:
		snprintf(str, KRG_STATUS_LENGTH, "present");
		break;
	case ONLINE:
		snprintf(str, KRG_STATUS_LENGTH, "online");
		break;
	default:
		snprintf(str, KRG_STATUS_LENGTH, "invalid");
	}
	
	return str;
}

krg_nodes_t krg_nodes_init()
{
	krg_nodes_t item;

	item = malloc(sizeof(struct krg_nodes));
	if (!item)
		return NULL;

	item->nodes = malloc(sizeof(char)*krg_get_max_nodes());
	if (!item->nodes)
		return NULL;

	return item;
}

void krg_nodes_destroy(krg_nodes_t item)
{
	free(item->nodes);
	free(item);
}

int krg_nodes_num(krg_nodes_t nodes, enum krg_status s)
{
	int ret = 0;
	int bcl;

	for (bcl=0; bcl<krg_get_max_nodes(); bcl++) {
		if (nodes->nodes[bcl] == (char)s)
			ret++;
	}

	return ret;
}

int krg_nodes_num_possible(krg_nodes_t nodes)
{
	return krg_nodes_num(nodes, POSSIBLE);
}

int krg_nodes_num_present(krg_nodes_t nodes)
{
	return krg_nodes_num(nodes, PRESENT);
}

int krg_nodes_num_online(krg_nodes_t nodes)
{
	return krg_nodes_num(nodes, ONLINE);
}

int krg_nodes_is(krg_nodes_t nodes, int n, enum krg_status s)
{
	int ret = 0;

	if (n>=0 && n<krg_get_max_nodes()) {
		if (nodes->nodes[n] == (char)s)
			ret = 1;
	} else {
		ret = -1;
	}
	
	return ret;
}

int krg_nodes_is_possible(krg_nodes_t nodes, int n)
{
	return krg_nodes_is(nodes, n, POSSIBLE);
}

int krg_nodes_is_present(krg_nodes_t nodes, int n)
{
	return krg_nodes_is(nodes, n, PRESENT);
}

int krg_nodes_is_online(krg_nodes_t nodes, int n)
{
	return krg_nodes_is(nodes, n, ONLINE);
}

int krg_nodes_next(krg_nodes_t nodes, int node, enum krg_status s)
{
	int bcl;

	bcl = node+1;
	while (bcl < krg_get_max_nodes()) {
		if (nodes->nodes[bcl] == (char)s)
			return bcl;
		bcl++;
	}
	return -1;
}

int krg_nodes_next_online(krg_nodes_t nodes, int node)
{
	return krg_nodes_next(nodes, node, ONLINE);
}

int krg_nodes_next_possible(krg_nodes_t nodes, int node)
{
	return krg_nodes_next(nodes, node, POSSIBLE);
}

int krg_nodes_next_present(krg_nodes_t nodes, int node)
{
	return krg_nodes_next(nodes, node, PRESENT);
}

struct krg_node_set* krg_nodes_get(krg_nodes_t nodes, enum krg_status s)
{
	struct krg_node_set* r = NULL;
	int bcl;

	r = krg_node_set_init();
	if (r) {
		for (bcl = 0; bcl < krg_get_max_nodes(); bcl++) {
			if (nodes->nodes[bcl] == (char)s)
				krg_node_set_add(r, bcl);
		}
	}

	return r;
}

struct krg_node_set* krg_nodes_get_online(krg_nodes_t nodes)
{
	return krg_nodes_get(nodes, ONLINE);
}

struct krg_node_set* krg_nodes_get_possible(krg_nodes_t nodes)
{
	return krg_nodes_get(nodes, POSSIBLE);
}

struct krg_node_set* krg_nodes_get_present(krg_nodes_t nodes)
{
	return krg_nodes_get(nodes, PRESENT);
}

int krg_nodes_getnode(krg_nodes_t nodes, int n)
{
	if (n >= 0 && n < krg_get_max_nodes()) {
		return nodes->nodes[n];
	}else
		return -1;
}

int krg_nodes_nextnode(krg_nodes_t nodes, int node)
{
	int bcl;

	bcl = node+1;
	while (bcl < krg_get_max_nodes()) {
		if (nodes->nodes[bcl] > (char)INVALID)
			return bcl;
		bcl++;
	}
	return -1;
}

krg_clusters_t krg_clusters_init()
{
	krg_clusters_t item;

	item = malloc(sizeof(struct krg_clusters));
	if (!item)
		return NULL;

	item->clusters = malloc(sizeof(char)*krg_get_max_clusters());
	if (!item->clusters)
		return NULL;

	return item;
}

void krg_clusters_destroy(krg_clusters_t item)
{
	free(item->clusters);
	free(item);
}

int krg_clusters_is_up(krg_clusters_t item, int n)
{
	if (n >= 0 && n < krg_get_max_clusters()) {
		if (item->clusters[n])
			return 1;
		else
			return 0;
	}else
		return -1;
}

krg_node_set_t krg_node_set_init()
{
	krg_node_set_t item;
	int bcl, max_nodes;

	max_nodes = krg_get_max_nodes();

	item = malloc(sizeof(struct krg_node_set));
	if (!item)
		return NULL;

	item->subclusterid = 0;
	item->v = malloc(sizeof(char)*max_nodes);
	if (!item->v)
		return NULL;

	for (bcl = 0; bcl < max_nodes; bcl++)
		item->v[bcl] = 0;

	return item;
}

void krg_node_set_destroy(krg_node_set_t item)
{
	free(item->v);
	free(item);
}

int krg_node_set_add(krg_node_set_t item, int n)
{
	if (n >= 0 && n < krg_get_max_nodes()) {
		item->v[n] = 1;
		return 1;
	}else
		return -1;
}

int krg_node_set_remove(krg_node_set_t item, int n)
{
	if (n >= 0 && n < krg_get_max_nodes()) {
		item->v[n] = 0;
		return 1;
	}else
		return -1;
}

int krg_node_set_contains(krg_node_set_t node_set, int n)
{
	if (n >= 0 && n < krg_get_max_nodes())
		return node_set->v[n];
	else
		return -1;
}

int krg_node_set_weight(krg_node_set_t node_set)
{
	int r, bcl;

	r = 0;
	for (bcl = 0; bcl < krg_get_max_nodes(); bcl++)
		if (node_set->v[bcl])
			r++;

	return r;
}

int krg_node_set_next(krg_node_set_t node_set, int node)
{
	int bcl;

	bcl = node+1;
	while (bcl < krg_get_max_nodes()) {
		if (node_set->v[bcl])
			return bcl;
		bcl++;
	}
	return -1;
}

int krg_get_max_nodes(void)
{
	int r;

	if(kerrighed_max_nodes == -1){
		r = call_kerrighed_services(KSYS_NB_MAX_NODES, &kerrighed_max_nodes);
		if(r) return -1;
	}

	return kerrighed_max_nodes;
}

int krg_get_max_clusters(void)
{
	int r;

	if(kerrighed_max_clusters == -1){
		r = call_kerrighed_services(KSYS_NB_MAX_CLUSTERS, &kerrighed_max_clusters);
		if(r) return -1;
	}

	return kerrighed_max_clusters;
}

int krg_nodes_add(struct krg_node_set *krg_node_set)
{
	struct hotplug_node_set node_set;
	int i, r;

	krgnodes_clear(node_set.v);
	node_set.subclusterid = krg_node_set->subclusterid;
	
	for (i = 0; i < krg_get_max_nodes(); i++) {
		if(krg_node_set->v[i]){
			krgnode_set(i, node_set.v);
		}
	}

	r = call_kerrighed_services(KSYS_HOTPLUG_ADD, &node_set);
	if (r) return -1;
	
	return 0;
};

int krg_nodes_remove(struct krg_node_set *krg_node_set)
{
	struct hotplug_node_set node_set;
	int i, r;

	krgnodes_clear(node_set.v);
	node_set.subclusterid = krg_node_set->subclusterid;
	
	for(i = 0; i < krg_get_max_nodes(); i++) {
		if(krg_node_set->v[i]){
			printf("krg_nodes_remove: %d\n", i);
			krgnode_set(i, node_set.v);
		}
	}

	r = call_kerrighed_services(KSYS_HOTPLUG_REMOVE, &node_set);
	if (r) return -1;

	return 0;
}

int krg_nodes_fail(struct krg_node_set *krg_node_set){
	struct hotplug_node_set node_set;
	int i, r;

	krgnodes_clear(node_set.v);
	
	for (i = 0; i < krg_get_max_nodes(); i++) {
		if (krg_node_set->v[i]){
			krgnode_set(i, node_set.v);
		}
	}

	r = call_kerrighed_services(KSYS_HOTPLUG_FAIL, &node_set);
	if (r) return -1;

	return 0;
}

int krg_nodes_poweroff(struct krg_node_set *krg_node_set)
{
	struct hotplug_node_set node_set;
	int i, r;

	krgnodes_clear(node_set.v);
	
	for (i = 0; i < krg_get_max_nodes(); i++) {
		if (krg_node_set->v[i]) {
			krgnode_set(i, node_set.v);
		}
	}

	r = call_kerrighed_services(KSYS_HOTPLUG_POWEROFF, &node_set);
	if (r) return -1;

	return 0;
}

struct krg_nodes*  krg_nodes_status(void)
{
	struct krg_nodes *krg_nodes;
	struct hotplug_nodes hotplug_nodes;
	int r;

	krg_nodes = krg_nodes_init();
	if (!krg_nodes)
		return NULL;
	
	hotplug_nodes.nodes = krg_nodes->nodes;
	
	r = call_kerrighed_services(KSYS_HOTPLUG_NODES, &hotplug_nodes);
	
	if (r) {
		krg_nodes_destroy(krg_nodes);
		return  NULL;
	}

	return krg_nodes;
}

struct krg_clusters* krg_cluster_status(void)
{
	struct krg_clusters *krg_clusters;
	struct hotplug_clusters hotplug_clusters;
	int r;

	krg_clusters = krg_clusters_init();
	if (!krg_clusters)
		return NULL;
	
	r = call_kerrighed_services(KSYS_HOTPLUG_STATUS, &hotplug_clusters);

	if (r) {
		krg_clusters_destroy(krg_clusters);
		return NULL;
	}

	memcpy(krg_clusters->clusters, hotplug_clusters.clusters, krg_get_max_clusters());
	
	return krg_clusters;
}

int krg_cluster_start(struct krg_node_set *krg_node_set)
{
	struct hotplug_node_set node_set;
	struct krg_clusters* clusters;
	int i;

	clusters = krg_cluster_status();
	if (krg_clusters_is_up(clusters, 0)) {
		errno = EALREADY;
		return -1;
	}

	node_set.subclusterid = krg_node_set->subclusterid;

	krgnodes_clear(node_set.v);
	
	for (i = 0; i < krg_get_max_nodes(); i++) {
		if (krg_node_set->v[i]) {
			krgnode_set(i, node_set.v);
		}
	}
	
	return call_kerrighed_services(KSYS_HOTPLUG_START, &node_set);
}

int krg_cluster_start_all()
{
	struct krg_nodes* status;
	int r;

	status = krg_nodes_status();
	if(!status){
		return -1;
	}

	r = krg_cluster_start(krg_nodes_get_present(status));

	krg_nodes_destroy(status);

	return r;
}

int krg_cluster_wait_for_start(void)
{
	return call_kerrighed_services(KSYS_HOTPLUG_WAIT_FOR_START, NULL);
}

int krg_cluster_shutdown(int subclusterid)
{
	return call_kerrighed_services(KSYS_HOTPLUG_SHUTDOWN, &subclusterid);
}

int krg_cluster_reboot(int subclusterid)
{
	return call_kerrighed_services(KSYS_HOTPLUG_RESTART, &subclusterid);
}

int krg_get_errno()
{
	return errno;
}
