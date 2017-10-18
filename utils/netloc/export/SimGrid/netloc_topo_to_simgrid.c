/*
 * Copyright © 2017      Inria.  All rights reserved.
 *
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 * See COPYING in top-level directory.
 *
 * $HEADER$
 */

#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <dirent.h>
#include <libgen.h>

#include <netloc.h>
#include <private/netloc.h>
#include <private/netloc-xml.h>
#include <private/utils/xml.h>

#define DTD_REF "http://simgrid.gforge.inria.fr/simgrid.dtd"
#define TOPOVERSION  "3"
#define AS_TAG       "AS"
#define AS_ID        "AS0"
#define AS_ROUTING   "Floyd"
#define HOST_PWR     "1"
#define LATENCY_LVL1 "0.314159us"
#define LATENCY_LVL2 "1.314159us"
#define LATENCY_LVL3 "2.314159us"

static int handle_edge(netloc_edge_t *edge, xml_node_ptr as,
                       xml_node_ptr routing)
{
    int ret = NETLOC_ERROR;
    char *bw = NULL, *id = NULL, *src = NULL, *dest = NULL;
    /* Create link tag */
    xml_node_ptr link_tag = xml_node_new(NULL, BAD_CAST "link");
    /* Set id */
    if (-1 < asprintf(&id, "%d", edge->id)) {
        xml_node_attr_cpy_add(link_tag, BAD_CAST "id", id);
    } else {
        id = NULL;
        xml_node_free(link_tag);
        goto ERROR;
    }
    /* Set bandwidth */
    if (-1 < asprintf(&bw, "%f", edge->total_gbits)) {
        xml_node_attr_cpy_add(link_tag, BAD_CAST "bandwidth", bw);
        free(bw);
    } else {
        xml_node_free(link_tag);
        goto ERROR;
    }
    /* Add link_tag to AS */
    xml_node_child_add(as, link_tag);
    /* Set latency */
    /** @FIXME: latency is just defined regarding its switch to
        switch, switch to node, or intra node. */
    if ((NETLOC_NODE_TYPE_SWITCH == edge->node->type
         && NETLOC_NODE_TYPE_SWITCH == edge->dest->type) ||
        (NETLOC_NODE_TYPE_SWITCH == edge->dest->type
         && NETLOC_NODE_TYPE_SWITCH == edge->node->type)) {
        xml_node_attr_add(link_tag, BAD_CAST "latency", BAD_CAST LATENCY_LVL3);
    } else if ((NETLOC_NODE_TYPE_HOST == edge->node->type
                && NETLOC_NODE_TYPE_SWITCH == edge->dest->type) ||
               (NETLOC_NODE_TYPE_SWITCH == edge->node->type
                && NETLOC_NODE_TYPE_HOST == edge->dest->type)) {
        xml_node_attr_add(link_tag, BAD_CAST "latency", BAD_CAST LATENCY_LVL2);
    } else {
        xml_node_attr_add(link_tag, BAD_CAST "latency", BAD_CAST LATENCY_LVL1);
    }
    /* Create route tag */
    xml_node_ptr route_tag;
    if (NULL == (route_tag = xml_node_new(NULL, BAD_CAST "route"))) {
        goto ERROR;
    }
    if (0 > asprintf(&src, "%s_%s", netloc_node_type_encode(edge->node->type),
                     edge->node->physical_id)) {
        src = NULL;
        xml_node_free(route_tag);
        goto ERROR;
    }
    if (0 > asprintf(&dest, "%s_%s", netloc_node_type_encode(edge->dest->type),
                     edge->dest->physical_id)) {
        dest = NULL;
        xml_node_free(route_tag);
        goto ERROR;
    }
    /* Set source */
    xml_node_attr_cpy_add(route_tag, BAD_CAST "src", src); free(src);
    /* Set destination */
    xml_node_attr_cpy_add(route_tag, BAD_CAST "dst", dest); free(dest);
    /* Create link_ctn tag */
    xml_node_ptr link_ctn_tag = xml_node_child_new(route_tag, NULL,
                                                   BAD_CAST "link_ctn", NULL);
    if (!link_ctn_tag) {
        xml_node_free(route_tag);
        goto ERROR;
    }
    /* Set id */
    xml_node_attr_cpy_add(link_ctn_tag, BAD_CAST "id", id);
    /* Add route_tag to routing tag buffer */
    xml_node_child_add(routing, route_tag);

    ret = NETLOC_SUCCESS;
 ERROR:
    free(id);
    return ret;
}

static int handle_node(netloc_node_t *node, xml_node_ptr as)
{
    int ret = NETLOC_ERROR;
    char *id = NULL;
    xml_node_ptr router_tag = xml_node_child_new(as, NULL,
                                                 BAD_CAST "router", NULL);
    if (-1 < asprintf(&id, "%s_%s",
                      netloc_node_type_encode(node->type),
                      node->physical_id)) {
        xml_node_attr_cpy_add(router_tag, BAD_CAST "id", id);
        free(id);
        ret = NETLOC_SUCCESS;
    }
    return ret;
}                 

