/*
 * cputest.c: Test the libvirtd internal CPU APIs
 *
 * Copyright (C) 2010 Red Hat, Inc.
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
 * Author: Jiri Denemark <jdenemar@redhat.com>
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <fcntl.h>

#include "internal.h"
#include "xml.h"
#include "memory.h"
#include "buf.h"
#include "testutils.h"
#include "cpu_conf.h"
#include "cpu/cpu.h"
#include "cpu/cpu_map.h"

static const char *progname;
static const char *abs_srcdir;
static const char *abs_top_srcdir;

#define VIR_FROM_THIS VIR_FROM_CPU
#define MAX_FILE 4096

enum compResultShadow {
    ERROR           = VIR_CPU_COMPARE_ERROR,
    INCOMPATIBLE    = VIR_CPU_COMPARE_INCOMPATIBLE,
    IDENTICAL       = VIR_CPU_COMPARE_IDENTICAL,
    SUPERSET        = VIR_CPU_COMPARE_SUPERSET
};

enum cpuTestBoolWithError {
    FAIL    = -1,
    NO      = 0,
    YES     = 1
};

enum api {
    API_COMPARE,
    API_GUEST_DATA,
    API_BASELINE,
    API_UPDATE,
    API_HAS_FEATURE
};

static const char *apis[] = {
    "compare",
    "guest data",
    "baseline",
    "update",
    "has feature"
};

struct data {
    const char *arch;
    enum api api;
    const char *host;
    const char *name;
    const char **models;
    const char *modelsName;
    unsigned int nmodels;
    const char *preferred;
    int result;
};


static virCPUDefPtr
cpuTestLoadXML(const char *arch, const char *name)
{
    char xml[PATH_MAX];
    xmlDocPtr doc = NULL;
    xmlXPathContextPtr ctxt = NULL;
    virCPUDefPtr cpu = NULL;

    snprintf(xml, PATH_MAX,
             "%s/cputestdata/%s-%s.xml",
             abs_srcdir, arch, name);

    if (!(doc = virXMLParseFile(xml)) ||
        !(ctxt = xmlXPathNewContext(doc)))
        goto cleanup;

    ctxt->node = xmlDocGetRootElement(doc);
    cpu = virCPUDefParseXML(ctxt->node, ctxt, VIR_CPU_TYPE_AUTO);

cleanup:
    xmlXPathFreeContext(ctxt);
    xmlFreeDoc(doc);
    return cpu;
}


static virCPUDefPtr *
cpuTestLoadMultiXML(const char *arch,
                    const char *name,
                    unsigned int *count)
{
    char xml[PATH_MAX];
    xmlDocPtr doc = NULL;
    xmlXPathContextPtr ctxt = NULL;
    xmlNodePtr *nodes = NULL;
    virCPUDefPtr *cpus = NULL;
    int n;
    int i;

    snprintf(xml, PATH_MAX,
             "%s/cputestdata/%s-%s.xml",
             abs_srcdir, arch, name);

    if (!(doc = virXMLParseFile(xml)) ||
        !(ctxt = xmlXPathNewContext(doc)))
        goto error;

    ctxt->node = xmlDocGetRootElement(doc);

    n = virXPathNodeSet("/cpuTest/cpu", ctxt, &nodes);
    if (n <= 0 || !(cpus = calloc(n, sizeof(virCPUDefPtr))))
        goto error;

    for (i = 0; i < n; i++) {
        ctxt->node = nodes[i];
        cpus[i] = virCPUDefParseXML(nodes[i], ctxt, VIR_CPU_TYPE_HOST);
        if (!cpus[i])
            goto error;
    }

    *count = n;

cleanup:
    free(nodes);
    xmlXPathFreeContext(ctxt);
    xmlFreeDoc(doc);
    return cpus;

error:
    if (cpus) {
        for (i = 0; i < n; i++)
            virCPUDefFree(cpus[i]);
        free(cpus);
        cpus = NULL;
    }
    goto cleanup;
}


static int
cpuTestCompareXML(const char *arch,
                  const virCPUDefPtr cpu,
                  const char *name)
{
    char xml[PATH_MAX];
    char expected[MAX_FILE];
    char *expectedPtr = &(expected[0]);
    char *actual = NULL;
    int ret = -1;

    snprintf(xml, PATH_MAX,
             "%s/cputestdata/%s-%s.xml",
             abs_srcdir, arch, name);

    if (virtTestLoadFile(xml, &expectedPtr, MAX_FILE) < 0)
        goto cleanup;

    if (!(actual = virCPUDefFormat(cpu, NULL, 0)))
        goto cleanup;

    if (STRNEQ(expected, actual)) {
        virtTestDifference(stderr, expected, actual);
        goto cleanup;
    }

    ret = 0;

cleanup:
    free(actual);
    return ret;
}


static const char *
cpuTestCompResStr(virCPUCompareResult result)
{
    switch (result) {
    case VIR_CPU_COMPARE_ERROR:         return "ERROR";
    case VIR_CPU_COMPARE_INCOMPATIBLE:  return "INCOMPATIBLE";
    case VIR_CPU_COMPARE_IDENTICAL:     return "IDENTICAL";
    case VIR_CPU_COMPARE_SUPERSET:      return "SUPERSET";
    }

    return "unknown";
}


static const char *
cpuTestBoolWithErrorStr(enum cpuTestBoolWithError result)
{
    switch (result) {
    case FAIL:  return "FAIL";
    case NO:    return "NO";
    case YES:   return "YES";
    }

    return "unknown";
}


static int
cpuTestCompare(const void *arg)
{
    const struct data *data = arg;
    int ret = -1;
    virCPUDefPtr host = NULL;
    virCPUDefPtr cpu = NULL;
    virCPUCompareResult result;

    if (!(host = cpuTestLoadXML(data->arch, data->host)) ||
        !(cpu = cpuTestLoadXML(data->arch, data->name)))
        goto cleanup;

    result = cpuCompare(host, cpu);
    if (data->result == VIR_CPU_COMPARE_ERROR)
        virResetLastError();

    if (data->result != result) {
        if (virTestGetVerbose()) {
            fprintf(stderr, "\nExpected result %s, got %s\n",
                    cpuTestCompResStr(data->result),
                    cpuTestCompResStr(result));
            /* Pad to line up with test name ... in virTestRun */
            fprintf(stderr, "%74s", "... ");
        }
        goto cleanup;
    }

    ret = 0;

