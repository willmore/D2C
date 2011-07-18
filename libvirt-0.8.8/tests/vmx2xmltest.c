#include <config.h>

#ifdef WITH_VMX

# include <stdio.h>
# include <string.h>
# include <unistd.h>

# include "internal.h"
# include "memory.h"
# include "testutils.h"
# include "vmx/vmx.h"

static char *progname = NULL;
static char *abs_srcdir = NULL;
static virCapsPtr caps = NULL;
static virVMXContext ctx;

# define MAX_FILE 4096

static void
testCapsInit(void)
{
    virCapsGuestPtr guest = NULL;

    caps = virCapabilitiesNew("i686", 1, 1);

    if (caps == NULL) {
        return;
    }

    virCapabilitiesSetMacPrefix(caps, (unsigned char[]){ 0x00, 0x0c, 0x29 });
    virCapabilitiesAddHostMigrateTransport(caps, "esx");

    caps->hasWideScsiBus = true;

    /* i686 guest */
    guest =
      virCapabilitiesAddGuest(caps, "hvm", "i686", 32, NULL, NULL, 0, NULL);

    if (guest == NULL) {
        goto failure;
    }

    if (virCapabilitiesAddGuestDomain(guest, "vmware", NULL, NULL, 0,
                                      NULL) == NULL) {
        goto failure;
    }

    /* x86_64 guest */
    guest =
      virCapabilitiesAddGuest(caps, "hvm", "x86_64", 64, NULL, NULL, 0, NULL);

    if (guest == NULL) {
        goto failure;
    }

    if (virCapabilitiesAddGuestDomain(guest, "vmware", NULL, NULL, 0,
                                      NULL) == NULL) {
        goto failure;
    }

    return;

  failure:
    virCapabilitiesFree(caps);
    caps = NULL;
}

static int
testCompareFiles(const char *vmx, const char *xml)
{
    int result = -1;
    char vmxData[MAX_FILE];
    char xmlData[MAX_FILE];
    char *formatted = NULL;
    char *vmxPtr = &(vmxData[0]);
    char *xmlPtr = &(xmlData[0]);
    virDomainDefPtr def = NULL;
    virErrorPtr err = NULL;

    if (virtTestLoadFile(vmx, &vmxPtr, MAX_FILE) < 0) {
        goto failure;
    }

    if (virtTestLoadFile(xml, &xmlPtr, MAX_FILE) < 0) {
        goto failure;
    }

    def = virVMXParseConfig(&ctx, caps, vmxData);

    if (def == NULL) {
        err = virGetLastError();
        fprintf(stderr, "ERROR: %s\n", err != NULL ? err->message : "<unknown>");
        goto failure;
    }

    formatted = virDomainDefFormat(def, VIR_DOMAIN_XML_SECURE);

    if (formatted == NULL) {
        err = virGetLastError();
        fprintf(stderr, "ERROR: %s\n", err != NULL ? err->message : "<unknown>");
        goto failure;
    }

    if (STRNEQ(xmlData, formatted)) {
        virtTestDifference(stderr, xmlData, formatted);
        goto failure;
    }

    result = 0;

  failure:
    VIR_FREE(formatted);
    virDomainDefFree(def);

    return result;
}

struct testInfo {
    const char *input;
    const char *output;
};

static int
testCompareHelper(const void *data)
{
    const struct testInfo *info = data;
    char vmx[PATH_MAX];
    char xml[PATH_MAX];

    snprintf(vmx, PATH_MAX, "%s/vmx2xmldata/vmx2xml-%s.vmx", abs_srcdir,
             info->input);
    snprintf(xml, PATH_MAX, "%s/vmx2xmldata/vmx2xml-%s.xml", abs_srcdir,
             info->output);

    return testCompareFiles(vmx, xml);
}

static char *
testParseVMXFileName(const char *fileName, void *opaque ATTRIBUTE_UNUSED)
{
    char *copyOfFileName = NULL;
    char *tmp = NULL;
    char *saveptr = NULL;
    char *datastoreName = NULL;
    char *directoryAndFileName = NULL;
    char *src = NULL;

    if (STRPREFIX(fileName, "/vmfs/volumes/")) {
        /* Found absolute path referencing a file inside a datastore */
        copyOfFileName = strdup(fileName);

        if (copyOfFileName == NULL) {
            goto cleanup;
        }

        /* Expected format: '/vmfs/volumes/<datastore>/<path>' */
        if ((tmp = STRSKIP(copyOfFileName, "/vmfs/volumes/")) == NULL ||
            (datastoreName = strtok_r(tmp, "/", &saveptr)) == NULL ||
            (directoryAndFileName = strtok_r(NULL, "", &saveptr)) == NULL) {
            goto cleanup;
        }

        virAsprintf(&src, "[%s] %s", datastoreName, directoryAndFileName);
    } else if (STRPREFIX(fileName, "/")) {
        /* Found absolute path referencing a file outside a datastore */
        src = strdup(fileName);
    } else if (strchr(fileName, '/') != NULL) {
        /* Found relative path, this is not supported */
        src = NULL;
    } else {
        /* Found single file name referencing a file inside a datastore */
        virAsprintf(&src, "[datastore] directory/%s", fileName);
    }

  cleanup:
    VIR_FREE(copyOfFileName);

    return src;
}

