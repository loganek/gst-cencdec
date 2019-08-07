/*
 * Copyright (c) <2019> Alex Ashley <alex@digital-video.org.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <gst/base/gstbytereader.h>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <openssl/evp.h>

#include "gstcencdrm.h"

GST_DEBUG_CATEGORY_EXTERN (gst_cenc_decrypt_debug_category);
#define GST_CAT_DEFAULT gst_cenc_decrypt_debug_category

#define gst_cenc_drm_parent_class parent_class

static void gst_cenc_keypair_dispose (GObject * object);
/*static void gst_cenc_keypair_finalize (GObject * object); */

static void gst_cenc_drm_dispose (GObject * object);
static void gst_cenc_drm_finalize (GObject * object);
static void gst_cenc_drm_clear (GstCencDRM * self);

static GstCencDrmProcessing gst_cenc_drm_should_process_node (GstCencDRM * self,
    const gchar * namespace, const gchar * element, guint * identifier);
static GstCencDrmStatus gst_cenc_drm_configure (GstCencDRM *,
    guint identifier, GstBuffer * data);
static GstCencDrmStatus gst_cenc_drm_add_kid (GstCencDRM *, GstBuffer * kid);
static GstCencDrmStatus
gst_cenc_drm_parse_content_protection_xml_element (GstCencDRM *,
    const gchar * system_id, GstBuffer * pssi);
static void gst_cenc_drm_keypair_dispose (GstCencDRM *, GstCencKeyPair *);

G_DEFINE_TYPE (GstCencDRM, gst_cenc_drm, G_TYPE_OBJECT);

static void
gst_cenc_drm_class_init (GstCencDRMClass * klass)
{
  GObjectClass *object = G_OBJECT_CLASS (klass);

  object->dispose = gst_cenc_drm_dispose;
  object->finalize = gst_cenc_drm_finalize;
  klass->should_process_node = gst_cenc_drm_should_process_node;
  klass->configure = gst_cenc_drm_configure;
  klass->add_kid = gst_cenc_drm_add_kid;
  klass->keypair_dispose = gst_cenc_drm_keypair_dispose;
}

static void
gst_cenc_drm_init (GstCencDRM * self)
{
  /*  g_rec_mutex_init (&self->test_task_lock);
     g_mutex_init (&self->test_task_state_lock);
     g_cond_init (&self->test_task_state_cond); */
  gst_cenc_drm_clear (self);
}

static void
gst_cenc_drm_clear (GstCencDRM * self)
{
  self->drm_type = GST_DRM_UNKNOWN;
  if (self->system_id) {
    gst_buffer_unref (self->system_id);
    self->system_id = NULL;
  }
}

static void
gst_cenc_drm_dispose (GObject * object)
{
  GstCencDRM *self = GST_CENC_DRM (object);

  gst_cenc_drm_clear (self);

  GST_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
}

static void
gst_cenc_drm_finalize (GObject * object)
{
  /*  GstCencDRM *self = GST_CENC_DRM (object); */

  GST_CALL_PARENT (G_OBJECT_CLASS, finalize, (object));
}

static GstCencDrmProcessing
gst_cenc_drm_should_process_node (GstCencDRM * self,
    const gchar * namespace, const gchar * element, guint * identifier)
{
  return GST_DRM_SKIP;
}

static GstCencDrmStatus
gst_cenc_drm_configure (GstCencDRM * self, guint identifier, GstBuffer * data)
{
  return GST_DRM_ERROR_NOT_IMPLEMENTED;
}

static GstCencDrmStatus
gst_cenc_drm_add_kid (GstCencDRM * self, GstBuffer * kid)
{
  return GST_DRM_ERROR_NOT_IMPLEMENTED;
}