cleanup:
    virCPUDefFree(host);
    virCPUDefFree(cpu);
    return ret;
}


static int
cpuTestGuestData(const void *arg)
{
    const struct data *data = arg;
    int ret = -1;
    virCPUDefPtr host = NULL;
    virCPUDefPtr cpu = NULL;
    virCPUDefPtr guest = NULL;
    union cpuData *guestData = NULL;
    virCPUCompareResult cmpResult;
    virBuffer buf = VIR_BUFFER_INITIALIZER;
    char *result = NULL;

    if (!(host = cpuTestLoadXML(data->arch, data->host)) ||
        !(cpu = cpuTestLoadXML(data->arch, data->name)))
        goto cleanup;

    cmpResult = cpuGuestData(host, cpu, &guestData);
    if (cmpResult == VIR_CPU_COMPARE_ERROR ||
        cmpResult == VIR_CPU_COMPARE_INCOMPATIBLE)
        goto cleanup;

    if (VIR_ALLOC(guest) < 0 || !(guest->arch = strdup(host->arch)))
        goto cleanup;

    guest->type = VIR_CPU_TYPE_GUEST;
    guest->match = VIR_CPU_MATCH_EXACT;
    if (cpuDecode(guest, guestData, data->models,
                  data->nmodels, data->preferred) < 0) {
        if (data->result < 0) {
            virResetLastError();
            ret = 0;
        }
        goto cleanup;
    }

    virBufferVSprintf(&buf, "%s+%s", data->host, data->name);
    if (data->nmodels)
        virBufferVSprintf(&buf, ",%s", data->modelsName);
    if (data->preferred)
        virBufferVSprintf(&buf, ",%s", data->preferred);
    virBufferAddLit(&buf, "-result");

    if (virBufferError(&buf)) {
        virBufferFreeAndReset(&buf);
        goto cleanup;
    }
    result = virBufferContentAndReset(&buf);

    ret = cpuTestCompareXML(data->arch, guest, result);

cleanup:
    VIR_FREE(result);
    if (host)
        cpuDataFree(host->arch, guestData);
    virCPUDefFree(host);
    virCPUDefFree(cpu);
    virCPUDefFree(guest);
    return ret;
}


