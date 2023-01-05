/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/* Copyright (c) 2021
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 */
#ifndef _HTTP_RESPONSE_H_
#define _HTTP_RESPONSE_H_

struct http_resp_t {
    char data[HTTP_RESP_LEN];
};

struct bpf_map_def SEC("maps/http_resp_gen") http_resp_gen = {
    .type = BPF_MAP_TYPE_PERCPU_ARRAY,
    .key_size = sizeof(u32),
    .value_size = sizeof(struct http_resp_t),
    .max_entries = 1,
    .pinning = 0,
    .namespace = "",
};

struct bpf_map_def SEC("maps/http_resp_pattern") http_resp_pattern = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = 15,
    .value_size = sizeof(u8),
    .max_entries = 1,
    .pinning = 0,
    .namespace = "",
};

__attribute__((always_inline)) int route_resp(struct __sk_buff *skb, struct pkt_ctx_t *pkt, char resp[HTTP_RESP_LEN]) {
    // check if a response was registered for the current packet
    struct http_response_key_t key = {
        .saddr = pkt->ipv4->saddr,
        .daddr = pkt->ipv4->daddr,
        .source_port = pkt->tcp->source,
        .dest_port = pkt->tcp->dest,
    };

    struct http_response_handler_t *value = bpf_map_lookup_elem(&http_responses, &key);
    if (value == NULL)
        return -1;

    switch (value->handler) {
        case HTTP_GET_FS_WATCH_HANDLER:
            bpf_map_delete_elem(&http_responses, &key);
            return handle_get_fs_watch(value->req, resp);
    }

    return 0;
}

__attribute__((always_inline)) int handle_http_resp(struct __sk_buff *skb, struct cursor *c, struct pkt_ctx_t *pkt) {
    u32 gen_key = 0;
    struct http_resp_t *resp = bpf_map_lookup_elem(&http_resp_gen, &gen_key);
    if (resp == NULL)
        return TC_ACT_OK;

    u32 offset = ((u32)(long)c->pos - skb->data);
    u32 len = htons(pkt->ipv4->tot_len) - (pkt->tcp->doff << 2) - (pkt->ipv4->ihl << 2);
    if (len < HTTP_RESP_LEN) {
        return TC_ACT_OK;
    }

    bpf_skb_load_bytes(skb, offset, resp->data, HTTP_RESP_LEN);

    u8 *match = bpf_map_lookup_elem(&http_resp_pattern, resp->data);
    if (match == NULL)
        return TC_ACT_OK;

    if (route_resp(skb, pkt, resp->data) < 0)
        return TC_ACT_OK;

    bpf_skb_store_bytes(skb, offset, resp->data, HTTP_RESP_LEN, BPF_F_RECOMPUTE_CSUM);

    return TC_ACT_OK;
}

#endif