static int
mymain(int argc, char **argv)
{
    int result = 0;
    char cwd[PATH_MAX];

    progname = argv[0];

    if (argc > 1) {
        fprintf(stderr, "Usage: %s\n", progname);
        return EXIT_FAILURE;
    }

    abs_srcdir = getenv("abs_srcdir");

    if (abs_srcdir == NULL) {
        abs_srcdir = getcwd(cwd, sizeof(cwd));
    }

    if (argc > 1) {
        fprintf(stderr, "Usage: %s\n", progname);
        return EXIT_FAILURE;
    }

# define DO_TEST(_in, _out)                                                   \
        do {                                                                  \
            struct testInfo info = { _in, _out };                             \
            virResetLastError();                                              \
            if (virtTestRun("VMware VMX-2-XML "_in" -> "_out, 1,              \
                            testCompareHelper, &info) < 0) {                  \
                result = -1;                                                  \
            }                                                                 \
        } while (0)

    testCapsInit();

    if (caps == NULL) {
        return EXIT_FAILURE;
    }

    ctx.opaque = NULL;
    ctx.parseFileName = testParseVMXFileName;
    ctx.formatFileName = NULL;
    ctx.autodetectSCSIControllerModel = NULL;

    DO_TEST("case-insensitive-1", "case-insensitive-1");
    DO_TEST("case-insensitive-2", "case-insensitive-2");

    DO_TEST("minimal", "minimal");
    DO_TEST("minimal-64bit", "minimal-64bit");

    DO_TEST("graphics-vnc", "graphics-vnc");

    DO_TEST("scsi-driver", "scsi-driver");
    DO_TEST("scsi-writethrough", "scsi-writethrough");

    DO_TEST("harddisk-scsi-file", "harddisk-scsi-file");
    DO_TEST("harddisk-ide-file", "harddisk-ide-file");

    DO_TEST("cdrom-scsi-file", "cdrom-scsi-file");
    DO_TEST("cdrom-scsi-device", "cdrom-scsi-device");
    DO_TEST("cdrom-ide-file", "cdrom-ide-file");
    DO_TEST("cdrom-ide-device", "cdrom-ide-device");

    DO_TEST("floppy-file", "floppy-file");
    DO_TEST("floppy-device", "floppy-device");

    DO_TEST("ethernet-e1000", "ethernet-e1000");
    DO_TEST("ethernet-vmxnet2", "ethernet-vmxnet2");

    DO_TEST("ethernet-custom", "ethernet-custom");
    DO_TEST("ethernet-bridged", "ethernet-bridged");

    DO_TEST("ethernet-generated", "ethernet-generated");
    DO_TEST("ethernet-static", "ethernet-static");
    DO_TEST("ethernet-vpx", "ethernet-vpx");
    DO_TEST("ethernet-other", "ethernet-other");

    DO_TEST("serial-file", "serial-file");
    DO_TEST("serial-device", "serial-device");
    DO_TEST("serial-pipe-client-app", "serial-pipe");
    DO_TEST("serial-pipe-server-vm", "serial-pipe");
    DO_TEST("serial-pipe-client-app", "serial-pipe");
    DO_TEST("serial-pipe-server-vm", "serial-pipe");
    DO_TEST("serial-network-server", "serial-network-server");
    DO_TEST("serial-network-client", "serial-network-client");

    DO_TEST("parallel-file", "parallel-file");
    DO_TEST("parallel-device", "parallel-device");

    DO_TEST("esx-in-the-wild-1", "esx-in-the-wild-1");
    DO_TEST("esx-in-the-wild-2", "esx-in-the-wild-2");
    DO_TEST("esx-in-the-wild-3", "esx-in-the-wild-3");
    DO_TEST("esx-in-the-wild-4", "esx-in-the-wild-4");
    DO_TEST("esx-in-the-wild-5", "esx-in-the-wild-5");

    DO_TEST("gsx-in-the-wild-1", "gsx-in-the-wild-1");
    DO_TEST("gsx-in-the-wild-2", "gsx-in-the-wild-2");
    DO_TEST("gsx-in-the-wild-3", "gsx-in-the-wild-3");
    DO_TEST("gsx-in-the-wild-4", "gsx-in-the-wild-4");

    DO_TEST("annotation", "annotation");

    DO_TEST("smbios", "smbios");

    DO_TEST("svga", "svga");

    virCapabilitiesFree(caps);

    return result == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

VIRT_TEST_MAIN(mymain)

#else

int main (void)
{
    return 77; /* means 'test skipped' for automake */
}

#endif /* WITH_VMX */