static int
cpuTestBaseline(const void *arg)
{
    const struct data *data = arg;
    int ret = -1;
    virCPUDefPtr *cpus = NULL;
    virCPUDefPtr baseline = NULL;
    unsigned int ncpus = 0;
    char result[PATH_MAX];
    unsigned int i;

    if (!(cpus = cpuTestLoadMultiXML(data->arch, data->name, &ncpus)))
        goto cleanup;

    baseline = cpuBaseline(cpus, ncpus, NULL, 0);
    if (data->result < 0) {
        virResetLastError();
        if (!baseline)
            ret = 0;
        else if (virTestGetVerbose()) {
            fprintf(stderr, "\n%-70s... ",
                    "cpuBaseline was expected to fail but it succeeded");
        }
        goto cleanup;
    }
    if (!baseline)
        goto cleanup;

    snprintf(result, PATH_MAX, "%s-result", data->name);
    if (cpuTestCompareXML(data->arch, baseline, result) < 0)
        goto cleanup;

    for (i = 0; i < ncpus; i++) {
        virCPUCompareResult cmp;

        cmp = cpuCompare(cpus[i], baseline);
        if (cmp != VIR_CPU_COMPARE_SUPERSET &&
            cmp != VIR_CPU_COMPARE_IDENTICAL) {
            if (virTestGetVerbose()) {
                fprintf(stderr,
                        "\nbaseline CPU is incompatible with CPU %u\n", i);
                fprintf(stderr, "%74s", "... ");
            }
            ret = -1;
            goto cleanup;
        }
    }

    ret = 0;

cleanup:
    if (cpus) {
        for (i = 0; i < ncpus; i++)
            virCPUDefFree(cpus[i]);
        free(cpus);
    }
    virCPUDefFree(baseline);
    return ret;
}


static int
cpuTestUpdate(const void *arg)
{
    const struct data *data = arg;
    int ret = -1;
    virCPUDefPtr host = NULL;
    virCPUDefPtr cpu = NULL;
    char result[PATH_MAX];

    if (!(host = cpuTestLoadXML(data->arch, data->host)) ||
        !(cpu = cpuTestLoadXML(data->arch, data->name)))
        goto cleanup;

    if (cpuUpdate(cpu, host) < 0)
        goto cleanup;

    snprintf(result, PATH_MAX, "%s+%s", data->host, data->name);
    ret = cpuTestCompareXML(data->arch, cpu, result);

cleanup:
    virCPUDefFree(host);
    virCPUDefFree(cpu);
    return ret;
}


static int
cpuTestHasFeature(const void *arg)
{
    const struct data *data = arg;
    int ret = -1;
    virCPUDefPtr host = NULL;
    union cpuData *hostData = NULL;
    int result;

    if (!(host = cpuTestLoadXML(data->arch, data->host)))
        goto cleanup;

    if (cpuEncode(host->arch, host, NULL, &hostData,
                  NULL, NULL, NULL, NULL) < 0)
        goto cleanup;

    result = cpuHasFeature(host->arch, hostData, data->name);
    if (data->result == -1)
        virResetLastError();

    if (data->result != result) {
        if (virTestGetVerbose()) {
            fprintf(stderr, "\nExpected result %s, got %s\n",
                    cpuTestBoolWithErrorStr(data->result),
                    cpuTestBoolWithErrorStr(result));
            /* Pad to line up with test name ... in virTestRun */
            fprintf(stderr, "%74s", "... ");
        }
        goto cleanup;
    }

    ret = 0;

cleanup:
    if (host)
        cpuDataFree(host->arch, hostData);
    virCPUDefFree(host);
    return ret;
}


static int (*cpuTest[])(const void *) = {
    cpuTestCompare,
    cpuTestGuestData,
    cpuTestBaseline,
    cpuTestUpdate,
    cpuTestHasFeature
};