static int write_topology(netloc_topology_t *topology, char *output_path)
{
    int ret = NETLOC_ERROR;

    XML_LIB_CHECK_VERSION;
    
    xml_doc_ptr doc = xml_doc_new(BAD_CAST "1.0");
    xml_dtd_subset_create(doc, BAD_CAST "platform", NULL, BAD_CAST DTD_REF);
    xml_node_ptr platform = xml_node_new(NULL, BAD_CAST "platform");
    xml_doc_set_root_element(doc, platform);
    /* Set version */
    xml_node_attr_add(platform, BAD_CAST "version", BAD_CAST TOPOVERSION);
    /* Add AS tag */
    xml_node_ptr as = xml_node_child_new(platform, NULL, BAD_CAST AS_TAG, NULL);
    xml_node_attr_add(as, BAD_CAST "id", BAD_CAST AS_ID);
    xml_node_attr_add(as, BAD_CAST "routing", BAD_CAST AS_ROUTING);

    /* Create a temporary tag to keep all route tags, so they can be
       added to AS at the end when every route/host/link tags are
       already inserted. <__tmp__> will be deleted in xml_çnode_merge
    */
    xml_node_ptr route_tag_buffer = xml_node_new(NULL, BAD_CAST "__tmp__");

    /* @TODO: iterate topology to add everything to the proper
       tags. */
    
    xml_node_merge(as, route_tag_buffer);

    if (-1 < xml_doc_write(output_path, doc, "UTF-8", 1))
        ret = NETLOC_SUCCESS;
    /* Free the document */
    xml_doc_free(doc);
    /*
     * Free the global variables that may
     * have been allocated by the parser.
     */
    xml_parser_cleanup();

    return ret;
}

static int netloc_topo_to_simgrid(netloc_topology_t *topology)
{
    int ret;
    char *node_uri = topology->topopath;
    int basename_len = strlen(node_uri)-10;
    char *basename = (char *)malloc(sizeof(char[basename_len + 1]));
    char *simgrid;

    netloc_topology_read_hwloc(topology, 0, NULL);

    strncpy(basename, node_uri, basename_len);
    basename[basename_len] = '\0';

    if (0 > asprintf(&simgrid, "%s-%s.xml", basename, "simgrid"))
        simgrid = NULL;

    ret = write_topology(topology, simgrid);
    if (NETLOC_SUCCESS != ret)
        fprintf(stderr, "Error: Unable to write to file \"%s\"\n", simgrid);

    free(basename); free(simgrid);
    return ret;
}

static char *read_param(int *argc, char ***argv)
{
    if (!*argc)
        return NULL;

    char *ret = **argv;
    (*argv)++;
    (*argc)--;

    return ret;
}

void help(char *name, FILE *f)
{
    fprintf(f, "Usage: %s <path to topology directory>\n", name);
}

int main(int argc, char **argv)
{
    char *prog_name = basename(argv[0]);

    if (argc != 2) {
        help(prog_name, stderr);
        return EXIT_FAILURE;
    }
    read_param(&argc, &argv);

    char *param;
    param = read_param(&argc, &argv);

    char *netlocpath;
    if (!strcmp(param, "--help")) {
        help(prog_name, stdout);
        return EXIT_SUCCESS;
    } else {
        netlocpath = param;
    }

    DIR *netlocdir = opendir(netlocpath);
    if (!netlocdir) {
        fprintf(stderr, "Error: Cannot open the directory <%s>.\n", netlocpath);
        return EXIT_FAILURE;
    }

    struct dirent *dir_entry = NULL;
    while ((dir_entry = readdir(netlocdir)) != NULL) {
        char *topopath;

#ifdef _DIRENT_HAVE_D_TYPE
        /* Skip directories if the filesystem returns a useful d_type.
         * Otherwise, continue and let the actual opening will fail later.
         */
        if( DT_DIR == dir_entry->d_type ) {
            continue;
        }
#endif

        /* Skip if does not end in .xml extension */
        if( NULL == strstr(dir_entry->d_name, "-nodes.xml") ) {
            continue;
        }

        if (0 > asprintf(&topopath, "%s/%s", netlocpath, dir_entry->d_name))
            topopath = NULL;

        netloc_topology_t *topology;
        int ret = netloc_topology_xml_load(topopath, &topology);
        if (NETLOC_SUCCESS != ret) {
            fprintf(stderr, "Error: netloc_topology_construct failed\n");
            return ret;
        }

        netloc_edge_reset_uid();

        netloc_topo_to_simgrid(topology);

        netloc_topology_destruct(topology);
    }
    closedir(netlocdir);

    return EXIT_SUCCESS;
}
