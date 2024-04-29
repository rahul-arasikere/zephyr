/*
 * Copyright (c) 2017 PHYTEC Messtechnik GmbH
 * Copyright (c) 2017, 2018 Intel Corporation
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/drivers/hwinfo.h>

#include "usbd_desc.h"
#include "usbd_device.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(usbd_desc, CONFIG_USBD_LOG_LEVEL);

/**
 * @brief Get common USB descriptor
 *
 * Get descriptor from internal descriptor list.
 *
 * @param[in] dn     Pointer to descriptor node
 *
 * @return 0 on success, other values on fail.
 */
static int usbd_get_sn_from_hwid(struct usbd_desc_node *const dn)
{
	static const char hex[] = "0123456789ABCDEF";
	struct usb_string_descriptor *desc = dn->desc;
	uint8_t *desc_data = (uint8_t *)&desc->bString;
	uint8_t hwid[16];
	ssize_t hwid_len;
	ssize_t min_len;

	hwid_len = hwinfo_get_device_id(hwid, sizeof(hwid));
	if (hwid_len < 0) {
		if (hwid_len == -ENOSYS) {
			LOG_WRN("hwinfo not implemented");
			return 0;
		}

		return hwid_len;
	}

	min_len = MIN(hwid_len, desc->bLength / 2);
	for (size_t i = 0; i < min_len; i++) {
		desc_data[i * 2] = hex[hwid[i] >> 4];
		desc_data[i * 2 + 1] = hex[hwid[i] & 0xF];
	}

	LOG_HEXDUMP_DBG(&desc->bString, desc->bLength, "SerialNumber");

	return 0;
}

static inline bool desc_type_equal(const struct usbd_desc_node *const a,
				   const struct usbd_desc_node *const b)
{
	const struct usb_desc_header *const head_a = a->desc;
	const struct usb_desc_header *const head_b = b->desc;

	return head_a->bDescriptorType == head_b->bDescriptorType;
}

/*
 * Add descriptor node to the descriptor list in ascending order by index
 * and sorted by bDescriptorType. For the string descriptors, the function
 * does not care about index zero for the language string descriptor,
 * so if it is not added first, the device will be non-compliant.
 */
static int desc_add_and_update_idx(struct usbd_contex *const uds_ctx,
				   struct usbd_desc_node *const new_nd)
{
	struct usbd_desc_node *tmp_nd;

	SYS_DLIST_FOR_EACH_CONTAINER(&uds_ctx->descriptors, tmp_nd, node) {
		struct usbd_desc_node *next_nd;

		if (!desc_type_equal(tmp_nd, new_nd)) {
			continue;
		}

		next_nd = SYS_DLIST_PEEK_NEXT_CONTAINER(&uds_ctx->descriptors,
							tmp_nd,
							node);

		if (next_nd == NULL) {
			/* Last node of the same bDescriptorType or tail */
			new_nd->str.idx = tmp_nd->str.idx + 1;
			sys_dlist_append(&uds_ctx->descriptors, &new_nd->node);
			LOG_DBG("Add %u behind %u", new_nd->str.idx, tmp_nd->str.idx);

			return 0;
		}

		if (!desc_type_equal(next_nd, new_nd)) {
			/* Last node of the same bDescriptorType */
			new_nd->str.idx = tmp_nd->str.idx + 1;
			sys_dlist_insert(&next_nd->node, &new_nd->node);
			LOG_DBG("Add %u before %u", new_nd->str.idx, next_nd->str.idx);

			return 0;
		}

		if (tmp_nd->str.idx != (next_nd->str.idx - 1)) {
			/* Add between nodes of the same bDescriptorType */
			new_nd->str.idx = tmp_nd->str.idx + 1;
			sys_dlist_insert(&next_nd->node, &new_nd->node);
			LOG_DBG("Add %u between %u and %u",
				tmp_nd->str.idx, next_nd->str.idx, new_nd->str.idx);
			return 0;
		}
	}

