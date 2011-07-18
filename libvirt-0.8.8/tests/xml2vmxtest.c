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
testCompareFiles(const char *xml, const char *vmx, int virtualHW_version)
{
    int result = -1;
    char xmlData[MAX_FILE];
    char vmxData[MAX_FILE];
    char *formatted = NULL;
    char *xmlPtr = &(xmlData[0]);
    char *vmxPtr = &(vmxData[0]);
    virDomainDefPtr def = NULL;

    if (virtTestLoadFile(xml, &xmlPtr, MAX_FILE) < 0) {
        goto failure;
    }

    if (virtTestLoadFile(vmx, &vmxPtr, MAX_FILE) < 0) {
        goto failure;
    }

    def = virDomainDefParseString(caps, xmlData, VIR_DOMAIN_XML_INACTIVE);

    if (def == NULL) {
        goto failure;
    }

    formatted = virVMXFormatConfig(&ctx, caps, def, virtualHW_version);

    if (formatted == NULL) {
        goto failure;
    }

    if (STRNEQ(vmxData, formatted)) {
        virtTestDifference(stderr, vmxData, formatted);
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
    int virtualHW_version;
};

static int
testCompareHelper(const void *data)
{
    const struct testInfo *info = data;
    char xml[PATH_MAX];
    char vmx[PATH_MAX];

    snprintf(xml, PATH_MAX, "%s/xml2vmxdata/xml2vmx-%s.xml", abs_srcdir,
             info->input);
    snprintf(vmx, PATH_MAX, "%s/xml2vmxdata/xml2vmx-%s.vmx", abs_srcdir,
             info->output);

    return testCompareFiles(xml, vmx, info->virtualHW_version);
}

static int
testAutodetectSCSIControllerModel(virDomainDiskDefPtr def ATTRIBUTE_UNUSED,
                                  int *model, void *opaque ATTRIBUTE_UNUSED)
{
    *model = VIR_DOMAIN_CONTROLLER_MODEL_LSILOGIC;

    return 0;
}

static char *
testFormatVMXFileName(const char *src, void *opaque ATTRIBUTE_UNUSED)
{
    bool success = false;
    char *copyOfDatastorePath = NULL;
    char *tmp = NULL;
    char *saveptr = NULL;
    char *datastoreName = NULL;
    char *directoryAndFileName = NULL;
    char *absolutePath = NULL;

    if (STRPREFIX(src, "[")) {
        /* Found potential datastore path */
        copyOfDatastorePath = strdup(src);

        if (copyOfDatastorePath == NULL) {
            goto cleanup;
        }

        /* Expected format: '[<datastore>] <path>' where <path> is optional */
        if ((tmp = STRSKIP(copyOfDatastorePath, "[")) == NULL || *tmp == ']' ||
            (datastoreName = strtok_r(tmp, "]", &saveptr)) == NULL) {
            goto cleanup;
        }

        directoryAndFileName = strtok_r(NULL, "", &saveptr);

        if (directoryAndFileName == NULL) {
            directoryAndFileName = (char *)"";
        } else {
            directoryAndFileName += strspn(directoryAndFileName, " ");
        }

        virAsprintf(&absolutePath, "/vmfs/volumes/%s/%s", datastoreName,
                    directoryAndFileName);
    } else if (STRPREFIX(src, "/")) {
        /* Found absolute path */
        absolutePath = strdup(src);
    } else {
        /* Found relative path, this is not supported */
        goto cleanup;
    }

    success = true;

  cleanup:
    if (! success) {
        VIR_FREE(absolutePath);
    }

    VIR_FREE(copyOfDatastorePath);

    return absolutePath;
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

# define DO_TEST(_in, _out, _version)                                         \
        do {                                                                  \
            struct testInfo info = { _in, _out, _version };                   \
            virResetLastError();                                              \
            if (virtTestRun("VMware XML-2-VMX "_in" -> "_out, 1,              \
                            testCompareHelper, &info) < 0) {                  \
                result = -1;                                                  \
            }                                                                 \
        } while (0)

    testCapsInit();

    if (caps == NULL) {
        return EXIT_FAILURE;
    }

    ctx.opaque = NULL;
    ctx.parseFileName = NULL;
    ctx.formatFileName = testFormatVMXFileName;
    ctx.autodetectSCSIControllerModel = testAutodetectSCSIControllerModel;

    DO_TEST("minimal", "minimal", 4);
    DO_TEST("minimal-64bit", "minimal-64bit", 4);

    DO_TEST("graphics-vnc", "graphics-vnc", 4);

    DO_TEST("scsi-driver", "scsi-driver", 4);
    DO_TEST("scsi-writethrough", "scsi-writethrough", 4);

    DO_TEST("harddisk-scsi-file", "harddisk-scsi-file", 4);
    DO_TEST("harddisk-ide-file", "harddisk-ide-file", 4);

    DO_TEST("cdrom-scsi-file", "cdrom-scsi-file", 4);
    DO_TEST("cdrom-scsi-device", "cdrom-scsi-device", 4);
    DO_TEST("cdrom-ide-file", "cdrom-ide-file", 4);
    DO_TEST("cdrom-ide-device", "cdrom-ide-device", 4);

    DO_TEST("floppy-file", "floppy-file", 4);
    DO_TEST("floppy-device", "floppy-device", 4);

    DO_TEST("ethernet-e1000", "ethernet-e1000", 4);
    DO_TEST("ethernet-vmxnet2", "ethernet-vmxnet2", 4);

    DO_TEST("ethernet-custom", "ethernet-custom", 4);
    DO_TEST("ethernet-bridged", "ethernet-bridged", 4);

    DO_TEST("ethernet-generated", "ethernet-generated", 4);
    DO_TEST("ethernet-static", "ethernet-static", 4);
    DO_TEST("ethernet-vpx", "ethernet-vpx", 4);
    DO_TEST("ethernet-other", "ethernet-other", 4);

    DO_TEST("serial-file", "serial-file", 4);
    DO_TEST("serial-device", "serial-device", 4);
    DO_TEST("serial-pipe", "serial-pipe", 4);
    DO_TEST("serial-network-server", "serial-network-server", 7);
    DO_TEST("serial-network-client", "serial-network-client", 7);

    DO_TEST("parallel-file", "parallel-file", 4);
    DO_TEST("parallel-device", "parallel-device", 4);

    DO_TEST("esx-in-the-wild-1", "esx-in-the-wild-1", 4);
    DO_TEST("esx-in-the-wild-2", "esx-in-the-wild-2", 4);
    DO_TEST("esx-in-the-wild-3", "esx-in-the-wild-3", 4);
    DO_TEST("esx-in-the-wild-4", "esx-in-the-wild-4", 4);
    DO_TEST("esx-in-the-wild-5", "esx-in-the-wild-5", 4);

    DO_TEST("gsx-in-the-wild-1", "gsx-in-the-wild-1", 4);
    DO_TEST("gsx-in-the-wild-2", "gsx-in-the-wild-2", 4);
    DO_TEST("gsx-in-the-wild-3", "gsx-in-the-wild-3", 4);
    DO_TEST("gsx-in-the-wild-4", "gsx-in-the-wild-4", 4);

    DO_TEST("annotation", "annotation", 4);

    DO_TEST("smbios", "smbios", 4);

    DO_TEST("svga", "svga", 4);

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
