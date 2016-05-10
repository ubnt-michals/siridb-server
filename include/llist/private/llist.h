/*
 * (Private) llist.h - Linked List
 *
 * author       : Jeroen van der Heijden
 * email        : jeroen@transceptor.technology
 * copyright    : 2016, Transceptor Technology
 *
 * changes
 *  - initial version, 03-05-2016
 *
 */
#pragma once

typedef struct llist_node_s
{
    void * data;
    struct llist_node_s next;
} llist_node_t;