	/* If there are none of same bDescriptorType, node idx is set to 0. */
	new_nd->str.idx = 0;
	sys_dlist_append(&uds_ctx->descriptors, &new_nd->node);
	LOG_DBG("Added first descriptor node (usage type %u)", new_nd->str.utype);

	return 0;
}

struct usbd_desc_node *usbd_get_descriptor(struct usbd_contex *const uds_ctx,
					   const uint8_t type, const uint8_t idx)
{
	struct usbd_desc_node *desc_nd;
	struct usb_desc_header *dh;

	SYS_DLIST_FOR_EACH_CONTAINER(&uds_ctx->descriptors, desc_nd, node) {
		dh = desc_nd->desc;
		if (desc_nd->str.idx == idx && dh->bDescriptorType == type) {
			return desc_nd;
		}
	}

	return NULL;
}

int usbd_desc_remove_all(struct usbd_contex *const uds_ctx)
{
	struct usbd_desc_node *tmp;
	sys_dnode_t *node;

	while ((node = sys_dlist_get(&uds_ctx->descriptors))) {
		tmp = CONTAINER_OF(node, struct usbd_desc_node, node);
		LOG_DBG("Remove descriptor node %p", tmp);
	}

	return 0;
}

int usbd_add_descriptor(struct usbd_contex *const uds_ctx,
			struct usbd_desc_node *const desc_nd)
{
	struct usb_device_descriptor *hs_desc, *fs_desc;
	struct usb_desc_header *head;
	int ret = 0;

	usbd_device_lock(uds_ctx);

	hs_desc = uds_ctx->hs_desc;
	fs_desc = uds_ctx->fs_desc;
	if (!fs_desc || !hs_desc || usbd_is_initialized(uds_ctx)) {
		ret = -EPERM;
		goto add_descriptor_error;
	}

	/* Check if descriptor list is initialized */
	if (!sys_dnode_is_linked(&uds_ctx->descriptors)) {
		LOG_DBG("Initialize descriptors list");
		sys_dlist_init(&uds_ctx->descriptors);
	}

	head = desc_nd->desc;
	if (sys_dnode_is_linked(&desc_nd->node)) {
		ret = -EALREADY;
		goto add_descriptor_error;
	}

	ret = desc_add_and_update_idx(uds_ctx, desc_nd);
	if (ret) {
		ret = -EINVAL;
		goto add_descriptor_error;
	}

	if (head->bDescriptorType == USB_DESC_STRING) {
		switch (desc_nd->str.utype) {
		case USBD_DUT_STRING_LANG:
			break;
		case USBD_DUT_STRING_MANUFACTURER:
			hs_desc->iManufacturer = desc_nd->str.idx;
			fs_desc->iManufacturer = desc_nd->str.idx;
			break;
		case USBD_DUT_STRING_PRODUCT:
			hs_desc->iProduct = desc_nd->str.idx;
			fs_desc->iProduct = desc_nd->str.idx;
			break;
		case USBD_DUT_STRING_SERIAL_NUMBER:
			if (!desc_nd->str.custom_sn) {
				ret = usbd_get_sn_from_hwid(desc_nd);
			}

			hs_desc->iSerialNumber = desc_nd->str.idx;
			fs_desc->iSerialNumber = desc_nd->str.idx;
			break;
		default:
			break;
		}
	}

add_descriptor_error:
	usbd_device_unlock(uds_ctx);
	return ret;
}

uint8_t usbd_str_desc_get_idx(const struct usbd_desc_node *const desc_nd)
{
	if (sys_dnode_is_linked(&desc_nd->node)) {
		return desc_nd->str.idx;
	}

	return 0;
}

void usbd_remove_descriptor(struct usbd_desc_node *const desc_nd)
{
	if (sys_dnode_is_linked(&desc_nd->node)) {
		sys_dlist_remove(&desc_nd->node);
		desc_nd->str.idx = 0;
	}
}