GstCencDrmStatus
gst_cenc_drm_process_content_protection_event (GstCencDRM * self,
    GstEvent * event)
{
  const gchar *system_id = NULL;
  GstBuffer *pssi = NULL;
  const gchar *loc = NULL;
  GstCencDrmStatus ret = GST_DRM_ERROR_OTHER;

  gst_event_parse_protection (event, &system_id, &pssi, &loc);
  if (g_ascii_strcasecmp (loc, "dash/mpd") == 0) {
    ret = gst_cenc_drm_parse_content_protection_xml_element (self,
        system_id, pssi);
  } else if (g_str_has_prefix (loc, "isobmff/")) {
    ret = gst_cenc_drm_parse_pssh_box (self, pssi);
  }
  return ret;
}

static GstBuffer *
gst_cenc_drm_buffer_from_raw_node (GstCencDRM * self, xmlNode * node)
{
  GBytes *bytes;
  GstBuffer *buf = NULL;
  xmlChar *node_content;

  node_content = xmlNodeGetContent (node);
  if (node_content) {
    bytes = g_bytes_new (node_content, strlen (node_content));
    buf = gst_buffer_new_wrapped_bytes (bytes);
    g_bytes_unref (bytes);
    xmlFree (node_content);
  }
  return buf;
}

static GstBuffer *
gst_cenc_drm_buffer_from_base64_node (GstCencDRM * self, xmlNode * node)
{
  GBytes *bytes = NULL;
  GstBuffer *buf;
  xmlChar *node_content;

  node_content = xmlNodeGetContent (node);
  if (!node_content) {
    return NULL;
  }
  bytes = gst_cenc_drm_base64_decode (self, (const gchar *) node_content);
  xmlFree (node_content);
  if (bytes) {
    buf = gst_buffer_new_wrapped_bytes (bytes);
    g_bytes_unref (bytes);
  }
  return buf;
}

static GstBuffer *
gst_cenc_drm_buffer_from_hex_node (GstCencDRM * self, xmlNode * node)
{
  return NULL;
}

static GstCencDrmStatus
gst_cenc_drm_walk_xml_nodes (GstCencDRM * self, xmlNode * root)
{
  GstCencDRMClass *klass = GST_CENC_DRM_GET_CLASS (self);
  xmlNode *node;
  GstCencDrmStatus ret = GST_DRM_NOT_FOUND;
  gboolean done = FALSE;

  /* Walk child elements  */
  for (node = root->children; node && !done; node = node->next) {
    guint identifier = 0;
    GstCencDrmProcessing processing;
    GstBuffer *buf = NULL;

    if (node->type != XML_ELEMENT_NODE)
      continue;

    processing = klass->should_process_node (self,
        (const gchar *) node->ns->href,
        (const gchar *) node->name, &identifier);

    switch (processing) {
      case GST_DRM_SKIP:
        break;
      case GST_DRM_PROCESS_RAW:
        buf = gst_cenc_drm_buffer_from_raw_node (self, node);
        if (buf) {
          ret = klass->configure (self, identifier, buf);
          done = TRUE;
        }
        break;
      case GST_DRM_PROCESS_BASE64:
        buf = gst_cenc_drm_buffer_from_base64_node (self, node);
        if (buf) {
          ret = klass->configure (self, identifier, buf);
          done = TRUE;
        }
        break;
      case GST_DRM_PROCESS_HEX:
        buf = gst_cenc_drm_buffer_from_hex_node (self, node);
        if (buf) {
          ret = klass->configure (self, identifier, buf);
          done = TRUE;
        }
        break;
      case GST_DRM_PROCESS_CHILDREN:
        ret = gst_cenc_drm_walk_xml_nodes (self, node);
        done = TRUE;
        break;
    }
    if (buf) {
      gst_buffer_unref (buf);
    }
  }
  return ret;
}