static int
cpuTestRun(const char *name, const struct data *data)
{
    char label[PATH_MAX];

    snprintf(label, PATH_MAX, "CPU %s(%s): %s",
             apis[data->api], data->arch, name);

    free(virtTestLogContentAndReset());

    if (virtTestRun(label, 1, cpuTest[data->api], data) < 0) {
        if (virTestGetDebug()) {
            char *log;
            if ((log = virtTestLogContentAndReset()) &&
                 strlen(log) > 0)
                fprintf(stderr, "\n%s\n", log);
            free(log);
        }
        return -1;
    }

    return 0;
}


static const char *model486[]   = { "486" };
static const char *nomodel[]    = { "nomodel" };
static const char *models[]     = { "qemu64", "core2duo", "Nehalem" };

static int
mymain(int argc, char **argv)
{
    int ret = 0;
    char cwd[PATH_MAX];
    char map[PATH_MAX];

    progname = argv[0];

    if (argc > 1) {
        fprintf(stderr, "Usage: %s\n", progname);
        return EXIT_FAILURE;
    }

    abs_srcdir = getenv("abs_srcdir");
    if (!abs_srcdir)
        abs_srcdir = getcwd(cwd, sizeof(cwd));

    abs_top_srcdir = getenv("abs_top_srcdir");
    if (!abs_top_srcdir)
        abs_top_srcdir = "..";

    snprintf(map, PATH_MAX, "%s/src/cpu/cpu_map.xml", abs_top_srcdir);
    if (cpuMapOverride(map) < 0)
        return EXIT_FAILURE;

#define DO_TEST(arch, api, name, host, cpu,                             \
                models, nmodels, preferred, result)                     \
    do {                                                                \
        struct data data = {                                            \
            arch, api, host, cpu, models,                               \
            models == NULL ? NULL : #models,                            \
            nmodels, preferred, result    \
        };                                                              \
        if (cpuTestRun(name, &data) < 0)                                \
            ret = -1;                                                   \
    } while (0)

#define DO_TEST_COMPARE(arch, host, cpu, result)                        \
    DO_TEST(arch, API_COMPARE,                                          \
            host "/" cpu " (" #result ")",                              \
            host, cpu, NULL, 0, NULL, result)

#define DO_TEST_UPDATE(arch, host, cpu, result)                         \
    do {                                                                \
        DO_TEST(arch, API_UPDATE,                                       \
                cpu " on " host,                                        \
                host, cpu, NULL, 0, NULL, 0);                           \
        DO_TEST_COMPARE(arch, host, host "+" cpu, result);              \
    } while (0)

#define DO_TEST_BASELINE(arch, name, result)                            \
    DO_TEST(arch, API_BASELINE, name, NULL, "baseline-" name,           \
            NULL, 0, NULL, result)

#define DO_TEST_HASFEATURE(arch, host, feature, result)                 \
    DO_TEST(arch, API_HAS_FEATURE,                                      \
            host "/" feature " (" #result ")",                          \
            host, feature, NULL, 0, NULL, result)

