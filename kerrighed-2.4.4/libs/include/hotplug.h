#ifndef __LIB_HOTPLUG__
#define __LIB_HOTPLUG__

#include <string.h>

#define KRG_STATUS_LENGTH 10
enum krg_status {
	INVALID,
	POSSIBLE,
	PRESENT,
	ONLINE,
};

struct krg_nodes {
	char* nodes;
};
typedef struct krg_nodes* krg_nodes_t;

struct krg_clusters {
	char* clusters;
};
typedef struct krg_clusters *krg_clusters_t;

struct krg_node_set {
	int subclusterid;
	char* v;
};
typedef struct krg_node_set *krg_node_set_t;

/*
 * krg_status_str
 *
 * Return the name of the status
 */
char* krg_status_str(int s);

/*
 * krg_node_set_init
 *
 * Creates and initializes a krg_node_set.
 *
 * Returns NULL on memory allocation error
 */
krg_node_set_t krg_node_set_init(void);

/*
 * krg_node_set_destroy
 *
 * Destroy a krg_node_set
 */
void krg_node_set_destroy(krg_node_set_t node_set);

/*
 * krg_node_set_add
 *
 * Add node with id n to node_set.
 *
 * Returns 1 on success, -1 on failure
 */
int krg_node_set_add(krg_node_set_t node_set, int n);

/*
 * krg_node_set_remove
 *
 * Remove node with id n from node_set.
 *
 * Returns 1 on success, -1 on failure
 */
int krg_node_set_remove(krg_node_set_t node_set, int n);

/*
 * krg_node_set_contains
 *
 * Returns 1 if node is present in the set, 0 if not, -1 in case of failure
 */
int krg_node_set_contains(krg_node_set_t node_set, int n);

/*
 * Returns number of nodes in set
 */
int krg_node_set_weight(krg_node_set_t node_set);

/*
 * Returns next node in the set, -1 if no next
 */
int krg_node_set_next(krg_node_set_t node_set, int n);

/*
 * krg_nodes_init
 *
 * Creates and initializes a krg_nodes struct
 */
krg_nodes_t krg_nodes_init();

/*
 * krg_nodes_destroy
 *
 * Destroy a krg_nodes
 */
void krg_nodes_destroy(krg_nodes_t nodes);

/*
 * krg_nodes_num
 *
 * Return number of nodes in the given status, -1 in case of failure
 */
int krg_nodes_num(krg_nodes_t nodes, enum krg_status s);
int krg_nodes_num_online(krg_nodes_t nodes);
int krg_nodes_num_possible(krg_nodes_t nodes);
int krg_nodes_num_present(krg_nodes_t nodes);

/*
 * krg_nodes_is
 *
 * Return 1 if node is in the given status, 0 otherwise, -1 in case of failure
 */
int krg_nodes_is(krg_nodes_t nodes, int node, enum krg_status s);
int krg_nodes_is_online(krg_nodes_t nodes, int node);
int krg_nodes_is_possible(krg_nodes_t nodes, int node);
int krg_nodes_is_present(krg_nodes_t nodes, int node);

/*
 * krg_nodes_next
 *
 * Return next node in the given state, -1 in case of failure or no next
 */
int krg_nodes_next(krg_nodes_t nodes, int node, enum krg_status s);
int krg_nodes_next_online(krg_nodes_t nodes, int node);
int krg_nodes_next_possible(krg_nodes_t nodes, int node);
int krg_nodes_next_present(krg_nodes_t nodes, int node);

/*
 * krg_nodes_get
 *
 * Return set of nodes in given state, NULL in case of failure
 */
struct krg_node_set* krg_nodes_get(krg_nodes_t nodes, enum krg_status s);
struct krg_node_set* krg_nodes_get_online(krg_nodes_t nodes);
struct krg_node_set* krg_nodes_get_possible(krg_nodes_t nodes);
struct krg_node_set* krg_nodes_get_present(krg_nodes_t nodes);

/*
 * krg_nodes_getnode
 *
 * Return status of given node, -1 in case of failure
 */
int krg_nodes_getnode(krg_nodes_t nodes, int node);

/*
 * krg_nodes_nextnode
 *
 * Return next node which is not invalid, -1 in case of failure or no next
 */
int krg_nodes_nextnode(krg_nodes_t nodes, int node);

/*
 * krg_clusters_init
 *
 * Creates and initializes a krg_clusters struct
 */
krg_clusters_t krg_clusters_init();

/*
 * krg_clusters_destroy
 *
 * Destroy a krg_clusters
 */
void krg_clusters_destroy(krg_clusters_t clusters);

/*
 * krg_clusters_is_up
 *
 * Return 1 if cluster is up, 0 otherwise, -1 in case of failure
 */
int krg_clusters_is_up(krg_clusters_t nodes, int cluster);

int krg_get_max_nodes(void);
int krg_get_max_clusters(void);
int krg_nodes_add(struct krg_node_set *node_set);
int krg_nodes_remove(struct krg_node_set *node_set);
int krg_nodes_fail(struct krg_node_set *node_set);
int krg_nodes_poweroff(struct krg_node_set *node_set);
struct krg_nodes* krg_nodes_status(void);
struct krg_clusters* krg_cluster_status(void);
int krg_cluster_start(struct krg_node_set *krg_node_set);
int krg_cluster_start_all();
int krg_cluster_wait_for_start(void);
int krg_cluster_shutdown(int subclusterid);
int krg_cluster_reboot(int subclusterid);

int krg_get_errno();

#endif