static GstCencDrmStatus
gst_cenc_drm_parse_content_protection_xml_element (GstCencDRM * self,
    const gchar * system_id, GstBuffer * pssi)
{
  GstMapInfo info;
  guint32 data_size;
  xmlDocPtr doc;
  xmlNode *root_element = NULL;
  GstCencDrmStatus ret = GST_DRM_NOT_FOUND;
  xmlNode *node;
  int i;

  gst_buffer_map (pssi, &info, GST_MAP_READ);

  /* this initialize the library and check potential ABI mismatches
   * between the version it was compiled for and the actual shared
   * library used
   */
  LIBXML_TEST_VERSION;

  doc =
      xmlReadMemory (info.data, info.size, "ContentProtection.xml", NULL,
      XML_PARSE_NONET);
  if (!doc) {
    ret = GST_DRM_ERROR_INVALID_MPD;
    GST_ERROR_OBJECT (self, "Failed to parse XML from pssi event");
    goto beach;
  }
  root_element = xmlDocGetRootElement (doc);

  if (root_element->type != XML_ELEMENT_NODE
      || xmlStrcmp (root_element->name, (xmlChar *) "ContentProtection") != 0) {
    GST_ERROR_OBJECT (self, "Failed to find ContentProtection element");
    ret = GST_DRM_ERROR_INVALID_MPD;
    goto beach;
  }
  ret = gst_cenc_drm_walk_xml_nodes (self, root_element);

beach:
  gst_buffer_unmap (pssi, &info);
  if (doc)
    xmlFreeDoc (doc);
  return ret;
}

#define PSSH_CHECK(a) {if (!(a)) { ret=GST_DRM_ERROR_INVALID_PSSH; goto beach; } }

GstCencDrmStatus
gst_cenc_drm_parse_pssh_box (GstCencDRM * self, GstBuffer * pssh)
{
  GstCencDRMClass *klass = GST_CENC_DRM_GET_CLASS (self);
  GstMapInfo info;
  GstByteReader br;
  guint8 version;
  guint32 pssh_length;
  guint32 data_size;
  GstCencDrmStatus ret = GST_DRM_OK;
  const guint8 *system_id;

  gst_buffer_map (pssh, &info, GST_MAP_READ);
  gst_byte_reader_init (&br, info.data, info.size);

  PSSH_CHECK (gst_byte_reader_get_uint32_be (&br, &pssh_length));
  if (pssh_length != info.size) {
    GST_WARNING_OBJECT (self, "Invalid PSSH length. Expected %lu got %u",
        info.size, pssh_length);
    goto beach;

  }
  gst_byte_reader_skip_unchecked (&br, 4);      /* 'PSSH' */
  PSSH_CHECK (gst_byte_reader_get_uint8 (&br, &version));
  GST_DEBUG_OBJECT (self, "pssh version: %u", version);
  gst_byte_reader_skip_unchecked (&br, 3);      /* FullBox flags */
  PSSH_CHECK (gst_byte_reader_get_data (&br, SYSTEM_ID_LENGTH, &system_id));

  if (gst_buffer_memcmp (self->system_id, 0, system_id, SYSTEM_ID_LENGTH) != 0) {
    GST_DEBUG_OBJECT (self, "Skipping PSSH as not for this DRM system");
    ret = GST_DRM_OK;
    goto beach;
  }
  if (version > 0) {
    /* Parse KeyIDs */
    guint32 kid_count = 0;

    PSSH_CHECK (gst_byte_reader_get_uint32_be (&br, &kid_count));
    GST_DEBUG_OBJECT (self, "there are %u key IDs", kid_count);
    if (gst_byte_reader_get_remaining (&br) < KID_LENGTH * kid_count) {
      ret = GST_DRM_ERROR_INVALID_PSSH;
      goto beach;
    }
    while (kid_count > 0) {
      GBytes *kid_bytes;
      GstBuffer *kid_buf;

      kid_bytes =
          g_bytes_new (gst_byte_reader_get_data_unchecked (&br, KID_LENGTH),
          KID_LENGTH);
      kid_buf = gst_buffer_new_wrapped_bytes (kid_bytes);
      klass->add_kid (self, kid_buf);
      gst_buffer_unref (kid_buf);
      g_bytes_unref (kid_bytes);
      --kid_count;
    }
  }

  /* Parse Data */
  PSSH_CHECK (gst_byte_reader_get_uint32_be (&br, &data_size));
  GST_DEBUG_OBJECT (self, "pssh data size: %u", data_size);

  if (data_size > 0U) {
    GBytes *bytes;
    GstBuffer *buffer;

    GST_DEBUG_OBJECT (self, "cenc protection system data size: %u", data_size);
    bytes = g_bytes_new (gst_byte_reader_get_data_unchecked (&br, data_size),
        data_size);
    buffer = gst_buffer_new_wrapped_bytes (bytes);
    ret = klass->configure (self, GST_DRM_IDENTIFIER_PSSH_PAYLOAD, buffer);
    gst_buffer_unref (buffer);
    g_bytes_unref (bytes);
  }
beach:
  gst_buffer_unmap (pssh, &info);
  return ret;
}


