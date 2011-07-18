#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#include <sys/types.h>
#include <fcntl.h>

#include "internal.h"
#include "testutils.h"
#include "xml.h"
#include "threads.h"
#include "nwfilter_params.h"
#include "nwfilter_conf.h"
#include "testutilsqemu.h"

static char *progname;
static char *abs_srcdir;

#define MAX_FILE 4096


static int testCompareXMLToXMLFiles(const char *inxml,
                                    const char *outxml,
                                    bool expect_error) {
    char inXmlData[MAX_FILE];
    char *inXmlPtr = &(inXmlData[0]);
    char outXmlData[MAX_FILE];
    char *outXmlPtr = &(outXmlData[0]);
    char *actual = NULL;
    int ret = -1;
    virNWFilterDefPtr dev = NULL;

    if (virtTestLoadFile(inxml, &inXmlPtr, MAX_FILE) < 0)
        goto fail;
    if (virtTestLoadFile(outxml, &outXmlPtr, MAX_FILE) < 0)
        goto fail;

    virResetLastError();

    if (!(dev = virNWFilterDefParseString(NULL, inXmlData)))
        goto fail;

    if (!!virGetLastError() != expect_error)
        goto fail;

    if (expect_error) {
        /* need to suppress the errors */
        virResetLastError();
    }

    if (!(actual = virNWFilterDefFormat(dev)))
        goto fail;

    if (STRNEQ(outXmlData, actual)) {
        virtTestDifference(stderr, outXmlData, actual);
        goto fail;
    }

    ret = 0;

 fail:
    free(actual);
    virNWFilterDefFree(dev);
    return ret;
}

typedef struct test_parms {
    const char *name;
    bool expect_warning;
} test_parms;

static int testCompareXMLToXMLHelper(const void *data) {
    const test_parms *tp = data;
    char inxml[PATH_MAX];
    char outxml[PATH_MAX];
    snprintf(inxml, PATH_MAX, "%s/nwfilterxml2xmlin/%s.xml",
             abs_srcdir, tp->name);
    snprintf(outxml, PATH_MAX, "%s/nwfilterxml2xmlout/%s.xml",
             abs_srcdir, tp->name);
    return testCompareXMLToXMLFiles(inxml, outxml, tp->expect_warning);
}


static int
mymain(int argc, char **argv)
{
    int ret = 0;
    char cwd[PATH_MAX];

    progname = argv[0];

    if (argc > 1) {
        fprintf(stderr, "Usage: %s\n", progname);
        return (EXIT_FAILURE);
    }

    abs_srcdir = getenv("abs_srcdir");
    if (!abs_srcdir)
        abs_srcdir = getcwd(cwd, sizeof(cwd));

#define DO_TEST(NAME, EXPECT_WARN)                                \
    do {                                                          \
        test_parms tp = {                                         \
            .name = NAME,                                         \
            .expect_warning = EXPECT_WARN,                        \
        };                                                        \
        if (virtTestRun("NWFilter XML-2-XML " NAME,               \
                        1, testCompareXMLToXMLHelper, (&tp)) < 0) \
            ret = -1;                                             \
    } while (0)

    DO_TEST("mac-test", true);
    DO_TEST("arp-test", true);
    DO_TEST("rarp-test", true);
    DO_TEST("ip-test", true);
    DO_TEST("ipv6-test", true);

    DO_TEST("tcp-test", true);
    DO_TEST("udp-test", true);
    DO_TEST("icmp-test", true);
    DO_TEST("igmp-test", false);
    DO_TEST("sctp-test", true);
    DO_TEST("udplite-test", false);
    DO_TEST("esp-test", false);
    DO_TEST("ah-test", false);
    DO_TEST("all-test", false);

    DO_TEST("tcp-ipv6-test", true);
    DO_TEST("udp-ipv6-test", true);
    DO_TEST("icmpv6-test", true);
    DO_TEST("sctp-ipv6-test", true);
    DO_TEST("udplite-ipv6-test", true);
    DO_TEST("esp-ipv6-test", true);
    DO_TEST("ah-ipv6-test", true);
    DO_TEST("all-ipv6-test", true);

    DO_TEST("ref-test", false);
    DO_TEST("ref-rule-test", false);
    DO_TEST("ipt-no-macspoof-test", false);
    DO_TEST("icmp-direction-test", false);
    DO_TEST("icmp-direction2-test", false);
    DO_TEST("icmp-direction3-test", false);

    DO_TEST("conntrack-test", false);

    DO_TEST("hex-data-test", true);

    DO_TEST("comment-test", true);

    DO_TEST("example-1", false);
    DO_TEST("example-2", false);

    return (ret==0 ? EXIT_SUCCESS : EXIT_FAILURE);
}

VIRT_TEST_MAIN(mymain)