#define DO_TEST_GUESTDATA(arch, host, cpu, models, preferred, result)   \
    DO_TEST(arch, API_GUEST_DATA,                                       \
            host "/" cpu " (" #models ", pref=" #preferred ")",         \
            host, cpu, models,                                          \
            models == NULL ? 0 : sizeof(models) / sizeof(char *),       \
            preferred, result)

    /* host to host comparison */
    DO_TEST_COMPARE("x86", "host", "host", IDENTICAL);
    DO_TEST_COMPARE("x86", "host", "host-better", INCOMPATIBLE);
    DO_TEST_COMPARE("x86", "host", "host-worse", SUPERSET);
    DO_TEST_COMPARE("x86", "host", "host-amd-fake", INCOMPATIBLE);
    DO_TEST_COMPARE("x86", "host", "host-incomp-arch", INCOMPATIBLE);
    DO_TEST_COMPARE("x86", "host", "host-no-vendor", IDENTICAL);
    DO_TEST_COMPARE("x86", "host-no-vendor", "host", INCOMPATIBLE);

    /* guest to host comparison */
    DO_TEST_COMPARE("x86", "host", "bogus-model", ERROR);
    DO_TEST_COMPARE("x86", "host", "bogus-feature", ERROR);
    DO_TEST_COMPARE("x86", "host", "min", SUPERSET);
    DO_TEST_COMPARE("x86", "host", "pentium3", SUPERSET);
    DO_TEST_COMPARE("x86", "host", "exact", SUPERSET);
    DO_TEST_COMPARE("x86", "host", "exact-forbid", INCOMPATIBLE);
    DO_TEST_COMPARE("x86", "host", "exact-forbid-extra", SUPERSET);
    DO_TEST_COMPARE("x86", "host", "exact-disable", SUPERSET);
    DO_TEST_COMPARE("x86", "host", "exact-disable2", SUPERSET);
    DO_TEST_COMPARE("x86", "host", "exact-disable-extra", SUPERSET);
    DO_TEST_COMPARE("x86", "host", "exact-require", SUPERSET);
    DO_TEST_COMPARE("x86", "host", "exact-require-extra", INCOMPATIBLE);
    DO_TEST_COMPARE("x86", "host", "exact-force", SUPERSET);
    DO_TEST_COMPARE("x86", "host", "strict", INCOMPATIBLE);
    DO_TEST_COMPARE("x86", "host", "strict-full", IDENTICAL);
    DO_TEST_COMPARE("x86", "host", "strict-disable", IDENTICAL);
    DO_TEST_COMPARE("x86", "host", "strict-force-extra", IDENTICAL);
    DO_TEST_COMPARE("x86", "host", "guest", SUPERSET);
    DO_TEST_COMPARE("x86", "host", "pentium3-amd", INCOMPATIBLE);
    DO_TEST_COMPARE("x86", "host-amd", "pentium3-amd", SUPERSET);
    DO_TEST_COMPARE("x86", "host-worse", "nehalem-force", IDENTICAL);

    /* guest updates for migration
     * automatically compares host CPU with the result */
    DO_TEST_UPDATE("x86", "host", "min", IDENTICAL);
    DO_TEST_UPDATE("x86", "host", "pentium3", IDENTICAL);
    DO_TEST_UPDATE("x86", "host", "guest", SUPERSET);

    /* computing baseline CPUs */
    DO_TEST_BASELINE("x86", "incompatible-vendors", -1);
    DO_TEST_BASELINE("x86", "no-vendor", 0);
    DO_TEST_BASELINE("x86", "some-vendors", 0);
    DO_TEST_BASELINE("x86", "1", 0);
    DO_TEST_BASELINE("x86", "2", 0);

    /* CPU features */
    DO_TEST_HASFEATURE("x86", "host", "vmx", YES);
    DO_TEST_HASFEATURE("x86", "host", "lm", YES);
    DO_TEST_HASFEATURE("x86", "host", "sse4.1", YES);
    DO_TEST_HASFEATURE("x86", "host", "3dnowext", NO);
    DO_TEST_HASFEATURE("x86", "host", "skinit", NO);
    DO_TEST_HASFEATURE("x86", "host", "foo", FAIL);

    /* computing guest data and decoding the data into a guest CPU XML */
    DO_TEST_GUESTDATA("x86", "host", "guest", NULL, NULL, 0);
    DO_TEST_GUESTDATA("x86", "host-better", "pentium3", NULL, NULL, 0);
    DO_TEST_GUESTDATA("x86", "host-better", "pentium3", NULL, "pentium3", 0);
    DO_TEST_GUESTDATA("x86", "host-better", "pentium3", NULL, "core2duo", 0);
    DO_TEST_GUESTDATA("x86", "host-worse", "guest", NULL, NULL, 0);
    DO_TEST_GUESTDATA("x86", "host", "strict-force-extra", NULL, NULL, 0);
    DO_TEST_GUESTDATA("x86", "host", "nehalem-force", NULL, NULL, 0);
    DO_TEST_GUESTDATA("x86", "host", "guest", model486, NULL, 0);
    DO_TEST_GUESTDATA("x86", "host", "guest", models, NULL, 0);
    DO_TEST_GUESTDATA("x86", "host", "guest", models, "Penryn", 0);
    DO_TEST_GUESTDATA("x86", "host", "guest", models, "qemu64", 0);
    DO_TEST_GUESTDATA("x86", "host", "guest", nomodel, NULL, -1);

    return (ret == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}

VIRT_TEST_MAIN(mymain)
