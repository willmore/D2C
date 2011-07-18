/*
 * storage_encryption_conf.c: volume encryption information
 *
 * Copyright (C) 2009-2010 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 *
 * Red Hat Author: Miloslav Trmač <mitr@redhat.com>
 */

#include <config.h>

#include <fcntl.h>
#include <unistd.h>

#include "internal.h"

#include "buf.h"
#include "memory.h"
#include "storage_conf.h"
#include "storage_encryption_conf.h"
#include "util.h"
#include "xml.h"
#include "virterror_internal.h"
#include "uuid.h"
#include "files.h"

#define VIR_FROM_THIS VIR_FROM_STORAGE

VIR_ENUM_IMPL(virStorageEncryptionSecretType,
              VIR_STORAGE_ENCRYPTION_SECRET_TYPE_LAST, "passphrase")

VIR_ENUM_IMPL(virStorageEncryptionFormat,
              VIR_STORAGE_ENCRYPTION_FORMAT_LAST,
              "default", "qcow")

static void
virStorageEncryptionSecretFree(virStorageEncryptionSecretPtr secret)
{
    if (!secret)
        return;
    VIR_FREE(secret);
}

void
virStorageEncryptionFree(virStorageEncryptionPtr enc)
{
    size_t i;

    if (!enc)
        return;

    for (i = 0; i < enc->nsecrets; i++)
        virStorageEncryptionSecretFree(enc->secrets[i]);
    VIR_FREE(enc->secrets);
    VIR_FREE(enc);
}

static virStorageEncryptionSecretPtr
virStorageEncryptionSecretParse(xmlXPathContextPtr ctxt,
                                xmlNodePtr node)
{
    xmlNodePtr old_node;
    virStorageEncryptionSecretPtr ret;
    char *type_str;
    int type;
    char *uuidstr = NULL;

    if (VIR_ALLOC(ret) < 0) {
        virReportOOMError();
        return NULL;
    }

    old_node = ctxt->node;
    ctxt->node = node;

    type_str = virXPathString("string(./@type)", ctxt);
    if (type_str == NULL) {
        virStorageReportError(VIR_ERR_XML_ERROR, "%s",
                              _("unknown volume encryption secret type"));
        goto cleanup;
    }
    type = virStorageEncryptionSecretTypeTypeFromString(type_str);
    if (type < 0) {
        virStorageReportError(VIR_ERR_XML_ERROR,
                              _("unknown volume encryption secret type %s"),
                              type_str);
        VIR_FREE(type_str);
        goto cleanup;
    }
    VIR_FREE(type_str);
    ret->type = type;

    uuidstr = virXPathString("string(./@uuid)", ctxt);
    if (uuidstr) {
        if (virUUIDParse(uuidstr, ret->uuid) < 0) {
            virStorageReportError(VIR_ERR_XML_ERROR,
                                  _("malformed volume encryption uuid '%s'"),
                                  uuidstr);
            goto cleanup;
        }
        VIR_FREE(uuidstr);
    } else {
        virStorageReportError(VIR_ERR_XML_ERROR, "%s",
                              _("missing volume encryption uuid"));
        goto cleanup;
    }
    ctxt->node = old_node;
    return ret;

  cleanup:
    virStorageEncryptionSecretFree(ret);
    VIR_FREE(uuidstr);
    ctxt->node = old_node;
    return NULL;
}