GstCencKeyPair *
gst_cenc_drm_keypair_ref (GstCencKeyPair * kp)
{
  kp->ref_count++;
  return kp;
}

void
gst_cenc_drm_keypair_unref (GstCencKeyPair * kp)
{
  g_return_if_fail (kp != NULL);
  kp->ref_count--;
  if (kp->ref_count == 0) {
    if (kp->owner) {
      GST_CENC_DRM_GET_CLASS (kp->owner)->keypair_dispose (kp->owner, kp);
    } else {
      g_free (kp);
    }
  }
}

static void
gst_cenc_drm_keypair_dispose (GstCencDRM * self, GstCencKeyPair * key_pair)
{
  if (key_pair->key_id) {
    g_bytes_unref (key_pair->key_id);
  }
  if (key_pair->key) {
    g_bytes_unref (key_pair->key);
  }
  g_free (key_pair);
}

GBytes *
gst_cenc_drm_base64_decode (GstCencDRM * self, const gchar * encoded)
{
  gint alloc_size;
  gint padding = 0;
  gint pos;
  gpointer decoded;
  gint decoded_len;
  gsize encoded_len;

  encoded_len = strlen (encoded);

  /* For every 4 input bytes exactly 3 output bytes will be produced. The output
     will be padded with 0 bits if necessary to ensure that the output is always
     3 bytes for every 4 input bytes. */
  alloc_size = 3 + encoded_len * 3 / 4;

  /* EVP_DecodeBlock() includes the padding in its output length, therefore it
     needs to be removed after decoding */
  pos = encoded_len - 1;
  while (pos && encoded[pos] == '=') {
    padding++;
    pos--;
  }
  decoded = g_malloc (alloc_size);
  decoded_len =
      EVP_DecodeBlock (decoded, (const unsigned char *) encoded, encoded_len);
  if (decoded_len < 1) {
    GST_ERROR_OBJECT (self, "Failed base64 decode");
    g_free (decoded);
    return NULL;
  }
  g_assert_true (decoded_len <= alloc_size);
  decoded_len -= padding;
  if (decoded_len < alloc_size) {
    decoded = g_realloc (decoded, decoded_len);
  }
  return g_bytes_new_take (decoded, decoded_len);
}

GstBuffer *
gst_cenc_drm_urn_string_to_raw (GstCencDRM * self, const gchar * urn)
{
  const gchar prefix[] = "urn:uuid:";
  GstBuffer *rv;
  GstMapInfo map;
  gboolean failed = FALSE;
  guint i, pos, length;

  if (!g_ascii_strncasecmp (prefix, urn, sizeof (prefix)) != 0) {
    return NULL;
  }
  length = strlen (urn);
  rv = gst_buffer_new_allocate (NULL, SYSTEM_ID_LENGTH, NULL);
  gst_buffer_map (rv, &map, GST_MAP_READWRITE);
  for (i = 0, pos = sizeof (prefix); i < SYSTEM_ID_LENGTH && pos < length; ++i) {
    guint b;
    if (urn[pos] == '-') {
      pos++;
    }
    if ((pos + 1) >= length || !sscanf (&urn[pos], "%02x", &b)) {
      failed = TRUE;
      break;
    }
    map.data[i] = b;
    pos += 2;
  }
  gst_buffer_unmap (rv, &map);
  if (failed) {
    gst_buffer_unref (rv);
    rv = NULL;
  }
  return rv;
}