static virStorageEncryptionPtr
virStorageEncryptionParseXML(xmlXPathContextPtr ctxt)
{
    xmlNodePtr *nodes = NULL;
    virStorageEncryptionPtr ret;
    char *format_str;
    int format, i, n;

    if (VIR_ALLOC(ret) < 0) {
        virReportOOMError();
        return NULL;
    }

    format_str = virXPathString("string(./@format)", ctxt);
    if (format_str == NULL) {
        virStorageReportError(VIR_ERR_XML_ERROR, "%s",
                              _("unknown volume encryption format"));
        goto cleanup;
    }
    format = virStorageEncryptionFormatTypeFromString(format_str);
    if (format < 0) {
        virStorageReportError(VIR_ERR_XML_ERROR,
                              _("unknown volume encryption format type %s"),
                              format_str);
        VIR_FREE(format_str);
        goto cleanup;
    }
    VIR_FREE(format_str);
    ret->format = format;

    n = virXPathNodeSet("./secret", ctxt, &nodes);
    if (n < 0){
        virStorageReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                              _("cannot extract volume encryption secrets"));
        goto cleanup;
    }
    if (n != 0 && VIR_ALLOC_N(ret->secrets, n) < 0) {
        virReportOOMError();
        goto cleanup;
    }
    ret->nsecrets = n;
    for (i = 0; i < n; i++) {
        ret->secrets[i] = virStorageEncryptionSecretParse(ctxt, nodes[i]);
        if (ret->secrets[i] == NULL)
            goto cleanup;
    }
    VIR_FREE(nodes);

    return ret;

  cleanup:
    VIR_FREE(nodes);
    virStorageEncryptionFree(ret);
    return NULL;
}

virStorageEncryptionPtr
virStorageEncryptionParseNode(xmlDocPtr xml, xmlNodePtr root)
{
    xmlXPathContextPtr ctxt = NULL;
    virStorageEncryptionPtr enc = NULL;

    if (STRNEQ((const char *) root->name, "encryption")) {
        virStorageReportError(VIR_ERR_XML_ERROR,
                              "%s", _("unknown root element for volume "
                                      "encryption information"));
        goto cleanup;
    }

    ctxt = xmlXPathNewContext(xml);
    if (ctxt == NULL) {
        virReportOOMError();
        goto cleanup;
    }

    ctxt->node = root;
    enc = virStorageEncryptionParseXML(ctxt);

  cleanup:
    xmlXPathFreeContext(ctxt);
    return enc;
}


static int
virStorageEncryptionSecretFormat(virBufferPtr buf,
                                 virStorageEncryptionSecretPtr secret,
                                 int indent)
{
    const char *type;
    char uuidstr[VIR_UUID_STRING_BUFLEN];

    type = virStorageEncryptionSecretTypeTypeToString(secret->type);
    if (!type) {
        virStorageReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                              _("unexpected volume encryption secret type"));
        return -1;
    }

    virUUIDFormat(secret->uuid, uuidstr);
    virBufferVSprintf(buf, "%*s<secret type='%s' uuid='%s'/>\n",
                      indent, "", type, uuidstr);
    return 0;
}

int
virStorageEncryptionFormat(virBufferPtr buf,
                           virStorageEncryptionPtr enc,
                           int indent)
{
    const char *format;
    size_t i;

    format = virStorageEncryptionFormatTypeToString(enc->format);
    if (!format) {
        virStorageReportError(VIR_ERR_INTERNAL_ERROR,
                              "%s", _("unexpected encryption format"));
        return -1;
    }
    virBufferVSprintf(buf, "%*s<encryption format='%s'>\n",
                      indent, "", format);

    for (i = 0; i < enc->nsecrets; i++) {
        if (virStorageEncryptionSecretFormat(buf, enc->secrets[i],
                                             indent + 2) < 0)
            return -1;
    }

    virBufferVSprintf(buf, "%*s</encryption>\n", indent, "");

    return 0;
}

int
virStorageGenerateQcowPassphrase(unsigned char *dest)
{
    int fd;
    size_t i;

    /* A qcow passphrase is up to 16 bytes, with any data following a NUL
       ignored.  Prohibit control and non-ASCII characters to avoid possible
       unpleasant surprises with the qemu monitor input mechanism. */
    fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) {
        virStorageReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                              _("Cannot open /dev/urandom"));
        return -1;
    }
    i = 0;
    while (i < VIR_STORAGE_QCOW_PASSPHRASE_SIZE) {
        ssize_t r;

        while ((r = read(fd, dest + i, 1)) == -1 && errno == EINTR)
            ;
        if (r <= 0) {
            virStorageReportError(VIR_ERR_INTERNAL_ERROR, "%s",
                                  _("Cannot read from /dev/urandom"));
            VIR_FORCE_CLOSE(fd);
            return -1;
        }
        if (dest[i] >= 0x20 && dest[i] <= 0x7E)
            i++; /* Got an acceptable character */
    }
    VIR_FORCE_CLOSE(fd);
    return 0;
}
